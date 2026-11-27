#include "Shell.hpp"

#include <array>
#include <charconv>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <iostream>

#include "Timer.hpp"


namespace
{
	// 材质属性名字到枚举的映射
	std::map<std::string, LT::MtlProp> g_mapMtlPropStr2Enum = []() {
		std::map<std::string, LT::MtlProp> mapStr2Enum;

#define X(eProp, eType)	mapStr2Enum[#eProp] = LT::MtlProp::e##eProp;
		MtlPropList(X)
#undef X

		return mapStr2Enum;
		}();

	// 材质类型名到枚材质类型举的映射
	std::map<std::string, LT::MaterialType> g_mapMtlTypeStr2Enum = []() {
		std::map<std::string, LT::MaterialType> mapStr2Enum;

#define X(MtlType) mapStr2Enum[#MtlType] = LT::MaterialType::e##MtlType;
		MaterialTypeList(X)
#undef X

		return mapStr2Enum;
		
		}();

	// 灯光类型名到灯光类型枚举的映射
	std::map<std::string, LT::LightType> g_mapLightTypeStr2Enum = []() {
		std::map<std::string, LT::LightType> mapStr2Enum;
#define X(PLightType) mapStr2Enum[#PLightType] = LT::LightType::e##PLightType;
		LightTypeList(X)
#undef X
			return mapStr2Enum;
		}();

	// 判断Shell参数类型与材质属性期望的数据类型是否匹配
	bool IsMtlPropTypeMatch(LT::MtlPropDataType ePropType, LT::DataType eParamType)
	{
		const bool bIsInt = (eParamType == LT::DataType::eInt || eParamType == LT::DataType::eInt64);

		switch (ePropType)
		{
		case LT::MtlPropDataType::eBool:
		case LT::MtlPropDataType::eInt:
		case LT::MtlPropDataType::eImage:
			return bIsInt;
		case LT::MtlPropDataType::eInt2:
			return eParamType == LT::DataType::eInt2;
		case LT::MtlPropDataType::eInt3:
			return eParamType == LT::DataType::eInt3;
		case LT::MtlPropDataType::eInt4:
			return eParamType == LT::DataType::eInt4;
		case LT::MtlPropDataType::eFloat:
			return eParamType == LT::DataType::eFloat;
		case LT::MtlPropDataType::eFloat2:
			return eParamType == LT::DataType::eFloat2;
		case LT::MtlPropDataType::eFloat3:
			return eParamType == LT::DataType::eFloat3;
		case LT::MtlPropDataType::eFloat4:
			return eParamType == LT::DataType::eFloat4;
		default:
			return false;
		}
	}

	// 空白字符
	constexpr std::string_view STR_WHITESPACE = " \t\r\n";

	// 去掉首尾空白字符
	std::string_view Trim(std::string_view strView)
	{
		size_t nBegin = strView.find_first_not_of(STR_WHITESPACE);
		if (nBegin == std::string_view::npos)
		{
			return {};
		}
		size_t nEnd = strView.find_last_not_of(STR_WHITESPACE);
		return strView.substr(nBegin, nEnd - nBegin + 1);
	}

	// 解析整数 整个token必须都是整数（允许前导'+'）
	// 统一按64位读取 对应TValue中的int64_t 与NodeID等句柄类型一致
	bool ParseInt(std::string_view strToken, int64_t& nOut)
	{
		if (!strToken.empty() && strToken.front() == '+')
		{
			strToken.remove_prefix(1);
		}
		if (strToken.empty())
		{
			return false;
		}
		const char* pBegin = strToken.data();
		const char* pEnd = strToken.data() + strToken.size();
		auto [pParsed, nEc] = std::from_chars(pBegin, pEnd, nOut);
		return nEc == std::errc{} && pParsed == pEnd;
	}

	// 解析浮点数 整个token必须都是浮点数（允许前导'+'）
	bool ParseFloat(std::string_view strToken, float& fOut)
	{
		if (!strToken.empty() && strToken.front() == '+')
		{
			strToken.remove_prefix(1);
		}
		if (strToken.empty())
		{
			return false;
		}
		const char* pBegin = strToken.data();
		const char* pEnd = strToken.data() + strToken.size();
		auto [pParsed, nEc] = std::from_chars(pBegin, pEnd, fOut);
		return nEc == std::errc{} && pParsed == pEnd;
	}

