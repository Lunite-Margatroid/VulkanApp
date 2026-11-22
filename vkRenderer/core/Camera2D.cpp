#include "EngineCommon.h"
#include "Camera2D.hpp"

namespace LT {
	Camera2D::Camera2D()
		: m_position(0.f, 0.f)
		, m_fYaw(0.f)
		, m_fWidth(static_cast<float>(DEFAULT_WIDTH))
		, m_fAspect(static_cast<float>(DEFAULT_WIDTH) / static_cast<float>(DEFAULT_HEIGHT))
		, m_bDirt(true)
		, m_VPMat(1.f)
		, m_offset(0.f, 0.f)
	{}

	void Camera2D::UpdateMat() const
	{
		if (!m_bDirt)
		{
			return;
		}

		// 视口高度由宽度与宽高比推出
		const float fHeight = m_fWidth / m_fAspect;

		// 世界空间 -> NDC
		// X: [-width/2, width/2] 映射到 [-1, 1]
		// Y: [-height/2, height/2] 映射到 [-1, 1]
		// Vulkan屏幕空间Y轴朝下 引擎屏幕空间Y轴朝上 因此Y方向取负
		// glm::mat2 列主序
		const glm::mat2 sMat(
			2.f / m_fWidth, 0.f,
			0.f, -2.f / fHeight
		);

		// 右手系绕Z轴旋转 相机yaw为正表示逆时针旋转
		// 世界空间 -> 视图空间 需要对相机旋转取逆 即旋转 -yaw
		const float fCos = glm::cos(m_fYaw);
		const float fSin = glm::sin(m_fYaw);
		// 列主序 R(-yaw)
		const glm::mat2 matView(
			fCos, -fSin,
			fSin, fCos
		);

		m_VPMat = sMat * matView;

		// 将相机位置对齐到NDC原点
		// 着色器中offset在VP矩阵变换之后相加 因此该值位于NDC空间
		// P_ndc = vpMat * (P_world - position) = vpMat * P_world - vpMat * position
		m_offset = -(m_VPMat * m_position);

		m_bDirt = false;
	}

	const glm::mat2& Camera2D::GetVPMat() const
	{
		UpdateMat();
		return m_VPMat;
	}

	const glm::vec2& Camera2D::GetOffset() const
	{
		UpdateMat();
		return m_offset;
	}

	void Camera2D::SetPosition(const glm::vec2& position)
	{
		m_position = position;
		m_bDirt = true;
	}

	const glm::vec2& Camera2D::GetPosition() const
	{
		return m_position;
	}

	void Camera2D::SetWidth(float fWidth)
	{
		m_fWidth = fWidth;
		m_bDirt = true;
	}

	float Camera2D::GetWidth() const
	{
		return m_fWidth;
	}

	void Camera2D::SetYaw(float fYaw)
	{
		m_fYaw = fYaw;
		m_bDirt = true;
	}

	float Camera2D::GetYaw() const
	{
		return m_fYaw;
	}

	void Camera2D::SetAspect(float fAspect)
	{
		m_fAspect = fAspect;
		m_bDirt = true;
	}

	float Camera2D::GetAspect() const
	{
		return m_fAspect;
	}
}
