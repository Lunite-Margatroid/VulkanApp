# Lua 脚本接入（sandbox）

本文说明如何把 Lua 5.4 + sol2 接进 `sandbox`，让 `.lua` 脚本能够调用 `LT::Engine` 搭建和驱动场景。

- 路线背景见 `doc/CmdDesign.md`。
- 依赖已经准备好：Lua 与 sol2 由仓库脚本安装到 `vendor/dist/`，`#include <sol/sol.hpp>` 的用法见 `TestSol2`。
- **本文不涉及 Lua/sol2 的安装、头文件路径与链接**；sandbox 的构建配置参照 `TestSol2/CMakeLists.txt` 即可。

## 0. 目标与非目标

目标：

1. sandbox 可以加载一个 `.lua` 入口脚本。
2. 脚本通过一组 `engine.*` 函数调用 `LT::Engine`，创建场景、材质、网格、灯光。
3. 脚本可以定义 `on_update(dt)`，随主循环逐帧执行。
4. 保留现有 `Shell`：新增 `lua.run <file>` 命令，在运行期加载/重跑脚本。

非目标（放到后续再做）：

- 热重载（文件变化自动重跑）。
- 完整面向对象的节点封装（`node:set_position(...)`）。
- 用 Lua 替换 Shell、协程调度、脚本沙箱化。

## 1. 先确定线程模型

现有的执行结构：

- **主线程**：SDL 事件、`Shell::Flush()`（执行排队的 Shell 命令）、`Engine::UpdateDebugScene()`、`DisplaySurface::Present()`。
- **Shell 线程**：`std::getline(std::cin, ...)` 读一行 → 解析 → 加锁塞进 `m_qCmd`。它从不直接调用 Engine。

```
stdin 线程 ──► Shell::Command() ──► m_qCmd (mutex)
                                      │
主线程 App::Run                        ▼
  ├─ Shell::Flush() ──► 执行命令（lua.run 在这里执行 Lua）
  ├─ LuaScriptEngine::OnUpdate(dt)   ◄── Lua VM 只在主线程使用
  ├─ Engine::UpdateDebugScene()
  └─ DisplaySurface::Present()
```

Lua 接入必须遵守这些规则：

1. **`sol::state` 不是线程安全的**。创建、执行、销毁都必须发生在主线程。
2. **所有 Engine 调用只允许发生在主线程**。GPU 资源没有跨线程保护。
3. **Shell 线程永远不碰 Lua**。`lua.run` 只会被 Shell 线程解析并入队，真正的脚本执行发生在 `Shell::Flush()` 里，而 `Flush()` 由主线程调用。这是安全的，不需要额外加队列。
4. **不要在 Lua 绑定里调用 `Shell::Command()`**。`Flush()` 执行命令期间持有 `m_mtxCmdQueue`，`Command()` 会再次加同一把锁 → 死锁。脚本需要什么能力就直接绑定到 `LT::Engine`，不要绕回 Shell。
5. 脚本在主线程里同步执行，**死循环/超长循环会卡住渲染循环**。耗时逻辑应拆到 `on_update` 里分帧完成；Lua 没有抢占式超时，这里假定运行的是本地可信脚本。

因此不要为了"不卡帧"把 Lua 放到独立线程。正确做法和 Shell 一致：把脚本请求入队，由主线程在安全点执行。

## 2. 落地步骤

1. 在 `sandbox/` 下新增 `LuaScriptEngine.hpp/.cpp`（放在 sandbox 根目录，现有的 `file(GLOB "./*.cpp")` 会自动收集；放子目录需要改 CMake）。
2. `App` 持有 `LuaScriptEngine`，在 `CreateDebugScene` 之后创建，在帧循环里调用 `OnUpdate`。
3. 实现绑定层：`vec3`、`log`、`engine.img / mtl / node / light`。
4. `Shell` 新增 `lua.run <path>` 命令。
5. 把 `App.cpp` 里建 400 个立方体的 C++ 代码迁移到 `scripts/main.lua`。
6. 补错误路径测试：路径不存在、语法错误、运行时错误、错误的属性名/类型。

## 3. 新增 LuaScriptEngine

### 3.1 头文件

```cpp
// sandbox/LuaScriptEngine.hpp
#pragma once

// sol2 的安全检查。必须在包含 sol.hpp 之前定义。
// 工程中所有包含 sol2 的翻译单元要保持一致，否则可能产生 ODR 问题。
#ifndef SOL_ALL_SAFETIES_ON
#define SOL_ALL_SAFETIES_ON 1
#endif
#include <sol/sol.hpp>

#include "Engine.h"

#include <cstdint>
#include <set>
#include <string>
#include <string_view>

class LuaScriptEngine
{
public:
	explicit LuaScriptEngine(LT::Engine* pEngine);
	~LuaScriptEngine();

	LuaScriptEngine(const LuaScriptEngine&) = delete;
	LuaScriptEngine& operator=(const LuaScriptEngine&) = delete;

	// 以下接口都只能在主线程调用
	bool RunFile(std::string_view strPath);
	bool RunString(std::string_view strCode, std::string_view strChunkName = "=sandbox");
	void OnUpdate(double dDeltaTimeSec);

private:
	void InitState();
	void RegisterCoreTypes();
	void RegisterBindings();
	void CleanupCreatedObjects();

	LT::Engine* m_pEngine = nullptr; // 不持有
	LT::NodeID m_nMainScene = LT::INVALID_NODE_ID;
	sol::state m_lua;

	// 脚本创建的引擎对象 析构/重跑时统一释放（思路同 Shell 的 m_setNode 等）
	std::set<LT::NodeID> m_setNode;
	std::set<LT::MaterialID> m_setMtl;
	std::set<LT::ImageID> m_setImg;
};
```