	// 判断token是否是整数写法（可选符号 + 若干数字） 用于区分整数和浮点数
	bool LooksLikeInt(std::string_view strToken)
	{
		if (strToken.empty())
		{
			return false;
		}
		size_t nPos = (strToken.front() == '+' || strToken.front() == '-') ? 1 : 0;
		if (nPos == strToken.size())
		{
			return false;
		}
		for (; nPos < strToken.size(); ++nPos)
		{
			if (strToken[nPos] < '0' || strToken[nPos] > '9')
			{
				return false;
			}
		}
		return true;
	}

	// 解析以','分隔的浮点数列表 例如 "0.2, 0.3, 0.4"
	bool ParseFloatList(std::string_view strList, std::vector<float>& vecOut)
	{
		size_t nPos = 0;
		while (true)
		{
			size_t nComma = strList.find(',', nPos);
			size_t nCount = nComma == std::string_view::npos ? std::string_view::npos : nComma - nPos;
			float fValue = 0.f;
			if (!ParseFloat(Trim(strList.substr(nPos, nCount)), fValue))
			{
				return false;
			}
			vecOut.push_back(fValue);

			if (nComma == std::string_view::npos)
			{
				break;
			}
			nPos = nComma + 1;
		}
		return true;
	}

	// 把浮点分量转换成2/3/4分量的TValue 同时给出对应的数据类型
	bool MakeVecValue(const std::vector<float>& vecComp, size_t nExpectComp, LT::TValue& outValue, LT::DataType& eOutType)
	{
		// 声明了分量数时 必须与实参数量一致
		if (nExpectComp != 0 && vecComp.size() != nExpectComp)
		{
			return false;
		}

		switch (vecComp.size())
		{
		case 2:
			outValue = std::array<float, 2>{ vecComp[0], vecComp[1] };
			eOutType = LT::DataType::eFloat2;
			return true;
		case 3:
			outValue = std::array<float, 3>{ vecComp[0], vecComp[1], vecComp[2] };
			eOutType = LT::DataType::eFloat3;
			return true;
		case 4:
			outValue = std::array<float, 4>{ vecComp[0], vecComp[1], vecComp[2], vecComp[3] };
			eOutType = LT::DataType::eFloat4;
			return true;
		default:
			return false;
		}
	}
}

