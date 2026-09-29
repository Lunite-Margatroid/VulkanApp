// 材质Shader资源管理器
#include "vkRendererCommon.h"
#include "BufferManager.h"
#include "ShaderResourceManager.hpp"

namespace LT {
	ShaderResourceManager* ShaderResourceManager::s_pInstance = nullptr;

	ShaderResourceManager::ShaderResourceManager()
		:m_nConstBufferHandleCounter(0)
	{
	}

	ShaderResourceManager::~ShaderResourceManager()
	{
		if (m_mapHandle.size() > 0) {
			LOG_ERROR("%s, ConstBuffers did not release.", __FUNCTION__);
		}
	}

	void ShaderResourceManager::Init() {
		if (!s_pInstance)
		{
			s_pInstance = new ShaderResourceManager();
		}
	}

	void ShaderResourceManager::Release() {
		if (s_pInstance)
		{
			delete s_pInstance;
		}
		s_pInstance = nullptr;
	}

	ShaderResourceManager& ShaderResourceManager::GetInstance() {
		RENDERER_ASSERT(s_pInstance, "ShaderResourceManager Did not init.");
		return *s_pInstance;
	}

	BufferBinding ShaderResourceManager::GetConstBufferBinding(ConstBufferHandle nHandle) {
		auto& mgr = GetInstance();
		auto iter = mgr.m_mapHandle.find(nHandle);
		if (iter != mgr.m_mapHandle.end())
		{
			return iter->second;
		}
		return BufferBinding{INVALID_BUFFER_ID, 0, 0};
	}

	void ShaderResourceManager::UpdateConstBuffer(ConstBufferHandle nHandle, void* pData, size_t nOffset, size_t nSize)
	{
		auto& mgr = GetInstance();
		auto iter = mgr.m_mapHandle.find(nHandle);
		if (iter != mgr.m_mapHandle.end())
		{
			BufferBinding binding = iter->second;
			ConstBuffer* pBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(binding.nBufferID));
			if (pBuffer)
			{
				nOffset = binding.nOffset + nOffset;
				RENDERER_ASSERT(binding.nSize - nOffset >= nSize, "Out of bounds");
				pBuffer->UpdateConstBuffer(pData);
			}
		}

	}




} // namespace LT