`App.hpp` 只需要前向声明 `class LuaScriptEngine;`，不要包含这个头文件，避免 sol2 扩散到所有包含 `App.hpp` 的编译单元。

### 3.2 构造与脚本执行

```cpp
// sandbox/LuaScriptEngine.cpp
#include "LuaScriptEngine.hpp"

#include "logger.hpp"

#include <array>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

LuaScriptEngine::LuaScriptEngine(LT::Engine* pEngine)
	: m_pEngine(pEngine)
{
	// 记录 View 的主场景。脚本创建的对象默认挂到它下面，
	// 与 Shell 的 node.mesh.cube 挂到 SELECTED_S 的行为保持一致。
	std::vector<LT::NodeID> vecRoots;
	if (m_pEngine->GetRootSceneNodes(vecRoots) == LT::EngineResult::eSuccess && !vecRoots.empty())
	{
		m_nMainScene = vecRoots.front();
	}

	InitState();
}

void LuaScriptEngine::InitState()
{
	// 只打开需要的库。不打开 io/os/debug/package：
	// 脚本不需要文件系统/环境变量访问，也避免误用 debug.sethook。
	m_lua.open_libraries(
		sol::lib::base,
		sol::lib::coroutine,
		sol::lib::table,
		sol::lib::string,
		sol::lib::math);

	RegisterCoreTypes();
	RegisterBindings();
}

bool LuaScriptEngine::RunFile(std::string_view strPath)
{
	// script_pass_on_error：脚本出错时返回无效的结果对象，而不是抛异常
	sol::protected_function_result result =
		m_lua.safe_script_file(std::string(strPath), sol::script_pass_on_error);

	if (!result.valid())
	{
		sol::error err = result;
		LOG_ERROR("Lua script error (%s): %s", std::string(strPath).c_str(), err.what());
		return false;
	}
	return true;
}

bool LuaScriptEngine::RunString(std::string_view strCode, std::string_view strChunkName)
{
	sol::protected_function_result result =
		m_lua.safe_script(strCode, sol::script_pass_on_error, std::string(strChunkName));

	if (!result.valid())
	{
		sol::error err = result;
		LOG_ERROR("Lua string chunk error: %s", err.what());
		return false;
	}
	return true;
}
```

要点：

- `safe_script_file` 的默认错误策略是抛 `sol::error`；显式传 `sol::script_pass_on_error` 改成返回结果对象，避免异常穿过 C++ 调用栈，脚本错误只记日志、不影响程序继续运行。
- 错误信息自带 `文件:行号`，排查足够用。需要更详细的调用栈时，可以在错误处理里用 `luaL_traceback` 加工，这里不展开。
- Lua 的全局函数 `print` 直接写 stdout；如果需要和渲染日志统一，使用下面绑定的 `log()`。

### 3.3 逐帧回调

约定：脚本定义全局函数 `on_update(dt)`，`dt` 单位为秒。没有定义就跳过。

```cpp
void LuaScriptEngine::OnUpdate(double dDeltaTimeSec)
{
	sol::object objUpdate = m_lua["on_update"];
	if (objUpdate.get_type() != sol::type::function)
	{
		return;
	}

	sol::protected_function fnUpdate = objUpdate.as<sol::protected_function>();
	sol::protected_function_result result = fnUpdate(dDeltaTimeSec);
	if (!result.valid())
	{
		// 单帧出错只记录 不中断渲染
		sol::error err = result;
		LOG_ERROR("Lua on_update error: %s", err.what());
	}
}
```

每帧查一次全局表开销很小；如果以后脚本很多，可以在 `RunFile` 之后把函数缓存到成员里，重跑脚本时再刷新。

### 3.4 清理

```cpp
LuaScriptEngine::~LuaScriptEngine()
{
	CleanupCreatedObjects();
}

void LuaScriptEngine::CleanupCreatedObjects()
{
	// 顺序与 Shell::~Shell 相同：先节点 再材质 最后贴图
	for (LT::NodeID nNode : m_setNode)
	{
		m_pEngine->DeleteSceneNode(nNode);
	}
	m_setNode.clear();

	for (LT::MaterialID nMtl : m_setMtl)
	{
		m_pEngine->DeleteMaterial(nMtl);
	}
	m_setMtl.clear();

	for (LT::ImageID nImg : m_setImg)
	{
		m_pEngine->DeleteImage(nImg);
	}
	m_setImg.clear();
}
```

