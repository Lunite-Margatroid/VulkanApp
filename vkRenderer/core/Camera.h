#pragma once
#include "IEventNode.h"
#include "EngineCommon.h"
namespace LT {
	class Camera : public IEventNode{
	protected:
		glm::vec3 m_vecEye;
		glm::vec3 m_vecLookAt;
		glm::vec3 m_dirUp;

	public:
		Camera();

		Camera& operator = (const Camera&) = delete;
		Camera& operator = (Camera&&) = delete;

		Camera(const Camera&) = delete;
		Camera(Camera&&) = delete;

		glm::mat4 GetViewMat() const;
		virtual glm::mat4 GetProjectionMat() const = 0;

		void SetEye(const glm::vec3& eye);
		void SetLookAt(const glm::vec3& lookat);
		void SetUpDir(const glm::vec3& upDir);

		glm::vec3 GetEye() const;
		glm::vec3 GetLookAt()const;
		glm::vec3 GetUpDir()const;

		void OnEvent() override;
	};

} // namespace LT
