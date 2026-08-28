#pragma once
#include "ExceptionRenderBase.hpp"

namespace LT {
	class ExceptionAssert : public ExceptionRenderBase{
	public:
		explicit ExceptionAssert(const std::string& msg);
	};
}