#include "vkRendererCommon.h"
#include "SlangCompiler.h"

namespace LT {
	SlangCompiler* SlangCompiler::s_pInstance = nullptr;

	SlangCompiler::SlangCompiler() {
		slang::createGlobalSession(m_pGlobalSession.writeRef());

		slang::TargetDesc sTargetDesc = {
			.format = SLANG_SPIRV,
			.profile = m_pGlobalSession->findProfile("spirv_1_5")
		};

		slang::SessionDesc sSessionDesc = {
			.targets = &sTargetDesc,
			.targetCount = 1,
			.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR
		};

		m_pGlobalSession->createSession(sSessionDesc, m_pSession.writeRef());
	}

	void SlangCompiler::Init() {
		if (!s_pInstance) {
			s_pInstance = new SlangCompiler();
			LOG_TRACING("Init SlangCompiler.");
		}
		else
		{
			LOG_WARNING("Repeat initing SlangCompiler.");
		}
	}

	void SlangCompiler::Release() {
		if (s_pInstance)
		{
			delete s_pInstance;
			s_pInstance = nullptr;
		}
		else
		{
			LOG_WARNING("SlangCompiler: Bad Release.");
		}
	}

	std::vector<BYTE> SlangCompiler::CompileFromFile(const std::filesystem::path& path, const std::vector<std::string>& vecEntryPoint) {

		Slang::ComPtr<slang::IModule> pModule;
		std::string strModuleName = path.stem().generic_string();
		std::string strPath = path.filename().generic_string();
		{
			Slang::ComPtr<slang::IBlob> pDiagnoseBlob;
			pModule = m_pSession->loadModuleFromSourceString(
				strModuleName.c_str(),
				strPath.c_str(),
				ReadText(path).c_str(),
				pDiagnoseBlob.writeRef()
			);

			if (!pModule)
			{
				LOG_ERROR_WITH_FILE("Slang Complier: Load module failed \"%s\"", strModuleName.c_str());
			}
		}



		//// 查询入口点
		//std::vector<Slang::ComPtr<slang::IEntryPoint>> vecPEntryPoint;
		//for (const auto& strEntryPoint : vecEntryPoint)
		//{
		//	Slang::ComPtr<slang::IEntryPoint> pEntryPoint;
		//	auto result = pModule->findEntryPointByName(strEntryPoint.c_str(), pEntryPoint.writeRef());

		//	if (SLANG_FAILED(result))
		//	{
		//		LOG_ERROR_WITH_FILE("Slang Complier: Find entry point failed \"%s\"", strEntryPoint.c_str());
		//	}

		//	vecPEntryPoint.emplace_back(pEntryPoint);
		//}

		//if (vecPEntryPoint.size() != 2)
		//{
		//	LOG_ERROR("There should be 2 entry points.");
		//}

		//std::array<slang::IComponentType*, 3> arrComponentTypes = {
		//	pModule.get(),
		//	vecPEntryPoint[0].get(),
		//	vecPEntryPoint[1].get()
		//};

		//Slang::ComPtr<slang::IComponentType> pComposedProgram;
		//{
		//	Slang::ComPtr<slang::IBlob> pDiagnoseBlob;
		//	auto result = m_pSession->createCompositeComponentType(
		//		arrComponentTypes.data(),
		//		arrComponentTypes.size(),
		//		pComposedProgram.writeRef(),
		//		pDiagnoseBlob.writeRef()
		//	);
		//	if (SLANG_FAILED(result))
		//	{
		//		LOG_ERROR_WITH_FILE("Slang Complier: Compose failed.");
		//	}
		//}

		//Slang::ComPtr<slang::IComponentType> pLinkedProgram;
		//{
		//	Slang::ComPtr<slang::IBlob> pDiagnoseBlob;
		//	auto result = pComposedProgram->link(
		//		pLinkedProgram.writeRef(),
		//		pDiagnoseBlob.writeRef()
		//	);
		//	if (SLANG_FAILED(result)) {
		//		LOG_ERROR_WITH_FILE("Slang Complier: Link failed.");
		//	}
		//}

		Slang::ComPtr<slang::IComponentType> pLinkedProgram;
		{
			Slang::ComPtr<slang::IBlob> pDiagnoseBlob;
			auto result = pModule->link(pLinkedProgram.writeRef(), pDiagnoseBlob.writeRef());
			if (SLANG_FAILED(result))
			{
				LOG_ERROR_WITH_FILE("Slang Compiler: Link failed.");
			}
		}

		Slang::ComPtr<slang::IBlob> pSprivCode;
		{
			Slang::ComPtr<slang::IBlob> pDiagnoseBlob;
			auto result = pLinkedProgram->getTargetCode(0, pSprivCode.writeRef(), pDiagnoseBlob.writeRef());
			if (SLANG_FAILED(result))
			{
				LOG_ERROR_WITH_FILE("Slang Compiler: Get Code Failed.");
			}
		}

		std::vector<BYTE> vecOutCode;
		pSprivCode->getBufferSize();
		pSprivCode->getBufferPointer();

		vecOutCode.resize(pSprivCode->getBufferSize());
		memcpy(vecOutCode.data(), pSprivCode->getBufferPointer(), pSprivCode->getBufferSize());
		return vecOutCode;
	}

