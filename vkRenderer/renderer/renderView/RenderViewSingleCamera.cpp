// 渲染视图 单相机
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "RenderViewSingleCamera.hpp"
#include "BufferManager.h"

#include "ShaderResourceManager.hpp"

namespace LT{
	RenderViewSingleCamera::RenderViewSingleCamera()
	{
		for (auto& nBufferHandle : m_arrConstBufferVertTrans)
		{
			nBufferHandle = ShaderResourceManager::CreateTransBufferHandle();
		}
	}
	RenderViewSingleCamera::~RenderViewSingleCamera()
	{
		for (ConstBufferHandle& hBuffer : m_arrConstBufferVertTrans)
		{
			ShaderResourceManager::ReleaseTransBufferHandle(hBuffer);
			hBuffer = INVALID_CONST_BUFFER_HANDLE;
		}
	}
	void RenderViewSingleCamera::SetProjectionMat(const glm::mat4& matProjection)
	{
		m_VPMatBuf.projectionMat = matProjection;
	}
	void RenderViewSingleCamera::SetViewMat(const glm::mat4& matView)
	{
		m_VPMatBuf.viewMat = matView;
	}
	void RenderViewSingleCamera::SetCameraPos(const glm::vec3& vec3CameraPos)
	{
		m_VPMatBuf.cameraPos = glm::vec4(vec3CameraPos, 1.f);
	}
	const VPMatrixBuffer* RenderViewSingleCamera::GetTransBuffer()
	{
		m_VPMatBuf.vpMat = m_VPMatBuf.projectionMat * m_VPMatBuf.viewMat;
		return &m_VPMatBuf;
	}

	ConstBufferHandle RenderViewSingleCamera::GetTransBufferHandle(FlightFrameIndex nFlightIndex) const {
		RENDERER_ASSERT(nFlightIndex < RENDERER_DEFAULT_FLIGHT_FRAME_NUM, "Out of bound.");
		return m_arrConstBufferVertTrans[nFlightIndex];
	}
} // namespace LT