Shell::Shell(LT::Engine* pEngine)
	:m_pEngine(pEngine)
{
#define IS_REAL(PType) (PType == LT::DataType::eInt64 || PType == LT::DataType::eFloat || PType == LT::DataType::eInt)

#define GET_FLOAT(PVar, POutFloat) do{ \
	if(PVar.m_eType == LT::DataType::eInt64) \
		{(POutFloat) = static_cast<float>(std::get<int64_t>((PVar).m_value));}\
	else if(PVar.m_eType == LT::DataType::eInt)\
		{(POutFloat) = static_cast<float>(std::get<int>((PVar).m_value));}\
	else if(PVar.m_eType == LT::DataType::eFloat)\
		{(POutFloat) = std::get<float>((PVar).m_value);}\
	}while(false)

	// 退出
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
				return LT::EngineResult::eSuccess;
		}
		,"quit"
	);
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			return LT::EngineResult::eSuccess;
		}
		, "exit"
	);
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			return LT::EngineResult::eSuccess;
		}
		, "terminate"
	);

	// ----------------- 注册命令函数 ---------------------------
	// 回调参数为std::vector<ShellVar> 每项携带数据类型与值
	// 向量默认是float向量

	// node.position
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult{
			if (vecParams.size() < 2 || vecParams[0].m_eType != LT::DataType::eInt64 || vecParams[1].m_eType != LT::DataType::eFloat3)
			{
				return LT::EngineResult::eInvalidParam;
			}
			return m_pEngine->SetSceneNodePosition(
				std::get<LT::NodeID>(vecParams[0].m_value),
				std::get<std::array<float, 3>>(vecParams[1].m_value)
			);
		},
		"node", "position"
		);
	// img.file
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			if (vecParams.size() < 1)
				return LT::EngineResult::eInvalidParam;

			LT::ImageID nImg = LT::INVALID_IMAGE_ID;
			LT::EngineResult nResult = m_pEngine->LoadImageFromFile(nImg, vecParams[0].m_str);
			if (nImg != LT::INVALID_IMAGE_ID)
			{
				SetShellVar(SELECTED_IMG, static_cast<int64_t>(nImg));
				m_setImg.insert(nImg);
			}
			return nResult;
		},
	"img", "file"
	);

	// mtl
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			if (vecParams.size() < 1 || vecParams[0].m_eType != LT::DataType::eString)
				return LT::EngineResult::eFailed;
			LT::MaterialID nMtl = LT::INVALID_MATERIAL_ID;
			auto iter = g_mapMtlTypeStr2Enum.find(vecParams[0].m_str);
			if (iter == g_mapMtlTypeStr2Enum.end())
			{
				return LT::EngineResult::eInvalidParam;
			}

			LT::EngineResult nResult = m_pEngine->CreateMaterial(nMtl, iter->second);
			if (nMtl != LT::INVALID_MATERIAL_ID)
			{
				SetShellVar(SELECTED_MTL, static_cast<int64_t>(nMtl));
				m_setMtl.insert(nMtl);
			}
			return nResult;
		}
	,"mtl"
	);

	// mtl.prop <材质ID> <属性名> <值>
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {

			if (vecParams.size() < 3 || vecParams[1].m_eType != LT::DataType::eString)
			{
				return LT::EngineResult::eInvalidParam;
			}

			auto iter = g_mapMtlPropStr2Enum.find(vecParams[1].m_str);
			if (iter == g_mapMtlPropStr2Enum.end())
			{
				return LT::EngineResult::eInvalidParam;
			}

			LT::MtlPropDataType eDataType = LT::GetMtlPropDataType(iter->second);

			// 值类型必须与属性期望的类型匹配
			if (IsMtlPropTypeMatch(eDataType, vecParams[2].m_eType))
			{
				return m_pEngine->SetMaterialProp(
					std::get<int64_t>(vecParams[0].m_value),
					iter->second,
					vecParams[2].m_value
				);

			}
			else if (eDataType == LT::MtlPropDataType::eFloat && (vecParams[2].m_eType == LT::DataType::eInt || vecParams[2].m_eType == LT::DataType::eInt64))
			{
				// 由于标量数字默认解析为整数int64
				// 如果属性类型是浮点数，先从整数转成浮点数
				LT::TValue mtlPorpValue;
				if (vecParams[2].m_eType == LT::DataType::eInt)
				{
					mtlPorpValue = static_cast<float>(std::get<int>(vecParams[2].m_value));
				}
				else
				{
					mtlPorpValue = static_cast<float>(std::get<int64_t>(vecParams[2].m_value));
				}

				return m_pEngine->SetMaterialProp(
					std::get<int64_t>(vecParams[0].m_value),
					iter->second,
					mtlPorpValue
				);
			}
			else
			{
				return LT::EngineResult::eInvalidParam;
			}

		}
		, "mtl", "prop"
	);

	// node.light <light_type>
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			if (vecParams.size() < 1 || vecParams[0].m_eType != LT::DataType::eString)
				return LT::EngineResult::eInvalidParam;

			auto iterLightType = g_mapLightTypeStr2Enum.find(vecParams[0].m_str);
			if (iterLightType == g_mapLightTypeStr2Enum.end())
				return LT::EngineResult::eInvalidParam;

			LT::NodeID nNode = LT::INVALID_NODE_ID;
			LT::NodeID nRoot = LT::INVALID_NODE_ID;
			auto iterVar = m_mapVar.find(std::string(SELECTED_S));
			if (iterVar != m_mapVar.end()) {
				nRoot = std::get<int64_t>(iterVar->second.m_value);
			}

			LT::EngineResult nResult = LT::EngineResult::eFailed;
			nResult = m_pEngine->CreateLightNode(nNode, nRoot, iterLightType->second);
			if (nNode != LT::INVALID_NODE_ID)
			{
				SetShellVar(SELECTED_N, nNode);
				m_setNode.insert(nNode);
			}
			return nResult;
		}
		, "node", "light"
	);

	// node.light.color
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			if (vecParams.size() < 2 && (vecParams[0].m_eType != LT::DataType::eInt64 || vecParams[1].m_eType != LT::DataType::eFloat3))
			{
				return LT::EngineResult::eInvalidParam;
			}

			return m_pEngine->SetLightColor(std::get<int64_t>(vecParams[0].m_value), std::get<std::array<float,3>>(vecParams[1].m_value));

		}
		, "node", "light", "color"
	);
	// sence
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			LT::NodeID nNode = LT::INVALID_NODE_ID;
			LT::EngineResult nResult = LT::EngineResult::eFailed;

			nResult = m_pEngine->CreateSceneNode(nNode, LT::NodeType::eNode);

			if (nNode != LT::INVALID_NODE_ID)
			{
				SetShellVar(SELECTED_S, nNode);
				m_setNode.insert(nNode);
			}

			return nResult;

		}
		, "scene"
	);
	// node.mesh.cube <side_len>
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			if (vecParams.size() < 1 || !IS_REAL(vecParams[0].m_eType))
			{
				return LT::EngineResult::eInvalidParam;
			}

			LT::NodeID nNode = LT::INVALID_NODE_ID;

			float fSideLen = 1.f;
			GET_FLOAT(vecParams[0], fSideLen);

			LT::EngineResult nResult = LT::EngineResult::eFailed;
			nResult = m_pEngine->CreateCubeMeshNode(nNode, fSideLen);

			if (nNode != LT::INVALID_NODE_ID)
			{
				LT::NodeID nSceneRoot = LT::INVALID_NODE_ID;
				auto iter = m_mapVar.find(std::string(SELECTED_S));
				if (iter != m_mapVar.end())
				{
					nSceneRoot = std::get<int64_t>(iter->second.m_value);
					m_pEngine->SceneNodeRebase(nSceneRoot, nNode);
				}
				SetShellVar(SELECTED_N, nNode);
				m_setNode.insert(nNode);
			}
			return nResult;
		}
		, "node", "mesh", "cube"
		);

	// node.mtl <node_id> <mtl_id>
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			if (vecParams.size() < 2 || vecParams[0].m_eType != LT::DataType::eInt64 || vecParams[1].m_eType != LT::DataType::eInt64)
			{
				return LT::EngineResult::eInvalidParam;
			}

			return m_pEngine->SetMaterial(std::get<int64_t>(vecParams[0].m_value), std::get<int64_t>(vecParams[1].m_value));
		}
		, "node", "mtl"
	);

	// node.eulerXYZ <node_id> vec(<x>, <y>, <z>)
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			if (vecParams.size() < 2 || vecParams[0].m_eType != LT::DataType::eInt64 || vecParams[1].m_eType != LT::DataType::eFloat3)
			{
				return LT::EngineResult::eInvalidParam;
			}

			return m_pEngine->SetSceneNodeEulerRotation(
				std::get<int64_t>(vecParams[0].m_value),
				std::get<std::array<float, 3>>(vecParams[1].m_value)
			);
		}
		, "node", "eulerXYZ"
	);

	// node.scale <node_id> vec(<x>, <y>, <z>)
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			if (vecParams.size() < 2 || vecParams[0].m_eType != LT::DataType::eInt64 || vecParams[1].m_eType != LT::DataType::eFloat3)
			{
				return LT::EngineResult::eInvalidParam;
			}

			return m_pEngine->SetSceneNodeScale(
				std::get<int64_t>(vecParams[0].m_value),
				std::get<std::array<float, 3>>(vecParams[1].m_value)
			);
		}
		, "node", "scale"
	);

	// node.list
	RegisterCommand(
		[this](const std::vector<ShellVar>& vecParams) ->LT::EngineResult {
			std::ostringstream oss;
			for (LT::NodeID nNode : m_setNode)
			{
				oss << "Node: " << nNode << std::endl;
			}
			Echo(oss.view());
			return LT::EngineResult::eSuccess;
		}
		, "node", "list"
	);
}

