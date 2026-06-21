#pragma once

namespace LT {

	using BindTarget = long long int;

	class IBindable {
	public:
		virtual void Bind(BindTarget nTarget) = 0;
		virtual void Unbind() = 0;
	};
}//namespace LT

