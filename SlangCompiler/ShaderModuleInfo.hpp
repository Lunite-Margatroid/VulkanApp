// 记录资源信息
#pragma once
#include <vector>
namespace LT {

	// 与ShaderStage对应
	enum class BindingSpace : int16_t{
		eVertexShader = 0,
		eFragmentShader,
		eVertAndFragShader,
		BindingSpaceCount
	};

	enum class ShaderResSpace : int16_t {
		eTransBuffer = 0,
		eMtlProp = 1,
		ShaderResSpaceCount
	};

	struct BindingInfo {
		uint32_t nIndex;
		BindingSpace eSpace;
		// 该Binding所对应的Shader资源类型
		// 默认eMtlProp：反射得到的BindingInfo无法确定资源类型，由材质侧显式指定
		ShaderResSpace eResSpace;


		BindingInfo(uint32_t index, uint32_t space, ShaderResSpace eResSpace) :
			nIndex(index), eSpace(static_cast<BindingSpace>(space)), eResSpace(eResSpace)
		{}

		BindingInfo(uint32_t index, BindingSpace eSpace, ShaderResSpace eResSpace) :
			nIndex(index), eSpace(eSpace), eResSpace(eResSpace)
		{
		}

		bool operator==(const BindingInfo& rhs) const = default;
	};

	struct BindingInfoHash {
		size_t operator()(const BindingInfo& bindingInfo)const {
			uint64_t n = static_cast<uint64_t>(bindingInfo.nIndex);
			n = (n << 16) | (static_cast<uint64_t>(bindingInfo.eSpace) & 0xFFFF);
			n = (n << 16) | (static_cast<uint64_t>(bindingInfo.eResSpace) & 0xFFFF);
			return std::hash<uint64_t>{}(n);
		}
	};


	struct ShaderModuleInfo {
		std::vector<BindingInfo> m_vecTexture2DBindingInfo;
		std::vector<BindingInfo> m_vecConstBufferBindingInfo;


		void Clear();
	};

} // namespace LT