这里有一个**现有引擎的坑**，见第 9 节：`DeleteSceneNode` 只是裸删除，不会把节点从父节点上摘下来。v1 建议脚本只把节点挂在主场景下（下面绑定里的 parent 参数省略），且主场景在进程退出前不会析构，因此这种清理方式与 Shell 现在的做法一样"暂时安全"。一旦在脚本里显式传 parent 造子树，或做热重载，必须先修好引擎的删除语义。

## 4. 绑定层设计

### 4.1 不要把 `LT::Engine` 整体注册成 usertype

原因：

- Engine 大量接口是"出参 + 返回 `EngineResult`"，例如 `CreateCubeMeshNode(NodeID& out, float)`。直接注册 usertype 时出参在 Lua 里无法调用。
- 引擎接口名、所有权语义直接暴露给脚本，后续改引擎容易破坏脚本。
- 绑定层放在 sandbox 可以自由做"扁平函数 + 返回值 + 抛错"的转换，不影响渲染器。

所以采用：`engine` 表下按模块分组，每个函数是薄封装，职责只有参数转换 + 错误转换 + 记录创建的 ID。

### 4.2 错误约定

- Engine 返回 `EngineResult`，`eSuccess` 以外都在绑定层转换成 Lua 错误。
- 绑定层抛 `std::runtime_error`；sol2 在受保护调用里会把它转换成 Lua error。
  - 脚本可以用 `pcall` 捕获。
  - 如果没人捕获，`safe_script` / `protected_function` 会把它变成无效结果，由 `RunFile` / `OnUpdate` 记录日志。
- 这样 Lua 侧不需要理解枚举，出错时直接终止当前脚本片段。

```cpp
namespace
{
const char* ToString(LT::EngineResult eResult)
{
	switch (eResult)
	{
	case LT::EngineResult::eSuccess: return "eSuccess";
	case LT::EngineResult::eFailed: return "eFailed";
	case LT::EngineResult::eInvalidParam: return "eInvalidParam";
	case LT::EngineResult::eNoComponent: return "eNoComponent";
	case LT::EngineResult::eNoSceneNode: return "eNoSceneNode";
	case LT::EngineResult::eNoMaterial: return "eNoMaterial";
	}
	return "unknown";
}

void ThrowIfFailed(LT::EngineResult eResult, std::string_view strWhat)
{
	if (eResult == LT::EngineResult::eSuccess)
	{
		return;
	}
	throw std::runtime_error(std::string(strWhat) + " failed, EngineResult = " + ToString(eResult));
}
} // namespace
```

注意：依赖 C++ 异常。MSVC 默认开启异常，项目也使用异常（`RENDERER_ASSERT` 会抛 `LT::ExceptionAssert`），没有问题；如果以后改成关闭异常，这里的错误传播方式需要重新设计。

### 4.3 句柄与 vec3

- `NodeID` / `MaterialID` / `ImageID` 等都是 `int64_t`。Lua 5.4 的整数也是 64 位，直接当 number 传递不会丢精度。
- 无效句柄统一是 `-1`（`INVALID_*`）。脚本里不要自己判断 ID 是否还有效，见第 9 节。
- Engine 的向量参数是 `std::array<float, 2/3/4>`。**不要直接绑定 glm 类型**，在 sandbox 里做一个只面向脚本的轻量类型：

```cpp
namespace
{
struct LuaVec3
{
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

std::array<float, 3> ToArray(const LuaVec3& v)
{
	return { v.x, v.y, v.z };
}
} // namespace
```

注册：

```cpp
void LuaScriptEngine::RegisterCoreTypes()
{
	m_lua.new_usertype<LuaVec3>("Vec3",
		sol::constructors<LuaVec3(), LuaVec3(float, float, float)>(),
		"x", &LuaVec3::x,
		"y", &LuaVec3::y,
		"z", &LuaVec3::z);

	// 脚本侧用 vec3(x, y, z) 创建；Vec3.new(...) 也可用
	m_lua.set_function("vec3", [](float x, float y, float z) {
		return LuaVec3{ x, y, z };
	});

	m_lua.set_function("log", [](const std::string& strMsg) {
		LOG_INFO("%s", strMsg.c_str());
	});
}
```

为什么不直接把参数写成 `std::array<float, 3>`：sol2 虽然可以把 Lua 表按容器转换，但脚本侧只能写匿名表 `{1, 2, 3}`，可读性和报错都不如显式的 `vec3(...)`。

### 4.4 绑定实现

完整的 `RegisterBindings` 如下（按模块分段）。注意：`ThrowIfFailed`、`LuaVec3`、`ToString(EngineResult)` 以及下面的查表函数都放在 `.cpp` 顶部的匿名命名空间里，位置要在 `RegisterBindings` 之前。

