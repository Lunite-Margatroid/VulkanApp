#pragma once
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-com-helper.h>

#include "ShaderModuleInfo.hpp"

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
		std::vector<uint8_t> CompileFromFile(const std::filesystem::path &path, const std::vector<std::string>& vecEntryPoint);

		std::vector<uint8_t> CompileShader(const std::vector<std::string>& vecModules, const std::vector<std::pair<const char*, const char*>>& vecPPMacro, ShaderModuleInfo& sOutShaderModuleInfo);
		void CompileShaderToProgram(
			const std::vector<std::string>& vecModules, 
			const std::vector<std::pair<const char*, const char*>>& vecPPMacro,
			slang::ISession** ppOutSesson,
			slang::IComponentType** ppOutProgram);


		Slang::ComPtr<slang::IGlobalSession> GetGlobalSession() const;
	public:
		static void Init();
		static void Release();

		static SlangCompiler& GetInstance();

	private:
		static SlangCompiler* s_pInstance;
	};
}