#include "EngineCommon.h"
#include "CameraPerspective.h"

namespace LT {
	CameraPerspective::CameraPerspective()
		: Camera()
		, m_fFov(glm::pi<float>() / 4)
		, m_fAspect(1.f)
		, m_fNear(0.f)
		, m_fFar(100.f)

	{

	}
	glm::mat4 CameraPerspective::GetProjectionMat() const
	{
		return MAT4_SCREEN_TRANS_MAT * glm::perspective(m_fFov, m_fAspect, m_fNear, m_fFar);
	}
	float CameraPerspective::GetFov() const
	{
		return m_fFov;
	}

	float CameraPerspective::GetAspect() const
	{
		return m_fAspect;
	}

	float CameraPerspective::GetNear() const
	{
		return m_fNear;
	}

	float CameraPerspective::GetFar() const
	{
		return m_fFar;
	}

	void CameraPerspective::SetFov(float fFov)
	{
		m_fFov = fFov;
	}

	void CameraPerspective::SetAspect(float fAspect)
	{
		m_fAspect = fAspect;
	}

	void CameraPerspective::SetNear(float fNear)
	{
		m_fNear = fNear;
	}

	void CameraPerspective::SetFar(float fFar) {
		m_fFar = fFar;
	}
} // namespace LT