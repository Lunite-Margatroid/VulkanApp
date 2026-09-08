// Stage基类

namespace LT {
	enum class RenderStageType : uint32_t {
		eOpaqueForward,
	};

	class IRenderStage {
	public:
		IRenderStage() = default;
		virtual ~IRenderStage() = default;
	

		virtual RenderStageType GetRenderStageType() const = 0;
	};

} // namespace LT