Shell::~Shell()
{
	for (LT::NodeID nNode : m_setNode) {
		m_pEngine->DeleteSceneNode(nNode);
	}
	for (LT::MaterialID nMtl : m_setMtl)
	{
		m_pEngine->DeleteMaterial(nMtl);
	}
	for (LT::ImageID nImg : m_setImg)
	{
		m_pEngine->DeleteImage(nImg);
	}
}

void Shell::Command(std::string_view strCmd)
{
	Cmd cmd;

	// 去掉首尾空白（形参只读 使用局部变量保存处理后的命令）
	std::string_view strLine = Trim(strCmd);
	if (strLine.empty())
	{
		return;
	}

	// 分离命令名与参数字符串 "xxx.xxx.xx <param0> <param1> <param2>"
	size_t nNameEnd = strLine.find_first_of(STR_WHITESPACE);
	std::string_view strName = strLine.substr(0, nNameEnd);
	std::string_view strArgs = nNameEnd == std::string_view::npos ? std::string_view{} : strLine.substr(nNameEnd);

	// 解析失败时输出带命令名的日志
	auto LogInvalid = [strName](const char* pStrReason)
	{
		LOG_WARNING("Command \"%.*s\" failed: %s", static_cast<int>(strName.size()), strName.data(), pStrReason);
	};

	// 解析一个值（字符串 / 浮点向量 / 整数 / 浮点数 / $变量）
	// 失败时输出日志并返回false 成功时nPos前进到值的末尾
	auto ParseValue = [this, &LogInvalid](std::string_view strArgs, size_t& nPos, ShellVar& outVar) -> bool
	{
		size_t nBegin = strArgs.find_first_not_of(STR_WHITESPACE, nPos);
		if (nBegin == std::string_view::npos)
		{
			LogInvalid("missing value");
			return false;
		}
		nPos = nBegin;

		// 字符串 "..." 支持 \" 转义
		if (strArgs[nPos] == '"')
		{
			std::string strValue;
			size_t nCur = nPos + 1;
			bool bClosed = false;
			while (nCur < strArgs.size())
			{
				if (strArgs[nCur] == '\\' && nCur + 1 < strArgs.size() && strArgs[nCur + 1] == '"')
				{
					strValue.push_back('"');
					nCur += 2;
					continue;
				}
				if (strArgs[nCur] == '"')
				{
					bClosed = true;
					++nCur;
					break;
				}
				strValue.push_back(strArgs[nCur]);
				++nCur;
			}
			if (!bClosed)
			{
				LogInvalid("unclosed string");
				return false;
			}
			outVar.m_eType = LT::DataType::eString;
			outVar.m_str = std::move(strValue);
			nPos = nCur;
			return true;
		}

		// 变量引用 $name
		// 保存为对m_mapVar中变量的引用(eCustomedType0) 执行命令时再解析为实际值
		// 变量可能由同一批命令中更早执行的命令创建 因此此处不检查变量是否存在
		if (strArgs[nPos] == '$')
		{
			size_t nTokenEnd = strArgs.find_first_of(STR_WHITESPACE, nPos + 1);
			size_t nTokenLen = nTokenEnd == std::string_view::npos ? std::string_view::npos : nTokenEnd - (nPos + 1);
			std::string_view strVarName = strArgs.substr(nPos + 1, nTokenLen);
			if (strVarName.empty())
			{
				LogInvalid("empty variable name");
				return false;
			}
			outVar.m_eType = LT::DataType::eCustomedType0;
			outVar.m_str = std::string(strVarName);
			nPos = nTokenEnd;

			// 同时尽量消除m_mapVar中的引用
			// 变量本身若存的是引用(别名) 解析时把它的引用写回为实际值
			GetVarValue(strVarName);
			return true;
		}

		std::string_view strRest = strArgs.substr(nPos);
		if (strRest.substr(0, 3) == "vec")
		{
			// 浮点向量 vec(...) 或指定分量数的 vec2/vec3/vec4(...)
			size_t nPrefixLen = 3;
			size_t nExpectComp = 0;
			if (strRest.size() > 3 && strRest[3] >= '2' && strRest[3] <= '4')
			{
				nExpectComp = static_cast<size_t>(strRest[3] - '0');
				nPrefixLen = 4;
			}
			if (strRest.size() <= nPrefixLen || strRest[nPrefixLen] != '(')
			{
				LogInvalid("invalid vec param");
				return false;
			}

			size_t nListBegin = nPos + nPrefixLen + 1;
			size_t nClose = strArgs.find(')', nListBegin);
			if (nClose == std::string_view::npos)
			{
				LogInvalid("unclosed vec param");
				return false;
			}

			std::vector<float> vecComp;
			if (!ParseFloatList(strArgs.substr(nListBegin, nClose - nListBegin), vecComp) ||
				!MakeVecValue(vecComp, nExpectComp, outVar.m_value, outVar.m_eType))
			{
				LogInvalid("invalid vec components");
				return false;
			}
			nPos = nClose + 1;
			return true;
		}

		// 数字 截取到下一个空白
		size_t nTokenEnd = strArgs.find_first_of(STR_WHITESPACE, nPos);
		size_t nTokenLen = nTokenEnd == std::string_view::npos ? std::string_view::npos : nTokenEnd - nPos;
		std::string_view strToken = strArgs.substr(nPos, nTokenLen);

		int64_t nIntValue = 0;
		float fFloatValue = 0.f;
		if (LooksLikeInt(strToken))
		{
			// 整数写法只按整数解析 超出int64范围视为错误而不是退化成浮点数
			if (!ParseInt(strToken, nIntValue))
			{
				LogInvalid("integer param out of range");
				return false;
			}
			outVar.m_eType = LT::DataType::eInt64;
			outVar.m_value = nIntValue;
		}
		else if (ParseFloat(strToken, fFloatValue))
		{
			outVar.m_eType = LT::DataType::eFloat;
			outVar.m_value = fFloatValue;
		}
		else
		{
			LogInvalid("invalid param");
			return false;
		}
		nPos = nTokenEnd;
		return true;
	};

	// 变量赋值 "var_name = <value>"
	size_t nAssignPos = strArgs.find_first_not_of(STR_WHITESPACE);
	if (nAssignPos != std::string_view::npos && strArgs[nAssignPos] == '=' &&
		(nAssignPos + 1 == strArgs.size() || STR_WHITESPACE.find(strArgs[nAssignPos + 1]) != std::string_view::npos))
	{
		size_t nValuePos = nAssignPos + 1;
		ShellVar var;
		if (!ParseValue(strArgs, nValuePos, var))
		{
			return;
		}
		// 赋值指令只允许一个值
		if (strArgs.find_first_not_of(STR_WHITESPACE, nValuePos) != std::string_view::npos)
		{
			LogInvalid("unexpected token after value");
			return;
		}
		// 尽量不把引用存进m_mapVar：右值若是变量引用且当前能解析 直接存实际值
		if (var.IsVarRef())
		{
			ShellVar* pValue = GetVarValue(var.m_str);
			if (pValue != nullptr)
			{
				var = *pValue;
			}
			else if (var.m_str == strName)
			{
				// 不允许把变量赋值为对自身的引用
				LogInvalid("undefined variable");
				return;
			}
		}

		m_mapVar[std::string(strName)] = std::move(var);
		return;
	}

	// 命令名按'.'拆成多级key
	std::vector<std::string_view> vecKeys;
	size_t nKeyPos = 0;
	while (true)
	{
		size_t nDot = strName.find('.', nKeyPos);
		size_t nKeyLen = nDot == std::string_view::npos ? std::string_view::npos : nDot - nKeyPos;
		std::string_view strKey = strName.substr(nKeyPos, nKeyLen);
		if (strKey.empty())
		{
			LogInvalid("invalid command name");
			return;
		}
		vecKeys.push_back(strKey);

		if (nDot == std::string_view::npos)
		{
			break;
		}
		nKeyPos = nDot + 1;
	}

	// 从命令树获取回调
	std::span<const std::string_view> spanKeys(vecKeys);
	cmd.m_func = m_cmdTree.GetFunc(spanKeys);
	if (!cmd.m_func)
	{
		LogInvalid("unknown command");
		return;
	}

	// 逐个解析参数 支持变量引用
	size_t nPos = 0;
	while (strArgs.find_first_not_of(STR_WHITESPACE, nPos) != std::string_view::npos)
	{
		ShellVar var;
		if (!ParseValue(strArgs, nPos, var))
		{
			return;
		}
		cmd.m_vecParams.push_back(std::move(var));
	}

	m_mtxCmdQueue.lock();
	m_qCmd.push(std::move(cmd));
	m_mtxCmdQueue.unlock();
}