```cpp
void LuaScriptEngine::RegisterBindings()
{
	sol::table engine = m_lua["engine"].get_or_create<sol::table>();

	// ================= img =================
	sol::table img = engine["img"].get_or_create<sol::table>();
	img.set_function("file", [this](const std::string& strPath) -> int64_t
	{
		LT::ImageID nImg = LT::INVALID_IMAGE_ID;
		ThrowIfFailed(m_pEngine->LoadImageFromFile(nImg, strPath), "img.file");
		m_setImg.insert(nImg);
		return nImg;
	});

	// ================= mtl =================
	sol::table mtl = engine["mtl"].get_or_create<sol::table>();
	mtl.set_function("create", [this](const std::string& strType) -> int64_t
	{
		auto iter = GetMtlTypeMap().find(strType);
		if (iter == GetMtlTypeMap().end())
		{
			throw std::runtime_error("unknown material type: " + strType);
		}

		LT::MaterialID nMtl = LT::INVALID_MATERIAL_ID;
		ThrowIfFailed(m_pEngine->CreateMaterial(nMtl, iter->second), "mtl.create");
		m_setMtl.insert(nMtl);
		return nMtl;
	});

	// 值类型由属性本身的类型决定 见 4.6
	mtl.set_function("prop", [this](int64_t nMtl, const std::string& strProp, sol::object objValue)
	{
		auto iter = GetMtlPropMap().find(strProp);
		if (iter == GetMtlPropMap().end())
		{
			throw std::runtime_error("unknown material prop: " + strProp);
		}

		LT::MtlPropVar vValue;
		switch (LT::GetMtlPropDataType(iter->second))
		{
		case LT::MtlPropDataType::eBool:
		case LT::MtlPropDataType::eInt:
		case LT::MtlPropDataType::eImage:
			if (!objValue.is<int64_t>())
			{
				throw std::runtime_error("mtl.prop " + strProp + " expects an integer");
			}
			vValue = objValue.as<int64_t>();
			break;

		case LT::MtlPropDataType::eFloat:
			if (objValue.is<float>())
			{
				vValue = objValue.as<float>();
			}
			else if (objValue.is<int64_t>())
			{
				// 与 Shell 一致：Lua 的整数字面量允许赋给浮点属性
				vValue = static_cast<float>(objValue.as<int64_t>());
			}
			else
			{
				throw std::runtime_error("mtl.prop " + strProp + " expects a number");
			}
			break;

		case LT::MtlPropDataType::eFloat3:
			if (!objValue.is<LuaVec3>())
			{
				throw std::runtime_error("mtl.prop " + strProp + " expects a vec3");
			}
			vValue = ToArray(objValue.as<LuaVec3>());
			break;

		default:
			throw std::runtime_error("mtl.prop " + strProp + ": prop type not bound");
		}

		ThrowIfFailed(m_pEngine->SetMaterialProp(nMtl, iter->second, vValue), "mtl.prop");
	});

	// ================= node =================
	sol::table node = engine["node"].get_or_create<sol::table>();

	node.set_function("position", [this](int64_t nNode, const LuaVec3& v)
	{
		ThrowIfFailed(m_pEngine->SetSceneNodePosition(nNode, ToArray(v)), "node.position");
	});

	node.set_function("eulerXYZ", [this](int64_t nNode, const LuaVec3& v)
	{
		ThrowIfFailed(m_pEngine->SetSceneNodeEulerRotation(nNode, ToArray(v)), "node.eulerXYZ");
	});

	node.set_function("scale", [this](int64_t nNode, const LuaVec3& v)
	{
		ThrowIfFailed(m_pEngine->SetSceneNodeScale(nNode, ToArray(v)), "node.scale");
	});

	node.set_function("mtl", [this](int64_t nNode, int64_t nMtl)
	{
		ThrowIfFailed(m_pEngine->SetMaterial(nNode, nMtl), "node.mtl");
	});

	sol::table mesh = node["mesh"].get_or_create<sol::table>();

	mesh.set_function("cube", [this](float fSide, sol::optional<int64_t> optParent) -> int64_t
	{
		LT::NodeID nNode = LT::INVALID_NODE_ID;
		ThrowIfFailed(m_pEngine->CreateCubeMeshNode(nNode, fSide), "node.mesh.cube");
		m_setNode.insert(nNode); // 先记录 保证后续步骤失败也能被清理

		LT::NodeID nParent = optParent.value_or(m_nMainScene);
		if (nParent != LT::INVALID_NODE_ID)
		{
			ThrowIfFailed(m_pEngine->SceneNodeRebase(nParent, nNode), "node.mesh.cube(rebase)");
		}
		return nNode;
	});

	mesh.set_function("sphere", [this](float fRadius, uint32_t nLongSub, uint32_t nLatSub,
		sol::optional<int64_t> optParent) -> int64_t
	{
		LT::NodeID nNode = LT::INVALID_NODE_ID;
		ThrowIfFailed(m_pEngine->CreateSphereMeshNode(nNode, fRadius, nLongSub, nLatSub), "node.mesh.sphere");
		m_setNode.insert(nNode);

		LT::NodeID nParent = optParent.value_or(m_nMainScene);
		if (nParent != LT::INVALID_NODE_ID)
		{
			ThrowIfFailed(m_pEngine->SceneNodeRebase(nParent, nNode), "node.mesh.sphere(rebase)");
		}
		return nNode;
	});

	// ================= light =================
	// 独立成 engine.light 表而不是 engine.node.light：Lua 里一个名字不能
	// 同时是"可调用的创建函数"和"存放 color/range 的表"（除非额外设置 __call 元表）。
	sol::table light = engine["light"].get_or_create<sol::table>();

	light.set_function("create", [this](const std::string& strType,
		sol::optional<int64_t> optParent) -> int64_t
	{
		auto iter = GetLightTypeMap().find(strType);
		if (iter == GetLightTypeMap().end())
		{
			throw std::runtime_error("unknown light type: " + strType);
		}

		LT::NodeID nNode = LT::INVALID_NODE_ID;
		ThrowIfFailed(m_pEngine->CreateLightNode(nNode, optParent.value_or(m_nMainScene), iter->second),
			"light.create");
		m_setNode.insert(nNode);
		return nNode;
	});

	light.set_function("color", [this](int64_t nNode, const LuaVec3& v)
	{
		ThrowIfFailed(m_pEngine->SetLightColor(nNode, ToArray(v)), "light.color");
	});

	light.set_function("range", [this](int64_t nNode, float fRange)
	{
		ThrowIfFailed(m_pEngine->SetLightRange(nNode, fRange), "light.range");
	});

	light.set_function("radius", [this](int64_t nNode, float fRadius)
	{
		ThrowIfFailed(m_pEngine->SetLightRadius(nNode, fRadius), "light.radius");
	});

	light.set_function("rectSize", [this](int64_t nNode, float fU, float fV)
	{
		ThrowIfFailed(m_pEngine->SetLightRectSize(nNode, std::array<float, 2>{ fU, fV }), "light.rectSize");
	});

	light.set_function("spotRadian", [this](int64_t nNode, float fInner, float fOuter)
	{
		ThrowIfFailed(m_pEngine->SetLightSpotRadian(nNode, std::array<float, 2>{ fInner, fOuter }),
			"light.spotRadian");
	});
}
```

