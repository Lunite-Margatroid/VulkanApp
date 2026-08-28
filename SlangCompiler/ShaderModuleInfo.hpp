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
	};

	struct ShaderModuleInfo {
		std::vector<BindingInfo> m_vecTexture2DBindingInfo;
		std::vector<BindingInfo> m_vecConstBufferBindingInfo;


		void Clear();
	};

} // namespace LT