bool Shell::Run()
{
	std::string line;
	while (std::getline(std::cin, line)) {
		if (!m_bShellRun.load())
		{
			break;
		}

		if (line.empty()) continue;

		std::istringstream iss(line);
		std::string cmd;
		iss >> cmd;
		Command(line);
		if (cmd == "quit" || cmd == "exit") { 
			break;
		}


	}

	m_bShellRun.store(false);

	return true;
}

void Shell::Execute()
{
	m_bShellRun.store(true);
	m_thrShell = std::thread(std::bind(&Shell::Run, this));
}

void Shell::Terminate()
{
	m_bShellRun.store(false);
	m_thrShell.detach();
}

bool Shell::Flush()
{
	if (!m_bShellRun.load())
	{
		return false;
	}

	m_mtxCmdQueue.lock();
	while (!m_qCmd.empty())
	{
		Cmd& cmd = m_qCmd.front();

		// 执行前把变量引用(eCustomedType0)解析为变量的实际值
		// 变量可能由同一批中更早执行的命令创建 所以只能在执行时解析
		bool bValid = true;
		for (ShellVar& param : cmd.m_vecParams)
		{
			if (!param.IsVarRef())
			{
				continue;
			}

			// 解析变量引用时 若变量本身存的是引用(别名) 一并写回为实际值
			ShellVar* pValue = GetVarValue(param.m_str);
			if (pValue == nullptr)
			{
				LOG_WARNING("Shell command skipped: undefined variable \"%s\"", param.m_str.c_str());
				bValid = false;
				break;
			}
			param = *pValue;
		}

		if (bValid)
		{
			cmd();
		}

		m_qCmd.pop();
	}
	m_mtxCmdQueue.unlock();

	return true;
}

