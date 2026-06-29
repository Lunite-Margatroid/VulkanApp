#include "EngineCommon.h"
#include "CameraOrtho.h"

namespace LT {
	CameraOrtho::CameraOrtho()
		: Camera()
		, m_fLeft(-1.f)
		, m_fRight(1.f)
		, m_fTop(1.f)
		, m_fBottom(-1.f)
	{
	}
	glm::mat4 CameraOrtho::GetProjectionMat() const
	{
		return MAT4_SCREEN_TRANS_MAT * glm::ortho(m_fLeft, m_fRight, m_fBottom, m_fTop);
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
}