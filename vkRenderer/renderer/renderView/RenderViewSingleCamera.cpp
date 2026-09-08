// 渲染视图 单相机
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "RenderViewSingleCamera.hpp"
#include "BufferManager.h"


namespace LT{
	RenderViewSingleCamera::RenderViewSingleCamera()
	{
		m_pConstBuffer = BufferManager::CreateConstBuffer(sizeof(m_MVPMatBuf), nullptr);

	}
	RenderViewSingleCamera::~RenderViewSingleCamera()
	{
		if (m_pConstBuffer)
		{
			BufferManager::DeleteBuffer(m_pConstBuffer->GetBufferID());
		}
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
} // namespace LT