bool Shell::HasCmd()
{
	bool bHasCmd = false;
	m_mtxCmdQueue.lock();
	bHasCmd = !m_qCmd.empty();
	m_mtxCmdQueue.unlock();
	return bHasCmd;
}

void Shell::Echo(std::string_view strEcho)
{
	std::cout << strEcho << std::endl;
}

Shell::CmdTree::CmdTree()
	:m_strKey("root")
	,m_func(nullptr)
{
}

Shell::CmdTree::CmdTree(std::string_view strKey, FuncEngineCall func)
	: m_strKey(strKey)
	, m_func(func)
{
}

bool Shell::CmdTree::AddFunc(std::span<std::string_view> keys, FuncEngineCall func)
{
	if (keys.size() == 0)
	{
		m_func = func;
		return true;
	}
	else
	{
		CmdTree* pChild = nullptr;
		for (auto& child : m_listChildren) {
			if (child.m_strKey == keys[0])
			{
				pChild = &child;
			}
		}

		if (pChild == nullptr)
		{
			m_listChildren.emplace_back(keys[0], nullptr);
			pChild = &m_listChildren.back();
		}

		if (keys.size() == 1)
		{
			return pChild->AddFunc({}, func);
		}
		std::span<std::string_view> subKeys(keys.data() + 1, keys.size() - 1);
		return pChild->AddFunc(subKeys, func);
	}
	return false;
}

Shell::FuncEngineCall Shell::CmdTree::GetFunc(std::span<const std::string_view> keys)
{
	if (keys.empty())
	{
		return m_func;
	}
	for (auto& cmdTree : m_listChildren)
	{
		if (cmdTree.m_strKey == keys[0])
		{
			return cmdTree.GetFunc(keys.subspan(1));
		}
	}
	return nullptr;
}
