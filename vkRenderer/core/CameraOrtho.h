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
		float m_fNear;
		float m_fFar;

	protected:

	public:

		CameraOrtho();

		glm::mat4 GetProjectionMat() const override;

		void SetLeft(float fLeft);
		void SetRight(float fRight);
		void SetTop(float fTop);
		void SetBottom(float fBottom);
		void SetNear(float fNear);
		void SetFar(float fFar);


		float GetLeft() const;
		float GetRight() const;
		float GetTop() const;
		float GetBottom() const;
		float GetNear() const;
		float GetFar() const;
	};
} // namespace LT