字符串到枚举的映射表同样放在匿名命名空间、`RegisterBindings` 之前（与 Shell 的思路一致，宏直接用现有的列表宏生成）：

```cpp
namespace
{
const std::map<std::string, LT::MaterialType>& GetMtlTypeMap()
{
	static const std::map<std::string, LT::MaterialType> s_map = []()
	{
		std::map<std::string, LT::MaterialType> map;
#define X(MtlType) map[#MtlType] = LT::MaterialType::e##MtlType;
		MaterialTypeList(X)
#undef X
		return map;
	}();
	return s_map;
}

const std::map<std::string, LT::LightType>& GetLightTypeMap()
{
	static const std::map<std::string, LT::LightType> s_map = []()
	{
		std::map<std::string, LT::LightType> map;
#define X(PLightType) map[#PLightType] = LT::LightType::e##PLightType;
		LightTypeList(X)
#undef X
		return map;
	}();
	return s_map;
}

const std::map<std::string, LT::MtlProp>& GetMtlPropMap()
{
	static const std::map<std::string, LT::MtlProp> s_map = []()
	{
		std::map<std::string, LT::MtlProp> map;
#define X(eProp, eType) map[#eProp] = LT::MtlProp::e##eProp;
		MtlPropList(X)
#undef X
		return map;
	}();
	return s_map;
}
} // namespace
```

### 4.5 Lua API 一览

| Lua 调用 | 对应 Engine 接口 | 说明 |
|---|---|---|
| `engine.img.file(path)` | `LoadImageFromFile` | 返回 ImageID |
| `engine.mtl.create(type)` | `CreateMaterial` | type 见 `MaterialTypeList`（`Phong` 等） |
| `engine.mtl.prop(mtl, name, value)` | `SetMaterialProp` | 值类型由属性决定，见 4.6 |
| `engine.node.position(node, v)` | `SetSceneNodePosition` | |
| `engine.node.eulerXYZ(node, v)` | `SetSceneNodeEulerRotation` | XYZ 外旋 |
| `engine.node.scale(node, v)` | `SetSceneNodeScale` | |
| `engine.node.mtl(node, mtl)` | `SetMaterial` | |
| `engine.node.mesh.cube(side [, parent])` | `CreateCubeMeshNode` + `SceneNodeRebase` | 默认挂主场景 |
| `engine.node.mesh.sphere(r, longSub, latSub [, parent])` | `CreateSphereMeshNode` + `SceneNodeRebase` | 默认挂主场景 |
| `engine.light.create(type [, parent])` | `CreateLightNode` | type 见 `LightTypeList` |
| `engine.light.color(node, v)` | `SetLightColor` | |
| `engine.light.range(node, r)` | `SetLightRange` | 点光/聚光 |
| `engine.light.radius(node, r)` | `SetLightRadius` | 球形光 |
| `engine.light.rectSize(node, u, v)` | `SetLightRectSize` | 矩形光，两个半宽 |
| `engine.light.spotRadian(node, inner, outer)` | `SetLightSpotRadian` | 聚光灯内/外锥角（弧度） |
| `vec3(x, y, z)` | — | 创建向量 |
| `log(msg)` | — | 输出到 `LOG_INFO` |

