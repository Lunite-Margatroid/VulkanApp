#include "EngineCommon.h"
#include "LuaScriptEngine.hpp"
#include "vulkan/vulkan.hpp"
#include "Engine.h"

#include <array>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "RandomGenerator.hpp"

namespace
{
	struct LuaVec3 {
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
	};

	std::array<float, 3> ToArray(const LuaVec3& vec3) {
		return { vec3.x, vec3.y, vec3.z };
	}

	const char* ToString(LT::EngineResult eResult) {
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

	void EngineCallCheck(LT::EngineResult nResult) {
		if (nResult != LT::EngineResult::eSuccess)
		{
			LOG_WARNING("Lua. Engine Call Failed. error code: %s", ToString(nResult));
		}
	}

	// 材质类型名到枚举的映射
	const std::map<std::string, LT::MaterialType>& GetMtlTypeMap() {
		static const std::map<std::string, LT::MaterialType> s_map = []() {
			std::map<std::string, LT::MaterialType> map;
#define X(MtlType) map[#MtlType] = LT::MaterialType::e##MtlType;
			MaterialTypeList(X)
#undef X
			return map;
		}();
		return s_map;
	}

	// 灯光类型名到枚举的映射
	const std::map<std::string, LT::LightType>& GetLightTypeMap() {
		static const std::map<std::string, LT::LightType> s_map = []() {
			std::map<std::string, LT::LightType> map;
#define X(PLightType) map[#PLightType] = LT::LightType::e##PLightType;
			LightTypeList(X)
#undef X
			return map;
		}();
		return s_map;
	}

	// 材质属性名到枚举的映射
	const std::map<std::string, LT::MtlProp>& GetMtlPropMap() {
		static const std::map<std::string, LT::MtlProp> s_map = []() {
			std::map<std::string, LT::MtlProp> map;
#define X(eProp, eType) map[#eProp] = LT::MtlProp::e##eProp;
			MtlPropList(X)
#undef X
			return map;
		}();
		return s_map;
	}
}

void LuaScriptEngine::InitState()
{
	// 初始化lua状态
	// 打开需要的lib
	m_lua.open_libraries(
		sol::lib::base,
		sol::lib::coroutine,
		sol::lib::table,
		sol::lib::string,
		sol::lib::math
	);

	RegisterCoreType();
	RegisterBinding();
}

void LuaScriptEngine::RegisterCoreType()
{
	m_lua.new_usertype<LuaVec3>("Vec3",
		sol::constructors<LuaVec3(), LuaVec3(float, float, float)>(),
		"x", &LuaVec3::x,
		"y", &LuaVec3::y,
		"z", &LuaVec3::z
	);

	m_lua.set_function("vec3", [](float x,float y,float z) {
		return LuaVec3{x,y,z};
		});

	m_lua.set_function("log", [](const std::string& strLog) {
		LOG_INFO("%s", strLog.c_str());
		}
	);
}

void LuaScriptEngine::RegisterBinding()
{
	sol::table engine = m_lua["engine"].get_or_create<sol::table>();

	// ---------- img -------------
	sol::table img = engine["img"].get_or_create<sol::table>();
	img.set_function("file", [this](const std::string& strFilePath)->int64_t {
		LT::ImageID nImg = LT::INVALID_IMAGE_ID;
		EngineCallCheck(
			m_pEngine->LoadImageFromFile(nImg, strFilePath)
		);
		if (nImg != LT::INVALID_IMAGE_ID)
		{
			m_setImage.insert(nImg);
		}
		return nImg;
		}
	);

	// ---------- mtl -------------
	sol::table mtl = engine["mtl"].get_or_create<sol::table>();

	mtl.set_function("create", [this](const std::string& strType)->int64_t {
		auto iter = GetMtlTypeMap().find(strType);
		if (iter == GetMtlTypeMap().end())
		{
			LOG_WARNING("Lua. Unknown material type: %s", strType.c_str());
			return LT::INVALID_MATERIAL_ID;
		}

		LT::MaterialID nMtl = LT::INVALID_MATERIAL_ID;
		EngineCallCheck(m_pEngine->CreateMaterial(nMtl, iter->second));
		if (nMtl != LT::INVALID_MATERIAL_ID)
		{
			m_setMtl.insert(nMtl);
		}
		return nMtl;
		}
	);

	// mtl.prop <材质ID> <属性名> <值>
	// 值类型由属性本身的类型决定
	mtl.set_function("prop", [this](int64_t nMtl, const std::string& strProp, sol::object objValue) {
		auto iter = GetMtlPropMap().find(strProp);
		if (iter == GetMtlPropMap().end())
		{
			LOG_WARNING("Lua. Unknown material prop: %s", strProp.c_str());
			return;
		}

		LT::MtlPropVar vValue;
		switch (LT::GetMtlPropDataType(iter->second))
		{
			case LT::MtlPropDataType::eBool:
			case LT::MtlPropDataType::eInt:
			case LT::MtlPropDataType::eImage:
				if (!objValue.is<int64_t>())
				{
					LOG_WARNING("Lua. Material prop %s expects an integer", strProp.c_str());
					return;
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
					// 与Shell一致 整数字面量允许赋给浮点属性
					vValue = static_cast<float>(objValue.as<int64_t>());
				}
				else
				{
					LOG_WARNING("Lua. Material prop %s expects a number", strProp.c_str());
					return;
				}
				break;

			case LT::MtlPropDataType::eFloat3:
				if (!objValue.is<LuaVec3>())
				{
					LOG_WARNING("Lua. Material prop %s expects a vec3", strProp.c_str());
					return;
				}
				vValue = ToArray(objValue.as<LuaVec3>());
				break;

			default:
				LOG_WARNING("Lua. Material prop type not supported: %s", strProp.c_str());
				return;
		}

		EngineCallCheck(m_pEngine->SetMaterialProp(nMtl, iter->second, vValue));
		}
	);

	// ---------- node ---------------
	sol::table node = engine["node"].get_or_create<sol::table>();

	node.set_function("position", [this](int64_t nNodeID, const LuaVec3& pos) {
		EngineCallCheck(m_pEngine->SetSceneNodePosition(nNodeID, ToArray(pos)));
		});

	node.set_function("eulerXYZ", [this](int64_t nNodeID, const LuaVec3& euler) {
		EngineCallCheck(m_pEngine->SetSceneNodeEulerRotation(nNodeID, ToArray(euler)));
		});

	node.set_function("scale", [this](int64_t nNodeID, const LuaVec3& scale) {
		EngineCallCheck(m_pEngine->SetSceneNodeScale(nNodeID, ToArray(scale)));
		});

	node.set_function("mtl", [this](int64_t nNodeID, int64_t nMtlID) {
		EngineCallCheck(m_pEngine->SetMaterial(nNodeID, nMtlID));
		});

	// ---------- node.mesh -------------
	sol::table mesh = node["mesh"].get_or_create<sol::table>();

	mesh.set_function("cube", [this](float fSideLength, sol::optional<int64_t> optParent)->int64_t {
		LT::NodeID nNode = LT::INVALID_NODE_ID;
		EngineCallCheck(m_pEngine->CreateCubeMeshNode(nNode, fSideLength));
		if (nNode == LT::INVALID_NODE_ID)
		{
			return nNode;
		}
		// 先记录 保证后续步骤失败也能被清理
		m_setNode.insert(nNode);

		LT::NodeID nParent = optParent.value_or(m_nMainScene);
		if (nParent != LT::INVALID_NODE_ID)
		{
			EngineCallCheck(m_pEngine->SceneNodeRebase(nParent, nNode));
		}
		return nNode;
		}
	);

	mesh.set_function("sphere", [this](float fRadius, uint32_t nLongSubdivision, uint32_t nLatSubdivision,
		sol::optional<int64_t> optParent)->int64_t {
		LT::NodeID nNode = LT::INVALID_NODE_ID;
		EngineCallCheck(m_pEngine->CreateSphereMeshNode(nNode, fRadius, nLongSubdivision, nLatSubdivision));
		if (nNode == LT::INVALID_NODE_ID)
		{
			return nNode;
		}
		m_setNode.insert(nNode);

		LT::NodeID nParent = optParent.value_or(m_nMainScene);
		if (nParent != LT::INVALID_NODE_ID)
		{
			EngineCallCheck(m_pEngine->SceneNodeRebase(nParent, nNode));
		}
		return nNode;
		}
	);

	// ---------- light -------------
	sol::table light = engine["light"].get_or_create<sol::table>();

	light.set_function("create", [this](const std::string& strType, sol::optional<int64_t> optParent)->int64_t {
		auto iter = GetLightTypeMap().find(strType);
		if (iter == GetLightTypeMap().end())
		{
			LOG_WARNING("Lua. Unknown light type: %s", strType.c_str());
			return LT::INVALID_NODE_ID;
		}

		LT::NodeID nNode = LT::INVALID_NODE_ID;
		EngineCallCheck(m_pEngine->CreateLightNode(nNode, optParent.value_or(m_nMainScene), iter->second));
		if (nNode != LT::INVALID_NODE_ID)
		{
			m_setNode.insert(nNode);
		}
		return nNode;
		}
	);

	light.set_function("color", [this](int64_t nNodeID, const LuaVec3& color) {
		EngineCallCheck(m_pEngine->SetLightColor(nNodeID, ToArray(color)));
		});

	light.set_function("range", [this](int64_t nNodeID, float fRange) {
		EngineCallCheck(m_pEngine->SetLightRange(nNodeID, fRange));
		});

	light.set_function("radius", [this](int64_t nNodeID, float fRadius) {
		EngineCallCheck(m_pEngine->SetLightRadius(nNodeID, fRadius));
		});

	light.set_function("rectSize", [this](int64_t nNodeID, float fU, float fV) {
		EngineCallCheck(m_pEngine->SetLightRectSize(nNodeID, std::array<float, 2>{ fU, fV }));
		});

	light.set_function("spotRadian", [this](int64_t nNodeID, float fInner, float fOuter) {
		EngineCallCheck(m_pEngine->SetLightSpotRadian(nNodeID, std::array<float, 2>{ fInner, fOuter }));
		});
}

bool LuaScriptEngine::RunFile(std::string_view strFilePath)
{
	sol::protected_function_result result =
		m_lua.safe_script_file(std::string(strFilePath), sol::script_pass_on_error);

	if (!result.valid())
	{
		sol::error err = result;
		LOG_ERROR("Lua. Run file failed (%s): %s", std::string(strFilePath).c_str(), err.what());
		return false;
	}
	return true;
}

bool LuaScriptEngine::RunString(std::string_view strScript, std::string_view strChunkName)
{
	sol::protected_function_result result =
		m_lua.safe_script(strScript, sol::script_pass_on_error, std::string(strChunkName));

	if (!result.valid())
	{
		sol::error err = result;
		LOG_ERROR("Lua. Run string failed: %s", err.what());
		return false;
	}
	return true;
}

void LuaScriptEngine::SetMainSceneNode(LT::NodeID nNode)
{
	m_nMainScene = nNode;
}

void LuaScriptEngine::ClearUpObjects()
{
	for (LT::NodeID node : m_setNode)
	{
		m_pEngine->DeleteSceneNode(node);
	}
	m_setNode.clear();

	for (LT::MaterialID mtl : m_setMtl)
	{
		m_pEngine->DeleteMaterial(mtl);
	}
	m_setMtl.clear();

	for (LT::ImageID img : m_setImage)
	{
		m_pEngine->DeleteImage(img);
	}
	m_setImage.clear();
}

LuaScriptEngine::LuaScriptEngine(LT::Engine* pEngine)
	: m_pEngine(pEngine)
	, m_nMainScene(LT::INVALID_NODE_ID)
{
	RENDERER_ASSERT(pEngine, "Lua. Engine is null");

	// View的主场景 脚本创建的节点默认挂到它下面
	std::vector<LT::NodeID> vecRoots;
	if (m_pEngine->GetRootSceneNodes(vecRoots) == LT::EngineResult::eSuccess && !vecRoots.empty())
	{
		m_nMainScene = vecRoots.front();
	}

	InitState();
}

LuaScriptEngine::~LuaScriptEngine()
{
	ClearUpObjects();
}
