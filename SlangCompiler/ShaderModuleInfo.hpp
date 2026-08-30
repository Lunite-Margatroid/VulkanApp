// 记录资源信息
#include <vector>
namespace LT {

	enum class BindingSpace : uint32_t{
		eVertexShader = 0,
		eFragmentShader = 1,
		eVertAndFragShader = 2,
	};

	struct BindingInfo {
		uint32_t nIndex;
		BindingSpace eSpace;

		BindingInfo(uint32_t index, uint32_t space) :
			nIndex(index), eSpace(static_cast<BindingSpace>(space))
		{}


		vk::ShaderStageFlags GetShaderStageFlag() const {
			switch (eSpace)
			{
			case BindingSpace::eVertexShader:
				return vk::ShaderStageFlagBits::eVertex;
			case BindingSpace::eFragmentShader:
				return vk::ShaderStageFlagBits::eFragment;
			case BindingSpace::eVertAndFragShader:
				return (vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
			default:
				return vk::ShaderStageFlagBits::eAll;
			}
		}
	};

	struct ShaderModuleInfo {
		std::vector<BindingInfo> m_vecTexture2DBindingInfo;
		std::vector<BindingInfo> m_vecConstBufferBindingInfo;


		void Clear();
	};

} // namespace LT