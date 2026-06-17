#include "vkRendererCommon.h"
#include "ExceptionAssert.h"


namespace LT {
	ExceptionAssert::ExceptionAssert(const std::string& msg)
		:ExceptionRenderBase(msg)
	{
		SetMessage(std::string("Assert Failed!") + msg);
	}
}