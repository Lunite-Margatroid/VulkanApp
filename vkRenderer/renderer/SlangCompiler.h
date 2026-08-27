#pragma once
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-com-helper.h>
#include "renderpass/RenderPass.hpp"

namespace LT {
	class SlangCompiler final {
	private:
		SlangCompiler();
		~SlangCompiler() = default;

		SlangCompiler(const SlangCompiler& ) = delete;
		SlangCompiler(SlangCompiler&&) = delete;

		SlangCompiler& operator = (SlangCompiler&&) = delete;
		SlangCompiler& operator = (const SlangCompiler&) = delete;


		Slang::ComPtr<slang::IGlobalSession> m_pGlobalSession;
		Slang::ComPtr<slang::ISession> m_pSession;
	public:
		std::vector<BYTE> CompileFromFile(const std::filesystem::path &path, const std::vector<std::string>& vecEntryPoint);

		std::vector<BYTE> ComplieShader(const std::vector<std::string>& vecModules, const RenderPassFlag& nFlag);

		Slang::ComPtr<slang::IGlobalSession> GetGlobalSession() const;
	public:
		static void Init();
		static void Release();

		static SlangCompiler& GetInstance();

		static std::vector<slang::PreprocessorMacroDesc> GenGraphicPPMacroDesc(const RenderPassFlag& nFlag);

	private:
		static SlangCompiler* s_pInstance;
	};
}