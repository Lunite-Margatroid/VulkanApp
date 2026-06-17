#pragma once
namespace LT {
	class ExceptionRenderBase : public std::exception{
	private:
		std::string m_strMsg;

	public:
		explicit ExceptionRenderBase(const std::string& msg);

		const char* what() const throw() override;

	protected:
		void SetMessage(const std::string& newMsg);
		void SetMessage(std::string&& newMsg) noexcept;

	};

	using RunTimeExcept = ExceptionRenderBase;
}