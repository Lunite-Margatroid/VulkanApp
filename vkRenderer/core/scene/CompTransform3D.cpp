#pragma once
#include "EngineCommon.h"
#include "CompTransform3D.hpp"

namespace LT {
	CompTransform3D::CompTransform3D()
		: m_vPosition(0.f)
		, m_qRotation(glm::quat(1.f, 0.f, 0.f, 0.f))
		, m_vScale(1.f)
		, m_matLocal(glm::mat4(1.f))
		, m_matRelative(1.f)
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

		m_qRotation = glm::quat_cast(glm::eulerAngleZYX(vEulerRotation.z, vEulerRotation.y, vEulerRotation.x));
		m_bLocalDirty = true;
	}

	glm::vec3 CompTransform3D::GetEulerRotation() const {
		glm::mat4 mRotation = glm::mat4_cast(m_qRotation);

		glm::vec3 vEuler;
		glm::extractEulerAngleXYZ(mRotation, vEuler.x, vEuler.y, vEuler.z);

		return vEuler;
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
	const glm::mat4& CompTransform3D::GetGlobalMatrix()
	{
		return GetLocalMatrix() * m_matRelative;
	}
	void CompTransform3D::UpdateGlobalMatrix(const glm::mat4& mat)
	{
		m_matRelative = mat;
	}
} // namespace LT
