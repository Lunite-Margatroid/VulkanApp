#pragma once
#include "EntityRender2D.hpp"

namespace LT {
	class EntityRenderLogLine : public EntityRender2D {
	protected:
		VertexBuffer* m_pVertexBuffer;

	public:
		EntityRenderLogLine(EntityID id);
		~EntityRenderLogLine() = default;

		void Draw(EntityDrawInfo& sInOutDrawInfo) override;

		void SetVertexBuffer(VertexBuffer* pVertexBuffer);
	};
} // namespace LT