命名尽量与 Shell 命令保持一致（`node.position`、`node.eulerXYZ`、`node.mtl`、`mtl.prop` …），减少两套接口之间的记忆负担。`lua.run` 与 Shell 的对应关系见第 6 节。

关于 `parent` 参数：省略时挂到主场景，与 Shell 行为一致，这是 v1 的推荐用法。显式传一个脚本自己创建的节点会形成子树，而当前引擎的删除语义不支持安全清理（见第 9 节），除非你已经修复了引擎。

### 4.6 材质属性与 variant

`SetMaterialProp` 的参数是 `LT::MtlPropVar`（variant），而 Lua 是动态类型，所以绑定函数接收 `sol::object`，先查属性名得到期望的数据类型，再做严格的转换：

- `eFloat` → 接受 float，也接受整数（转 float）。
- `eInt` / `eImage` → 只接受整数。
- `eFloat3` → 只接受 `vec3`。
- `eBool` 按整数处理（与 Shell 的 `IsMtlPropTypeMatch` 一致）。
- `eFloat2/eFloat4/eInt2/eInt3/eInt4` 目前没绑定，需要时按同样方式加 `vec2/vec4` 类型和分支。

属性名来自 `MtlPropList` 宏，注意引擎里已有的拼写（例如 `TexMeltalness`），脚本必须与之一致。

## 5. 接入 App

### 5.1 字段

`App.hpp`：

```cpp
class LuaScriptEngine; // 前向声明 不要在头文件里引入 sol2

class App {
private:
	...
	std::unique_ptr<LuaScriptEngine> m_pLua; // 在 m_pShell 之前声明
	std::unique_ptr<Shell> m_pShell;
	...
};
```

### 5.2 构造与析构

`App.cpp` 增加 include：

```cpp
#include "LuaScriptEngine.hpp"
```

构造顺序（现有代码基础上插入）：

```cpp
	m_pEngine->CreateDebugScene(m_pDisplaySurface);

	// 主场景此时已存在 Lua 绑定可以拿到它
	m_pLua = std::make_unique<LuaScriptEngine>(m_pEngine);

	// Shell 需要 Lua 引擎指针 用于注册 lua.run
	m_pShell = std::make_unique<Shell>(m_pEngine, m_pLua.get());

	...

	// 运行入口脚本（可选）
	m_pLua->RunFile("./scripts/main.lua");
```

如果 `main.lua` 负责搭场景，就把现有 `App.cpp` 里那一串 `m_pShell->Command(...)`（包括 400 个立方体的循环）删掉；如果先想对照，也可以让两边暂时共存，但会得到重复内容。

析构：

```cpp
App::~App()
{
	// 先销毁脚本和 Shell，再考虑销毁 Engine
	m_pShell.reset();
	m_pLua.reset();

	SDL_Quit();
}
```

当前 `App` 没有销毁 `LT::Engine`（`m_pEngine` 是裸指针），脚本清理发生在 Engine 仍然有效的阶段。以后补上 Engine 销毁时，必须保证 `m_pLua`、`m_pShell` 都先于 Engine 释放。

### 5.3 帧循环

在 SDL 事件处理之后、`UpdateDebugScene` 之前插入：

```cpp
	// dt 用上一帧的毫秒数换算；限制单帧最大步长，避免暂停/断点后数值跳变
	m_pLua->OnUpdate(std::min(dDeltaTime, 100.0) / 1000.0);
```

（需要 `#include <algorithm>`。）

放在这个位置的原因：

- 脚本在这一帧的渲染之前生效，本帧就能看到结果。
- `lua.run` 在 `Shell::Flush()` 里执行，而 `Flush()` 之前已经 `WaitIdel()`，所以脚本创建资源时 GPU 是空闲的。
- `on_update` **不要**每帧都 `WaitIdel()`，代价太高。约定 `on_update` 只做变换/灯光颜色这类轻量修改；创建/删除资源走 `lua.run` 或单独的"结构性修改"入口。

## 6. 接入 Shell：`lua.run`

### 6.1 Shell 头文件

```cpp
class LuaScriptEngine; // 前向声明

class Shell {
	...
private:
	LT::Engine* m_pEngine;
	LuaScriptEngine* m_pLua = nullptr;
	...
public:
	Shell(LT::Engine* pEngine, LuaScriptEngine* pLua = nullptr);
	...
};
```

### 6.2 构造与命令注册

`Shell.cpp`：

```cpp
#include "LuaScriptEngine.hpp"

Shell::Shell(LT::Engine* pEngine, LuaScriptEngine* pLua)
	: m_pEngine(pEngine)
	, m_pLua(pLua)
{
	...
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) -> LT::EngineResult {
			if (vecParams.size() < 1 || vecParams[0].m_eType != LT::DataType::eString || !m_pLua)
			{
				return LT::EngineResult::eInvalidParam;
			}
			return m_pLua->RunFile(vecParams[0].m_str)
				? LT::EngineResult::eSuccess
				: LT::EngineResult::eFailed;
		},
		"lua", "run"
	);
}
```

使用：

