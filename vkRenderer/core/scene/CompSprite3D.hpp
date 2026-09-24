#pragma once
#include "IComponent.hpp"
#include "IMesh.hpp"
#include "EntityRender.hpp"
#include "MeshManager.hpp"
#include "EntityManager.hpp"
#include "MaterialManager.hpp"

namespace LT {
	class VertexBuffer;
	class IndexBuffer;

	class CompSprite3D : public IComponent {
	private:
		glm::mat4 m_matWorld; // Sprite3D的世界变换矩阵
		EntityRef m_refEntityRender; // 关联的渲染实体
		VertexBuffer* m_pVertexBuffer;
		IndexBuffer* m_pIndexBuffer;

		RenderPassFlag m_nRenderPassFlag;
		MaterialRef m_refMtl;

	public:
		CompSprite3D();
		virtual ~CompSprite3D();
		// 获取组件类型
		virtual ComponentType GetComponentType() const override {
			return ComponentType::eSprite3D;
		}

		void SetMesh(MeshRef refMesh);
		void SetMaterial(MaterialRef refMtl);
		EntityRef GetRenderEntity();

		void SetLineWidth(float fLineWidth);
		float GetLineWidth() const;
		void SetPolygonMode(vk::PolygonMode ePolygonMode);
		vk::PolygonMode GetPolygonMode() const;
		void SetBackCull(bool bBackCull);
		bool GetBackCull() const;
		void SetClockwiseFront(bool bClockwiseFront);
		bool GetClockwiseFront() const;
		void SetBlendEnabled(bool bBlend);
		bool GetBlendEnabled()const;

		void SetPrimitiveTopology(vk::PrimitiveTopology ePrimitiveTopology);
		vk::PrimitiveTopology GetPrimitiveTopology() const;

		RenderPassFlag GetRenderPassFlag() const;

	};
} // namespace LT