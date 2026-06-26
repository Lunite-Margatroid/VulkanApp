#pragma once
#include "IEventNode.h"
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

		void OnEvent() override;
	};

} // namespace LT