```
lua.run "./scripts/demo.lua"
```

为什么不需要额外队列：此回调由 `Shell::Flush()` 在主线程执行，Shell 线程只负责解析 `lua.run ...` 并入队。执行期间 `m_mtxCmdQueue` 处于锁定状态，所以：

- 脚本执行会阻塞 Shell 线程继续入队（可接受）。
- **脚本里不能回调 Shell 的任何方法**（尤其 `Command()`），否则死锁。

## 7. 示例脚本

`scripts/main.lua`，把现有 `App.cpp` 里的场景搬到脚本：

```lua
-- 与 App.cpp 的 debug scene 共用主场景
-- Engine 的 debug 相机会自动环绕 这里只搭内容

math.randomseed(20261127)

local img = engine.img.file("./TestAsset/UVTest.png")
local mtl = engine.mtl.create("Phong")
engine.mtl.prop(mtl, "TexDiffuse", img)
engine.mtl.prop(mtl, "ColorDiffuse", vec3(1.0, 1.0, 1.0))
engine.mtl.prop(mtl, "Roughness", 0.5)
engine.mtl.prop(mtl, "IOR", 1.5)
engine.mtl.prop(mtl, "Metalness", 0)
engine.mtl.prop(mtl, "ColorSpecular", vec3(1.0, 1.0, 1.0))

local light = engine.light.create("LightDirectional")
engine.light.color(light, vec3(1.0, 1.0, 1.0))

-- 球面均匀采样 半径 20
local cubes = {}
for i = 1, 400 do
	local node = engine.node.mesh.cube(1.0)
	engine.node.mtl(node, mtl)

	local u = math.random() * 2.0 - 1.0
	local phi = math.random() * math.pi * 2.0
	local r = 20.0 * math.sqrt(1.0 - u * u)

	local rx = math.random() * math.pi * 2.0
	local ry = math.random() * math.pi * 2.0
	local rz = math.random() * math.pi * 2.0

	engine.node.position(node, vec3(r * math.cos(phi), 20.0 * u, r * math.sin(phi)))
	engine.node.eulerXYZ(node, vec3(rx, ry, rz))

	cubes[i] = { node = node, rx = rx, ry = ry, rz = rz }
end

local elapsed = 0.0

function on_update(dt)
	elapsed = elapsed + dt
	local spin = elapsed * 0.3

	for _, c in ipairs(cubes) do
		engine.node.eulerXYZ(c.node, vec3(c.rx, c.ry + spin, c.rz))
	end
end
```

脚本目录与运行目录：

- sandbox 从输出目录运行（VS 调试工作目录是 `$(OutDir)`），脚本路径是相对该目录的。
- 可以仿照 `cmake/cpyasset.cmake` 把 `scripts/` 拷贝到 `$<CONFIG>/scripts`，开发期也可以直接写绝对路径或用 Shell 的 `lua.run` 加载。

## 8. 错误处理与安全

已覆盖：

- 脚本语法/运行时错误：`safe_script_file(..., sol::script_pass_on_error)` → 记录日志，不影响主循环。
- 绑定内部错误：抛 `std::runtime_error` → sol2 转成 Lua error → 被 `pcall` 或外层保护调用捕获。
- 单帧 `on_update` 错误：记录日志后继续下一帧。

还可以按需补：

- **错误节流**：同一帧持续报错会刷屏，可以记录连续错误次数，超过阈值后停用该回调。
- **库白名单**：现在只开了 base/coroutine/table/string/math；需要多文件脚本时再打开 `package`，并把 `package.path` 限定到脚本目录。
- **死循环**：Lua 没有抢占式超时。对不可信脚本需要 `lua_sethook` 指令预算，或在独立进程/线程里跑（跨线程的 Engine 调用再入队），这些超出本文范围。
- **GC**：默认自动 GC 足够；不要在 `on_update` 里频繁创建大表。

## 9. 所有权与删除（重要）

Lua 里拿到的 ID 只是数字，引擎不知道谁持有它。建议的规则：

1. **谁创建，谁负责清理**。`LuaScriptEngine` 用自己的 `m_setNode/m_setMtl/m_setImg` 记录脚本创建的对象，`Shell` 继续用它的集合。两者互不重叠。
2. **v1 不向 Lua 暴露任何 `delete`**。`Engine::DeleteSceneNode` / `DeleteMaterial` / `DeleteImage` 都不绑定。脚本只做创建和修改。
3. **v1 只把节点挂到主场景**（即绑定里的 parent 参数省略）。在脚本里用显式 parent 造子树、或跨层重挂节点（Shell 创建的对象挂到 Lua 创建的对象下，或反过来），都会让两边的清理顺序产生悬空指针。

### 现有删除接口的坑

- `Engine::DeleteSceneNode`（`vkRenderer/core/Engine.cpp:532`）直接调用 `SceneManager::ReleaseNode`，**不会把节点从父节点的 children 列表里摘除**。
- `SceneManager::ReleaseNode`（`vkRenderer/core/scene/SceneManager.cpp:117`）只从 `m_mapNodes` 里删掉被删的那个 ID；而 `Node::~Node`（`vkRenderer/core/scene/Node.cpp:16`）会级联析构子节点——这些子节点的 ID 仍然留在 `m_mapNodes` 里，成为悬空项。

