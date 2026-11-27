#pragma once

#ifndef  SOL_ALL_SAFETIES_ON
#define SOL_ALL_SAFETIES_ON 1
#endif // ! SOL_ALL_SAFETIES_ON

#include "sol/sol.hpp"

#include <array>
#include <cstdint>
#include <set>
#include <string_view>
#include <variant>

#include "TypeDef.hpp"

// lua脚本
namespace LT
{
	class Engine;
}

class LuaScriptEngine {
private:

	LT::Engine* m_pEngine = nullptr;

	sol::state m_lua;

	LT::NodeID m_nMainScene = LT::INVALID_NODE_ID;

	// 脚本创建的对象
	std::set<LT::NodeID> m_setNode;
	std::set<LT::ImageID> m_setImage;
	std::set<LT::MaterialID> m_setMtl;


private:
	void InitState();
	void RegisterCoreType();
	void RegisterBinding();
	void ClearUpObjects();
public:
	explicit LuaScriptEngine(LT::Engine* pEngine);
	~LuaScriptEngine();

	LuaScriptEngine(const LuaScriptEngine&) = delete;
	LuaScriptEngine& operator = (const 	LuaScriptEngine&) = delete;

	bool RunFile(std::string_view strFilePath);
	bool RunString(std::string_view strScript, std::string_view strChunkName);

	void SetMainSceneNode(LT::NodeID nNode);

};
