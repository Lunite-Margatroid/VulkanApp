#include "EngineCommon.h"
#include "CameraOrtho.h"

namespace LT {
	CameraOrtho::CameraOrtho()
		: Camera()
		, m_fLeft(-1.f)
		, m_fRight(1.f)
		, m_fTop(1.f)
		, m_fBottom(-1.f)
		, m_fNear(0.01f)
		, m_fFar(100.f)
	{
	}
	glm::mat4 CameraOrtho::GetProjectionMat() const
	{
		return MAT4_SCREEN_TRANS_MAT * glm::ortho(m_fLeft, m_fRight, m_fBottom, m_fTop, m_fNear, m_fFar);
	}
	void CameraOrtho::SetLeft(float fLeft)
	{
		m_fLeft = fLeft;
	}
	void CameraOrtho::SetRight(float fRight)
	{
		m_fRight = fRight;
	}
	void CameraOrtho::SetTop(float fTop)
	{
		m_fTop = fTop;
	}
	void CameraOrtho::SetBottom(float fBottom)
	{
		m_fBottom = fBottom;
	}
	void CameraOrtho::SetNear(float fNear)
	{
		m_fNear = fNear;
	}
	void CameraOrtho::SetFar(float fFar)
	{
		m_fFar = fFar;
	}
	float CameraOrtho::GetLeft() const
	{
		return m_fLeft;
	}
	float CameraOrtho::GetRight() const
	{
		return m_fRight;
	}
	float CameraOrtho::GetTop() const
	{
		return m_fTop;
	}
	float CameraOrtho::GetBottom() const
	{
		return m_fBottom;
	}
	float CameraOrtho::GetNear() const
	{
		return m_fNear;
	}
	float CameraOrtho::GetFar() const
	{
		return m_fFar;
	}
}