// 渲染视图 单相机
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "RenderViewSingleCamera.hpp"
#include "BufferManager.h"


namespace LT{
	RenderViewSingleCamera::RenderViewSingleCamera()
	{
	}
	RenderViewSingleCamera::~RenderViewSingleCamera()
	{
	}
	void RenderViewSingleCamera::SetModelMat(const glm::mat4& matModel)
	{
		m_MVPMatBuf.modelMat = matModel;
	}
	void RenderViewSingleCamera::SetProjectionMat(const glm::mat4& matProjection)
	{
		m_MVPMatBuf.projectionMat = matProjection;
	}
	void RenderViewSingleCamera::SetViewMat(const glm::mat4& matView)
	{
		m_MVPMatBuf.viewMat = matView;
	}
	void RenderViewSingleCamera::SetCameraPos(const glm::vec3& vec3CameraPos)
	{
		m_MVPMatBuf.cameraPos = glm::vec4(vec3CameraPos, 1.f);
	}
	const MVPMatrixBuffer* RenderViewSingleCamera::GetTransBuffer()
	{
		m_MVPMatBuf.mvpMat = m_MVPMatBuf.projectionMat * m_MVPMatBuf.viewMat * m_MVPMatBuf.modelMat;

		glm::mat3 noramlMat = m_MVPMatBuf.modelMat;

		noramlMat = glm::transpose(glm::inverse(noramlMat));

		m_MVPMatBuf.normalMatCol[0] = glm::vec4(noramlMat[0], 0.f);
		m_MVPMatBuf.normalMatCol[1] = glm::vec4(noramlMat[1], 0.f);
		m_MVPMatBuf.normalMatCol[2] = glm::vec4(noramlMat[2], 0.f);

		return &m_MVPMatBuf;
	}
} // namespace LT

