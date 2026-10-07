// 材质Shader资源管理器
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "BufferManager.h"
#include "ShaderResourceManager.hpp"

namespace LT {
	ShaderResourceManager* ShaderResourceManager::s_pInstance = nullptr;

	ShaderResourceManager::ShaderResourceManager()
		: m_nConstBufferHandleCounter(0)
		, m_sMVPTransBufferPool(sizeof(MVPMatrixBuffer))
		, m_vkDescriptorPool(VK_NULL_HANDLE)
	{
	}

	ShaderResourceManager::~ShaderResourceManager()
	{
		if (m_mapHandle.size() > 0) {
			LOG_ERROR("%s, ConstBuffers did not release.", __FUNCTION__);
		}

		if (m_vkDescriptorPool)
		{
			vkContext::GetVkDevice().destroyDescriptorPool(m_vkDescriptorPool);
		}

		if (m_vkMtlPropDescriptorPool)
		{
			vkContext::GetVkDevice().destroyDescriptorPool(m_vkMtlPropDescriptorPool);
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

	void ShaderResourceManager::CreateDescriptorPool(const std::map<vk::DescriptorType, uint32_t>& mapDescriptorCount) {
		
		// 材质属性描述符
		{
			std::vector<vk::DescriptorPoolSize> vecDPS;
			for (const auto& [eType, nCount] : mapDescriptorCount)
			{
				vk::DescriptorPoolSize dps;
				dps
					.setType(eType)
					.setDescriptorCount(RENDERER_DEFAULT_FLIGHT_FRAME_NUM * nCount)
					;
				vecDPS.push_back(dps);
			}

			vk::DescriptorPoolCreateInfo dpci;
			dpci
				.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
				.setMaxSets(RENDERER_DEFAULT_FLIGHT_FRAME_NUM * static_cast<int>(BindingSpace::BindingSpaceCount) * static_cast<int>(MaterialType::MaterialTypeCount))
				.setPoolSizeCount(vecDPS.size())
				.setPPoolSizes(vecDPS.data())
				;

			GetInstance().m_vkMtlPropDescriptorPool = vkContext::GetVkDevice().createDescriptorPool(dpci);
		}

		// 其他描述符
		{
			// Trans Buffer
			vk::DescriptorPoolSize dps = {};
			dps
				.setType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				;
			vk::DescriptorPoolCreateInfo dpci;
			dpci
				.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
				.setMaxSets(RENDERER_DEFAULT_FLIGHT_FRAME_NUM)
				.setPoolSizeCount(1)
				.setPPoolSizes(&dps)
				;
			GetInstance().m_vkDescriptorPool = vkContext::GetVkDevice().createDescriptorPool(dpci);
		}
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
		return BufferBinding{ INVALID_BUFFER_ID, 0, 0 };
	}

	ConstBufferHandle ShaderResourceManager::CreateTransBufferHandle()
	{
		auto& mgr = GetInstance();
		ConstBufferHandle nHandle = mgr.GenConstBufferHandle();
		mgr.m_mapHandle[nHandle] = mgr.m_sMVPTransBufferPool.AllocateBuffer();

		return nHandle;
	}

	void ShaderResourceManager::ReleaseTransBufferHandle(ConstBufferHandle nHandle)
	{
		auto& mgr = GetInstance();
		auto iter = mgr.m_mapHandle.find(nHandle);
		if (iter != mgr.m_mapHandle.end())
		{
			mgr.m_sMVPTransBufferPool.ReleaseBuffer(iter->second);
			mgr.m_mapHandle.erase(iter);
		}
		else
		{
			LOG_WARNING("invalid const buffer Handle");
		}

	}

	void ShaderResourceManager::UpdateTransBuffer(ConstBufferHandle nHandle, const void* pData)
	{
		auto& mgr = GetInstance();
		auto iter = mgr.m_mapHandle.find(nHandle);
		if (iter != mgr.m_mapHandle.end())
		{
			mgr.m_sMVPTransBufferPool.UpdateConstBuffer(iter->second, pData);
		}
		else
		{
			LOG_WARNING("invalid const buffer Handle");
		}
	}

	void ShaderResourceManager::UpdateDeviceTransBuffer()
	{
		auto& mgr = GetInstance();
		mgr.m_sMVPTransBufferPool.UpdateDeviceConstBuffer();
	}

	void ShaderResourceManager::UpdateDeviceMtlPropBuffer() {
		auto& mgr = GetInstance();
		for (auto& [eMaterialType, pool] : mgr.m_mapBuffers) {
			pool.UpdateDeviceConstBuffer();
		}
	}




} // namespace LT