#include <string>
#include <exception>
#include "ExceptionAssert.hpp"
#include "logger.hpp"

namespace LT {
	ExceptionAssert::ExceptionAssert(const std::string& msg)
		:ExceptionRenderBase(msg)
	{
		SetMessage(std::string("Assert Failed!") + msg);
	}
}