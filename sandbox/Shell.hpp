#pragma once
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include "EngineCommon.h"
#include "Engine.h"


#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include <set>


class LuaScriptEngine;

class Shell{

public:
	static constexpr std::string_view SELECTED_S = "SELECTED_S"; // 当前场景
	static constexpr std::string_view SELECTED_N = "SELECTED_N";// 当前选中的节点ID
	static constexpr std::string_view SELECTED_MTL = "SELECTED_MTL";// 当前选中的材质ID
	static constexpr std::string_view SELECTED_MESH = "SELECTED_MESH";// 当前选中的Mesh的ID
	static constexpr std::string_view SELECTED_V = "SELECTED_V";// 当前选中的视图ID
	static constexpr std::string_view SELECTED_D = "SELECTED_D";// 当前选中的显示设备ID
	static constexpr std::string_view SELECTED_IMG = "SELECTED_IMG";// 当前选中的图像ID

private:

	// 参数/变量 携带明确的数据类型
	struct ShellVar {
		LT::TValue m_value;
		// 字符串值 或 变量引用(eCustomedType0)时保存m_mapVar中的变量名
		std::string m_str;
		// 数据类型 eCustomedType0表示对m_mapVar中变量的引用
		LT::DataType m_eType;

		// 是否是变量引用
		bool IsVarRef() const {
			return m_eType == LT::DataType::eCustomedType0;
		}
	};

	// 命令回调 参数为带类型的Shell变量列表
	using FuncEngineCall = std::function<LT::EngineResult(const std::vector<ShellVar>&)>;

	struct CmdTree {
		std::string m_strKey;
		std::list<CmdTree> m_listChildren;
		FuncEngineCall m_func;

		CmdTree();
		CmdTree(std::string_view strKey, FuncEngineCall func);
		
		bool AddFunc(std::span<std::string_view> keys, FuncEngineCall func);

		template<typename TStrKey>
		FuncEngineCall GetFunc(const TStrKey& strKey) {
			for (auto & cmdTree : m_listChildren)
			{
				if (cmdTree.m_strKey == strKey)
				{
					return cmdTree.m_func;
				}
			}
			return nullptr;
		}

		// 多级key查找 例如 {"node", "position"}
		FuncEngineCall GetFunc(std::span<const std::string_view> keys);

		template<typename TStrKey, typename ... Args>
		FuncEngineCall GetFunc(const TStrKey& strKey, Args ... args) {

			for (auto& cmdTree : m_listChildren)
			{
				if (cmdTree.m_strKey == strKey)
				{
					return cmdTree.GetFunc(args...);
				}
			}
			return nullptr;
		}
	};

	struct Cmd {
		FuncEngineCall m_func;
		std::vector<ShellVar> m_vecParams;
		LT::EngineResult operator () () {
			if(m_func)
				return m_func(m_vecParams);
			return LT::EngineResult::eFailed;
		}

		Cmd() 
			: m_func(nullptr)
		{}
	};
	
	// 没有所有权
	LT::Engine* m_pEngine;
	// 没有所有权
	LuaScriptEngine* m_pLua;

	CmdTree m_cmdTree;
	
	std::queue<Cmd> m_qCmd;
	std::mutex m_mtxCmdQueue;

	std::map<std::string, ShellVar> m_mapVar;

	std::atomic<bool> m_bShellRun;

	std::thread m_thrShell;

	std::set<LT::NodeID> m_setNode;
	std::set<LT::ImageID> m_setImg;
	std::set<LT::MaterialID> m_setMtl;

private:
	// 依赖模板参数的false 用于在不支持的模板分支触发static_assert
	template<typename>
	static constexpr bool s_bAlwaysFalse = false;

	// 解析参数指向的实际变量：若参数是变量引用(eCustomedType0)则到m_mapVar中查找
	// 解析成功时顺带把引用链上各变量的引用写回为实际值 尽量减少m_mapVar中的引用
	// 引用的变量不存在或引用链成环时返回nullptr
	ShellVar* ResolveParam(ShellVar* pVar) {
		ShellVar* arrVisited[16];
		size_t nVisited = 0;
		while (nVisited < 16 && pVar != nullptr && pVar->IsVarRef()) {
			arrVisited[nVisited++] = pVar;
			auto iter = m_mapVar.find(pVar->m_str);
			pVar = iter == m_mapVar.end() ? nullptr : &iter->second;
		}
		if (pVar == nullptr || pVar->IsVarRef()) {
			return nullptr;
		}
		// 引用链上的变量全部写回为实际值
		for (size_t i = 0; i < nVisited; ++i) {
			if (arrVisited[i] != pVar) {
				*arrVisited[i] = *pVar;
			}
		}
		return pVar;
	}

	const ShellVar* ResolveParam(const ShellVar* pVar) const {
		for (size_t i = 0; i < 16; ++i) {
			if (pVar == nullptr || !pVar->IsVarRef()) {
				return pVar;
			}
			auto iter = m_mapVar.find(pVar->m_str);
			pVar = iter == m_mapVar.end() ? nullptr : &iter->second;
		}
		return nullptr;
	}

