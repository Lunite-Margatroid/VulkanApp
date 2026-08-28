#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include "SlangCompiler.hpp"

#include "logger.hpp"

namespace LT {

	inline std::string ReadText(const std::filesystem::path& filePath) {
		std::string strFilePath = filePath.generic_string();
		std::ifstream file(strFilePath);
		if (!file.is_open()) {
			LOG_ERROR("Can not open file %s.", filePath.generic_string().c_str());
		}

		std::string content;
		content.reserve(std::filesystem::file_size(filePath)); // 预分配空间
		content.assign(std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>());
		return content;
	}

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

	std::vector<uint8_t> SlangCompiler::CompileFromFile(const std::filesystem::path& path, const std::vector<std::string>& vecEntryPoint) {

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

		std::vector<uint8_t> vecOutCode;
		pSprivCode->getBufferSize();
		pSprivCode->getBufferPointer();

		vecOutCode.resize(pSprivCode->getBufferSize());
		memcpy(vecOutCode.data(), pSprivCode->getBufferPointer(), pSprivCode->getBufferSize());
		return vecOutCode;
	}

	std::vector<uint8_t> SlangCompiler::ComplieShader(const std::vector<std::string>& vecModules, const std::vector<std::pair<const char*, const char*>>& vecPPMacro)
	{
		// TargetDesc
		slang::TargetDesc td = {};
		td.format = SLANG_SPIRV;
		td.profile = m_pGlobalSession->findProfile("spirv_1_5");

		slang::SessionDesc sd = {};
		sd.targetCount = 1;
		sd.targets = &td;

		// 预处理宏
		sd.preprocessorMacroCount = vecPPMacro.size();
		sd.preprocessorMacros = reinterpret_cast<const slang::PreprocessorMacroDesc*>(vecPPMacro.data());

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

		std::vector<uint8_t> vecOutCode;
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

}// namespace LT

