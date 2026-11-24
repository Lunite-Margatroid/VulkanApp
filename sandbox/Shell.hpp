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
		std::string m_str;
		LT::DataType m_eType;
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
	
	LT::Engine* m_pEngine;

	CmdTree m_cmdTree;
	
	std::queue<Cmd> m_qCmd;

	std::map<std::string, ShellVar> m_mapVar;

	std::atomic<bool> m_bShellRun;

	std::thread m_thrShell;

private:
	// 依赖模板参数的false 用于在不支持的模板分支触发static_assert
	template<typename>
	static constexpr bool s_bAlwaysFalse = false;

	template<typename ... Args>
	void RegisterCommand(FuncEngineCall func, Args ... keys) {
		std::vector<std::string_view> vecKeys;
		((vecKeys.emplace_back(keys)), ... );
		m_cmdTree.AddFunc(vecKeys, func);
	}
public:
	Shell(LT::Engine* pEngine);
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
	
};