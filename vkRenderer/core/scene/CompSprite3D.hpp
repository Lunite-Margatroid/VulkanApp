#pragma once
#include "IComponent.hpp"
#include "IMesh.hpp"
#include "EntityRender.hpp"
#include "MeshManager.hpp"
#include "EntityManager.hpp"

namespace LT {
	class CompSprite3D : public IComponent {
	private:
		glm::mat4 m_matWorld; // Sprite3D的世界变换矩阵
		EntityRef m_refEntityRender; // 关联的渲染实体

	public:
		CompSprite3D();
		virtual ~CompSprite3D();
		// 获取组件类型
		virtual ComponentType GetComponentType() const override {
			return ComponentType::eSprite3D;
		}

		void Init(MeshRef refMesh);
	};
} // namespace LT