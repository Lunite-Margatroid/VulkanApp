#pragma once
#include "ExceptionRenderBase.h"

namespace LT {
	class ExceptionAssert : public ExceptionRenderBase{
	public:
		explicit ExceptionAssert(const std::string& msg);
	};
}