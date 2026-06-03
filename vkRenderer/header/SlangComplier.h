#pragma once
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-com-helper.h>

namespace LT {
	class SlangComplier final {
	private:
		SlangComplier();
		~SlangComplier() = default;

		SlangComplier(const SlangComplier& ) = delete;
		SlangComplier(SlangComplier&&) = delete;

		SlangComplier& operator = (SlangComplier&&) = delete;
		SlangComplier& operator = (const SlangComplier&) = delete;


		Slang::ComPtr<slang::IGlobalSession> m_pGlobalSession;
		Slang::ComPtr<slang::ISession> m_pSession;
	public:
		std::vector<BYTE> CompileFromFile(const std::filesystem::path &path, const std::vector<std::string>& vecEntryPoint);


	public:
		static void Init();
		static void Release();

		static SlangComplier& GetInstance();
	private:
		static SlangComplier* s_pInstance;
	};
}