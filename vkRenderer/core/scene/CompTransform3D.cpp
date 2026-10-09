#pragma once
#include "EngineCommon.h"
#include "CompTransform3D.hpp"

namespace LT {
	CompTransform3D::CompTransform3D()
		: m_vPosition(0.f)
		, m_qRotation(glm::quat(1.f, 0.f, 0.f, 0.f))
		, m_vScale(1.f)
		, m_matLocal(glm::mat4(1.f))
		, m_bLocalDirty(false)
	{
	}

	void CompTransform3D::SetPosition(const glm::vec3& vPosition) {
		m_vPosition = vPosition;
		m_bLocalDirty = true;
	}

	const glm::vec3& CompTransform3D::GetPosition() const {
		return m_vPosition;
	}

	void CompTransform3D::SetEulerRotation(const glm::vec3& vEulerRotation) {
		// XYZ内旋: 依次绕局部X、Y、Z轴旋转 等价于R = Rx * Ry * Rz
		glm::mat4 mRotation = glm::rotate(glm::mat4(1.f), vEulerRotation.x, VEC3_AXIS_X)
			* glm::rotate(glm::mat4(1.f), vEulerRotation.y, VEC3_AXIS_Y)
			* glm::rotate(glm::mat4(1.f), vEulerRotation.z, VEC3_AXIS_Z);
		m_qRotation = glm::quat_cast(mRotation);
		m_bLocalDirty = true;
	}

	glm::vec3 CompTransform3D::GetEulerRotation() const {
		// XYZ内旋: R = Rx * Ry * Rz
		glm::mat3 mRotation = glm::mat3_cast(m_qRotation);
		float fRotX = glm::atan(-mRotation[2][1], mRotation[2][2]); // atan2(-R23, R33)
		float fRotY = glm::asin(mRotation[2][0]); // asin(R13)
		float fRotZ = glm::atan(-mRotation[1][0], mRotation[0][0]); // atan2(-R12, R11)
		return glm::vec3(fRotX, fRotY, fRotZ);
	}

	void CompTransform3D::SetRotation(const glm::quat& qRotation) {
		m_qRotation = qRotation;
		m_bLocalDirty = true;
	}

	const glm::quat& CompTransform3D::GetRotation() const {
		return m_qRotation;
	}

	void CompTransform3D::SetScale(const glm::vec3& vScale) {
		m_vScale = vScale;
		m_bLocalDirty = true;
	}

	const glm::vec3& CompTransform3D::GetScale() const {
		return m_vScale;
	}

	const glm::mat4& CompTransform3D::GetLocalMatrix() {
		if (m_bLocalDirty)
		{
			m_matLocal = glm::translate(glm::identity<glm::mat4>(), m_vPosition)
				* glm::mat4_cast(m_qRotation)
				* glm::scale(glm::identity<glm::mat4>(), m_vScale);
			m_bLocalDirty = false;
		}
		return m_matLocal;
	}
} // namespace LT