	// 获取变量的当前值 尽量减少m_mapVar中的引用
	// 变量本身若存的是引用(别名) 解析时会把引用链上的变量写回为实际值
	// 变量不存在或引用无法解析时返回nullptr
	ShellVar* GetVarValue(std::string_view strVarName) {
		auto iter = m_mapVar.find(std::string(strVarName));
		if (iter == m_mapVar.end()) {
			return nullptr;
		}
		if (iter->second.IsVarRef()) {
			return ResolveParam(&iter->second);
		}
		return &iter->second;
	}

	// ---------------- 命令参数辅助 ----------------
	// 参数转float 接受int/int64/float
	static bool GetFloatValue(const ShellVar& var, float& fOut);
	// 参数转int64 只接受整数
	static bool GetIntValue(const ShellVar& var, int64_t& nOut);
	// 读取Shell变量中的节点ID 变量不存在或类型不符时返回INVALID_NODE_ID
	LT::NodeID GetNodeVarID(std::string_view strVarName);
	// 把节点挂到当前场景(SELECTED_S)下 SELECTED_S缺失时不挂载
	void AttachToSelectedScene(LT::NodeID nNode);

	template<typename ... Args>
	void RegisterCommand(FuncEngineCall func, Args ... keys) {
		std::vector<std::string_view> vecKeys;
		((vecKeys.emplace_back(keys)), ... );
		m_cmdTree.AddFunc(vecKeys, func);
	}
public:
	Shell(LT::Engine* pEngine, LuaScriptEngine* pLua);
	~Shell();
	
	// 字符串 "hello world"
	// 向量 "vec(0.2, 1, 4)" "vec2, vec3, vec4" 默认解析为浮点数
	// 整数 默认解析为int64
	// 浮点数
	// 变量赋值 "var_name = <值>" 例如 "x = vec(1, 2, 3)"
	// 变量使用 "node.position $x"
	void Command(std::string_view strCmd);

	// 设置Shell变量 供外部代码直接写入m_mapVar
	// 支持:
	//   字符串 std::string / std::string_view / const char* / 字符串字面量
	//   整数   int(int分支) / int64_t / 其它整数类型(统一按int64)
	//   浮点   float(float分支) / 其它浮点类型(统一转float)
	//   向量   std::array<float,2/3/4> / std::array<int,2/3/4>
	template<typename TypeVar>
	void SetShellVar(std::string_view strVarName, const TypeVar& value)
	{
		using T = std::remove_cvref_t<TypeVar>;

		ShellVar var;

		if constexpr (std::is_same_v<T, std::string>)
		{
			var.m_eType = LT::DataType::eString;
			var.m_str = value;
		}
		else if constexpr (std::is_same_v<T, std::string_view>)
		{
			var.m_eType = LT::DataType::eString;
			if (!value.empty())
			{
				var.m_str.assign(value.data(), value.size());
			}
		}
		else if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>)
		{
			var.m_eType = LT::DataType::eString;
			var.m_str = value;
		}
		else if constexpr (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>)
		{
			// 字符串字面量 char[N]
			var.m_eType = LT::DataType::eString;
			var.m_str = value;
		}
		else if constexpr (std::is_same_v<T, int>)
		{
			var.m_eType = LT::DataType::eInt;
			var.m_value = value;
		}
		else if constexpr (std::is_same_v<T, int64_t>)
		{
			var.m_eType = LT::DataType::eInt64;
			var.m_value = value;
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			var.m_eType = LT::DataType::eFloat;
			var.m_value = value;
		}
		else if constexpr (std::is_same_v<T, std::array<float, 2>>)
		{
			var.m_eType = LT::DataType::eFloat2;
			var.m_value = value;
		}
		else if constexpr (std::is_same_v<T, std::array<float, 3>>)
		{
			var.m_eType = LT::DataType::eFloat3;
			var.m_value = value;
		}
		else if constexpr (std::is_same_v<T, std::array<float, 4>>)
		{
			var.m_eType = LT::DataType::eFloat4;
			var.m_value = value;
		}
		else if constexpr (std::is_same_v<T, std::array<int, 2>>)
		{
			var.m_eType = LT::DataType::eInt2;
			var.m_value = value;
		}
		else if constexpr (std::is_same_v<T, std::array<int, 3>>)
		{
			var.m_eType = LT::DataType::eInt3;
			var.m_value = value;
		}
		else if constexpr (std::is_same_v<T, std::array<int, 4>>)
		{
			var.m_eType = LT::DataType::eInt4;
			var.m_value = value;
		}
		else if constexpr (std::is_integral_v<T>)
		{
			// 其余整数类型统一按int64存储
			var.m_eType = LT::DataType::eInt64;
			var.m_value = static_cast<int64_t>(value);
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			// 其余浮点类型统一转成float
			var.m_eType = LT::DataType::eFloat;
			var.m_value = static_cast<float>(value);
		}
		else
		{
			static_assert(s_bAlwaysFalse<T>, "SetShellVar: unsupported variable type");
		}

		m_mapVar[std::string(strVarName)] = std::move(var);
	}

	bool Run();

	void Execute();
	void Terminate();

	bool Flush();
	bool HasCmd();

	void Echo(std::string_view strEcho);
};