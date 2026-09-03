// Stage基类

namespace LT {
	enum class RenderStageType : uint32_t {
		eOpaqueForward,
	};

	class IRenderStage {
	public:
		IRenderStage() = default;
		virtual ~IRenderStage() = default;
	
	};

} // namespace LT