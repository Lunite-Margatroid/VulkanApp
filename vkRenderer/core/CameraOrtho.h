#pragma once
#include "Camera.h"

namespace LT {
	class CameraOrtho : public Camera
	{
	protected:
		float m_fLeft;
		float m_fRight;
		float m_fTop;
		float m_fBottom;

	protected:
		CameraOrtho();

	public:
		glm::mat4 GetProjectionMat() const override;

		void SetLeft(float fLeft);
		void SetRight(float fRight);
		void SetTop(float fTop);
		void SetBottom(float fBottom);

		float GetLeft() const;
		float GetRight() const;
		float GetTop() const;
		float GetBottom() const;
	};
} // namespace LT