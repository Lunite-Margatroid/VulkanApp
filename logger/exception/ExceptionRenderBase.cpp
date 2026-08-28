#include <string>
#include <exception>
#include "ExceptionRenderBase.hpp"
#include "logger.hpp"

namespace LT
{

	ExceptionRenderBase::ExceptionRenderBase(const std::string& msg)
		:m_strMsg(msg)
	{
	}

	const char* ExceptionRenderBase::what() const throw()
	{
		return m_strMsg.c_str();
	}

	void ExceptionRenderBase::SetMessage(const std::string& newMsg)
	{
		m_strMsg = newMsg;
	}
	void ExceptionRenderBase::SetMessage(std::string&& newMsg) noexcept
	{
		m_strMsg = std::move(newMsg);
	}
}