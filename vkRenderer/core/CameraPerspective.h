#pragma once
#include "Camera.h"

namespace LT {
	class CameraPerspective : public Camera
	{
	protected:
		float m_fFov; // 纵向的视角
		float m_fAspect; // 宽高比
		float m_fNear;
		float m_fFar;

	public:
		CameraPerspective();

		glm::mat4 GetProjectionMat() const override;


		float GetFov() const;
		float GetAspect() const;
		float GetNear() const;
		float GetFar() const;

		void SetFov(float fFov);
		void SetAspect(float fAspect);
		void SetNear(float fNear);
		void SetFar(float Far);
	};
}