	std::vector<BYTE> SlangCompiler::ComplieShader(const std::vector<std::string>& vecModules, const RenderPassFlag& nFlag)
	{
		// TargetDesc
		slang::TargetDesc td = {};
		td.format = SLANG_SPIRV;
		td.profile = m_pGlobalSession->findProfile("spirv_1_5");

		slang::SessionDesc sd = {};
		sd.targetCount = 1;
		sd.targets = &td;

		// 预处理宏
		std::vector<slang::PreprocessorMacroDesc> vecPPMacro = GenGraphicPPMacroDesc(nFlag);

		sd.preprocessorMacroCount = vecPPMacro.size();
		sd.preprocessorMacros = vecPPMacro.data();

		// 矩阵列主序
		sd.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

		// options
		std::vector<slang::CompilerOptionEntry> vecOptions;
		// option: searchPath
		slang::CompilerOptionEntry optSearchPath;
		optSearchPath.name = slang::CompilerOptionName::Include;
		optSearchPath.value.kind = slang::CompilerOptionValueKind::String;
		optSearchPath.value.intValue0 = 0;
		optSearchPath.value.intValue1 = 0;
		optSearchPath.value.stringValue0 = "./slang";
		optSearchPath.value.stringValue1 = nullptr;
		vecOptions.push_back(optSearchPath);
		// set options
		sd.compilerOptionEntries = vecOptions.data();
		sd.compilerOptionEntryCount = vecOptions.size();

		// 创建
		Slang::ComPtr<slang::ISession> pSession;
		m_pGlobalSession->createSession(sd, pSession.writeRef());

		// 加载Module
		std::vector<Slang::ComPtr<slang::IModule>> vecPModules;
		for (const std::string strModule : vecModules)
		{
			Slang::ComPtr<slang::IBlob> pDiagnosticsBlob;
			std::string strModulePath = strModule + ".slang";

			std::string strFilePath = "./slang/" + strModulePath;

			std::filesystem::path pathModuleFile(strFilePath);

			Slang::ComPtr<slang::IModule> pModule;
			pModule = pSession->loadModuleFromSourceString(
				strModule.c_str(),
				strModulePath.c_str(),
				ReadText(pathModuleFile).c_str(),
				pDiagnosticsBlob.writeRef()
			);

			if (pModule)
			{
				vecPModules.push_back(pModule);
			}
			else
			{
				// 查错
				pDiagnosticsBlob;
			}
		}
		// 组合
		std::vector<slang::IComponentType*> vecComponents;
		for (Slang::ComPtr<slang::IModule> pModule : vecPModules)
		{
			vecComponents.push_back(reinterpret_cast<slang::IComponentType*>(pModule.get()));
		}

		Slang::ComPtr<slang::IComponentType> pComposed;
		{
			Slang::ComPtr<slang::IBlob> pDiagnosticsBlob;
			SlangResult result = pSession->createCompositeComponentType(
				vecComponents.data(),
				vecComponents.size(),
				pComposed.writeRef(),
				pDiagnosticsBlob.writeRef()
			);
		}

		// 链接
		Slang::ComPtr<slang::IComponentType> pLinked;
		{
			Slang::ComPtr<slang::IBlob> pDiagnosticsBlob;
			SlangResult result = pComposed->link(
				pLinked.writeRef(),
				pDiagnosticsBlob.writeRef()
			);
		}

		// 获取二进制程序

		Slang::ComPtr<slang::IBlob> pSprivCode;
		{
			Slang::ComPtr<slang::IBlob> pDiagnoseBlob;
			auto result = pLinked->getTargetCode(0, pSprivCode.writeRef(), pDiagnoseBlob.writeRef());
			if (SLANG_FAILED(result))
			{
				LOG_ERROR_WITH_FILE("Slang Compiler: Get Code Failed.");
			}
		}

		std::vector<BYTE> vecOutCode;
		pSprivCode->getBufferSize();
		pSprivCode->getBufferPointer();

		vecOutCode.resize(pSprivCode->getBufferSize());
		memcpy(vecOutCode.data(), pSprivCode->getBufferPointer(), pSprivCode->getBufferSize());
		return vecOutCode;
	}

	Slang::ComPtr<slang::IGlobalSession> SlangCompiler::GetGlobalSession() const
	{
		return m_pGlobalSession;
	}

	SlangCompiler& SlangCompiler::GetInstance() {
		if (s_pInstance)
		{
			return *s_pInstance;
		}
		else
		{
			LOG_WARNING("SlangCompiler::GetInstance(). It does not init.");
			Init();
			return *s_pInstance;
		}
	}


	std::vector<slang::PreprocessorMacroDesc> GenGraphicPPMacroDesc(const RenderPassFlag& nFlag) {
		std::vector<slang::PreprocessorMacroDesc> vecPPMacro;
		if (HasVertexPos(nFlag)) {
			vecPPMacro.emplace_back("VERT_POSITION", "1");
		}
		if (HasUV(nFlag, 0))
		{
			vecPPMacro.emplace_back("VERT_UV0", "1");
		}
		if (HasUV(nFlag, 1))
		{
			vecPPMacro.emplace_back("VERT_UV1", "1");
		}
		if (HasUV(nFlag, 2))
		{
			vecPPMacro.emplace_back("VERT_UV2", "1");
		}
		if (HasUV(nFlag, 3))
		{
			vecPPMacro.emplace_back("VERT_UV3", "1");
		}
		if (HasUV(nFlag, 4))
		{
			vecPPMacro.emplace_back("VERT_UV4", "1");
		}
		if (HasNormal(nFlag))
		{
			vecPPMacro.emplace_back("VERT_NORMAL", "1");
		}
		if (HasTangent(nFlag))
		{
			vecPPMacro.emplace_back("VERT_TANGENT", "1");
		}
		if (HasBitangent(nFlag))
		{
			vecPPMacro.emplace_back("VERT_BITANGENT", "1");
		}
		if (HasColor(nFlag))
		{
			vecPPMacro.emplace_back("VERT_COLOR", "1");
		}
		if (HasAO(nFlag))
		{
			vecPPMacro.emplace_back("VERT_AO", "1");
		}

		return vecPPMacro;
	
	}
}// namespace LT