后果：

- 直接删一个父节点 → 子节点被析构，但 `m_mapNodes` 里残留悬空指针；之后再按子节点 ID 删除会二次释放。
- 先删子节点 → 父节点的 `m_listChildren` 里留下悬空指针；之后父节点析构会再次 delete 子节点。

Shell 现在能正常退出，是因为它只删主场景下的扁平子节点，而且主场景/Engine 在进程退出前不会被析构。Lua v1 沿用同样的假设：脚本节点默认直接挂主场景、不开放删除；如果使用显式 parent 造了子树，要先把引擎删除语义修好再做清理。

如果以后要做热重载或允许脚本任意嵌套，**先修引擎的删除语义**。修复方向（示意）：

```cpp
// vkRenderer 侧 不在本次脚本接入范围内
void SceneManager::ReleaseNode(NodeID id)
{
	auto iter = m_mapNodes.find(id);
	if (iter == m_mapNodes.end())
	{
		return;
	}

	Node* pNode = iter->second;

	// 1. 先从父节点摘下 避免父节点析构时再次 delete
	if (Node* pParent = pNode->GetParent())
	{
		pParent->RemoveChild(pNode);
	}

	// 2. 先递归释放子节点（拷贝列表 因为 RemoveChild 会修改原列表）
	std::vector<Node*> vecChildren(pNode->GetChildren().begin(), pNode->GetChildren().end());
	for (Node* pChild : vecChildren)
	{
		ReleaseNode(pChild->GetID());
	}

	// 3. 此时 pNode 已经没有子节点 直接删除并从映射中移除
	delete pNode;
	m_mapNodes.erase(iter);
}
```

修好之后，`CleanupCreatedObjects` 和热重载的清理才真正可靠。

## 10. 后续扩展

### 10.1 热重载

要做得干净，需要一个入口路径和"重建环境"的流程：

1. 保存入口文件路径（`m_strEntryFile`）。
2. 清理脚本创建的引擎对象（依赖第 9 节的引擎修复）。
3. 重建 Lua VM：`m_lua = sol::state{};`（`sol::state` 可移动赋值），重新 `InitState()`。
4. 重新 `RunFile(m_strEntryFile)`。

注意：重建 VM 后，之前导出的 `sol::function`、`sol::object`、`sol::environment` 全部失效，不能跨重建保存。脚本的全局状态也会丢，这是符合预期的。

开发期更省事的做法是只提供 `lua.reload` Shell 命令，重跑入口脚本，接受对象重复（或先手动关掉重复部分）。

### 10.2 面向对象的节点封装

现在脚本写 `engine.node.position(node, vec3(...))`，后续可以加一层句柄：

```lua
local n = engine.node.mesh.cube(1.0)
n:position(vec3(0, 1, 0))
n:set_material(mtl)
```

实现方式：注册一个只含 `LT::NodeID` 的 `NodeHandle` usertype，方法转发到 Engine。注意句柄无法知道节点是否已被删除，仍然需要第 9 节的规则。

### 10.3 多脚本

全局 `on_update` 只有一个，后加载的脚本会覆盖先前的定义。需要多脚本时：

- 每个脚本用 `sol::environment` 隔离执行；或
- 改成显式注册：`engine.on_update(function(dt) ... end)`，C++ 侧用 `std::vector<sol::protected_function>` 保存并按注册顺序调用。

### 10.4 与 Shell 的关系

`doc/CmdDesign.md` 的目标是交互式命令行。Lua 接好之后有两种演进路线：

- Shell 继续做"调试命令"，Lua 负责场景内容，二者共存（本文方案）。
- 逐步把 Shell 的命令实现搬到 Lua，Shell 只保留输入和补全（后续再评估）。

## 11. 检查清单

- [ ] `LuaScriptEngine.hpp/.cpp` 位于 `sandbox/` 根目录（或已更新 CMake 的 GLOB）。
- [ ] `App` 在 `CreateDebugScene` 之后创建 `LuaScriptEngine`，在 `UpdateDebugScene` 之前调用 `OnUpdate`。
- [ ] `App` 析构时先释放 Shell / Lua，再考虑释放 Engine。
- [ ] `SOL_ALL_SAFETIES_ON` 在所有包含 sol2 的编译单元中一致。
- [ ] 脚本执行入口只有主线程：`RunFile/RunString/OnUpdate` 没有被 Shell 线程或其它线程调用。
- [ ] Lua 绑定中没有调用 `Shell::Command()`。
- [ ] 绑定层错误用 `std::runtime_error`，Engine 失败用 `ThrowIfFailed` 转换。
- [ ] 脚本创建的节点/材质/贴图有记录，退出时按节点 → 材质 → 贴图清理。
- [ ] v1 不向脚本暴露删除接口；不在脚本里构造嵌套节点。
- [ ] 错误路径验证：文件不存在、语法错误、`mtl.prop` 未知属性、类型不匹配、Engine 返回失败。
