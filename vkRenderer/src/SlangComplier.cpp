#include "vkRendererCommon.h"
#include "SlangComplier.h"

namespace LT {
	SlangComplier::SlangComplier() {
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

	void SlangComplier::Init() {
		if (!s_pInstance) {
			s_pInstance = new SlangComplier();
			LOG_TRACING("Init SlangComplier.");
		}
		else
		{
			LOG_WARNING("Repeat initing SlangComplier.");
		}
	}

	void SlangComplier::Release() {
		if (s_pInstance)
		{
			delete s_pInstance;
			s_pInstance = nullptr;
		}
		else
		{
			LOG_WARNING("SlangComplier: Bad Release.");
		}
	}

	std::vector<BYTE> SlangComplier::CompileFromFile(const std::filesystem::path& path, const std::vector<std::string>& vecEntryPoint) {

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
}