#include "EngineCommon.h"
#include "Camera.h"

namespace LT {
	Camera::Camera()
		: m_vecEye(0.f, 0.f, 1.f)
		, m_vecLookAt(0.f, 0.f, 0.f)
		, m_dirUp(VEC3_AXIS_Y)
	{
	
	}
	glm::mat4 Camera::GetViewMat() const
	{
		return glm::lookAt(m_vecEye, m_vecLookAt, m_dirUp);
	}
	void Camera::OnEvent()
	{
		// TODO
	}
}//namespace LT