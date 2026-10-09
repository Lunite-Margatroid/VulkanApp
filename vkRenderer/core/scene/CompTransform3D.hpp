// 3D变换组件 控制节点在3D空间中的位置、旋转、缩放
#pragma once
#include "IComponent.hpp"


namespace LT {
	// 3D变换组件 控制节点在3D空间中的位置、旋转、缩放
	class CompTransform3D : public IComponent {
	private:
		glm::vec3 m_vPosition; // 位置
		glm::quat m_qRotation; // 旋转
		glm::vec3 m_vScale; // 缩放

		glm::mat4 m_matLocal; // 局部变换矩阵
		bool m_bLocalDirty; // 局部变换矩阵是否需要重新计算

	public:
		CompTransform3D();
		virtual ~CompTransform3D() = default;

		// 获取组件类型
		virtual ComponentType GetComponentType() const override {
			return ComponentType::eTransform3D;
		}

		void SetPosition(const glm::vec3& vPosition);
		const glm::vec3& GetPosition() const;

		// 设置欧拉角旋转 XYZ内旋
		void SetEulerRotation(const glm::vec3& vEulerRotation);
		// 获取欧拉角旋转 XYZ内旋
		glm::vec3 GetEulerRotation() const;

		void SetRotation(const glm::quat& qRotation);
		const glm::quat& GetRotation() const;

		void SetScale(const glm::vec3& vScale);
		const glm::vec3& GetScale() const;

		// 获取局部变换矩阵
		const glm::mat4& GetLocalMatrix();
	};
} // namespace LT
