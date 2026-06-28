#include "EngineCommon.h"
#include "Camera.h"

namespace LT {
	Camera::Camera()
		: m_vecEye(glm::vec3(0.f, 0.f, 1.f))
		, m_vecLookAt(glm::vec3(0.f, 0.f, 0.f))
		, m_dirUp(VEC3_AXIS_Y)
	{
	
	}
	glm::mat4 Camera::GetViewMat() const
	{
		return glm::lookAt(m_vecEye, m_vecLookAt, m_dirUp);
	}
	void Camera::SetLookAt(const glm::vec3& lookat)
	{
		m_vecLookAt = lookat;
	}
	void Camera::SetUpDir(const glm::vec3& upDir)
	{
		m_dirUp = upDir;
	}
	glm::vec3 Camera::GetEye() const
	{
		return m_vecEye;
	}
	glm::vec3 Camera::GetLookAt() const
	{
		return m_vecLookAt;
	}
	glm::vec3 Camera::GetUpDir() const
	{
		return m_dirUp;
	}
	void Camera::SetEye(const glm::vec3& eye)
	{
		m_vecEye = eye;
	}
	void Camera::OnEvent()
	{
		// TODO
	}
}//namespace LT