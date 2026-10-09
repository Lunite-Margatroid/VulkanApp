// 材质Shader资源管理器
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "BufferManager.h"
#include "ShaderResourceManager.hpp"

namespace LT {
	ShaderResourceManager* ShaderResourceManager::s_pInstance = nullptr;

	ShaderResourceManager::BufferWithCounter::BufferWithCounter()
		: nBufferID(INVALID_BUFFER_ID)
		, bDirt(false)
	{}

	ShaderResourceManager::BufferWithCounter::BufferWithCounter(BufferID bufferID, bool dirt)
		: nBufferID(bufferID)
		, bDirt(dirt)
	{}

	ShaderResourceManager::ConstBufferPool::ConstBufferPool(size_t nSizePerBlock)
		: m_nSizePerBlock(nSizePerBlock)
	{}

	ShaderResourceManager::ConstBufferPool::~ConstBufferPool()
	{
		for (BufferWithCounter& buffer : m_buffers) {
			if (buffer.flagRef.any())
			{
				LOG_ERROR("The Buffer is being used.");
			}
			BufferManager::DeleteBuffer(buffer.nBufferID);
		}
	}

	ShaderResourceManager::BufferWithCounter& ShaderResourceManager::ConstBufferPool::GetBuffer(BufferID nID)
	{
		for (BufferWithCounter& buffer : m_buffers)
		{
			if (buffer.nBufferID == nID)
			{
				return buffer;
			}
		}
		RENDERER_ASSERT(false, "The Buffer is not belong to this Pool.");
		return m_buffers.front();
	}

	BufferBinding ShaderResourceManager::ConstBufferPool::AllocateBuffer() {
		BufferBinding binding;
		binding.nSize = m_nSizePerBlock;
		bool bNeedAllocate = true;
		for (BufferWithCounter& buffer : m_buffers)
		{
			if (!buffer.flagRef.all()) {
				bNeedAllocate = false;
				for (size_t i = 0; i < buffer.flagRef.size(); ++i)
				{
					if (!buffer.flagRef.test(i))
					{
						binding.nBufferID = buffer.nBufferID;
						binding.nOffset = i * binding.nSize;
						buffer.flagRef.set(i);
						buffer.bDirt = true;
						break;
					}
				}
				break;
			}
		}

		if (bNeedAllocate)
		{
			BufferWithCounter buffer(
				BufferManager::CreateConstBuffer(BLOCK_COUNT_PER_BUFFER_HEAP * m_nSizePerBlock, nullptr)->GetBufferID(),
				true
			);

			binding.nBufferID = buffer.nBufferID;
			binding.nOffset = 0;
			buffer.flagRef.set(0);
			buffer.bDirt = true;

			m_buffers.push_back(std::move(buffer));
		}

		return binding;
	}

	void ShaderResourceManager::ConstBufferPool::ReleaseBuffer(const BufferBinding& binding) {

		RENDERER_ASSERT(m_nSizePerBlock == binding.nSize, "unexpected const buffer size.");

		bool bFound = false;
		for (BufferWithCounter& buffer : m_buffers)
		{
			if (buffer.nBufferID == binding.nBufferID) {
				size_t nIndex = binding.nOffset / binding.nSize;
				RENDERER_ASSERT(buffer.flagRef.test(nIndex), "The Buffer is not used.");
				buffer.flagRef.reset(nIndex);
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			LOG_WARNING("%s: Can't find the Buffer.", __FUNCTION__);
		}
	}

	void ShaderResourceManager::ConstBufferPool::UpdateConstBuffer(const BufferBinding& binding, const void* pData, size_t nOffset, size_t nSize)
	{
		BufferWithCounter& buffer = GetBuffer(binding.nBufferID);
		buffer.bDirt = true;

		ConstBuffer* pBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(binding.nBufferID));
		if (pBuffer)
		{
			RENDERER_ASSERT(binding.nSize - nOffset >= nSize, "Out of bounds");
			nOffset = binding.nOffset + nOffset;
			pBuffer->UpdateConstBuffer(pData, nOffset, nSize);
		}
	}

	void ShaderResourceManager::ConstBufferPool::UpdateConstBuffer(const BufferBinding& binding, const void* pData)
	{
		UpdateConstBuffer(binding, pData, 0, m_nSizePerBlock);
	}

	void ShaderResourceManager::ConstBufferPool::UpdateDeviceConstBuffer() {
		for (BufferWithCounter& buffer : m_buffers)
		{
			if (buffer.bDirt)
			{
				ConstBuffer* pBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(buffer.nBufferID));
				if (pBuffer)
				{
					pBuffer->UpdateConstBuffer();
				}
				else
				{
					LOG_ERROR("The Buffer is not Const Buffer.");
				}
				buffer.bDirt = false;
			}
		}
	}

	ShaderResourceManager::ShaderResourceManager()
		: m_nConstBufferHandleCounter(0)
		, m_sMVPTransBufferPool(sizeof(MVPMatrixBuffer))
		, m_vkDescriptorPool(VK_NULL_HANDLE)
	{
	}

	ShaderResourceManager::~ShaderResourceManager()
	{
		vk::Device& device = vkContext::GetVkDevice();

		if (m_mapHandle.size() > 0) {
			LOG_ERROR("%s, ConstBuffers did not release.", __FUNCTION__);
		}


		if (m_arrTransBufferDescriptorSet[0])
		{
			device.freeDescriptorSets(m_vkDescriptorPool, m_arrTransBufferDescriptorSet);
		}

		if (m_vkTransBufferDescriptorSetLayout) {
			device.destroyDescriptorSetLayout(m_vkTransBufferDescriptorSetLayout);
		}

		if (m_vkDescriptorPool)
		{
			device.destroyDescriptorPool(m_vkDescriptorPool);
		}

		if (m_vkMtlPropDescriptorPool)
		{
			device.destroyDescriptorPool(m_vkMtlPropDescriptorPool);
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
				.setMaxSets(RENDERER_DEFAULT_FLIGHT_FRAME_NUM * static_cast<int>(ShaderResSpace::ShaderResSpaceCount) * static_cast<int>(MaterialType::MaterialTypeCount))
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

	vk::DescriptorPool ShaderResourceManager::GetDescriptorPool() {
		return GetInstance().m_vkDescriptorPool;
	}

	vk::DescriptorPool ShaderResourceManager::GetMtlPropDescriptorPool() {
		return GetInstance().m_vkMtlPropDescriptorPool;
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

	ConstBufferHandle ShaderResourceManager::GenConstBufferHandle() {
		return m_nConstBufferHandleCounter++;
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

	vk::DescriptorSetLayout ShaderResourceManager::GetTransBufferDescriptorSetLayout() {

		ShaderResourceManager& mgr = GetInstance();

		if (!mgr.m_vkTransBufferDescriptorSetLayout)
		{
			vk::Device& device = vkContext::GetVkDevice();

			std::array<vk::DescriptorSetLayoutBinding, 1> bindings;
			bindings[0]
				.setBinding(MTL_TRANS_BUFFER_BINDING_INDEX)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(GetShaderStageFlag(MTL_TRANS_BUFFER_BINDING_SPACE))
				;

			vk::DescriptorSetLayoutCreateInfo dslci = {};
			dslci
				.setBindings(bindings)
				;

			mgr.m_vkTransBufferDescriptorSetLayout = device.createDescriptorSetLayout(dslci);

		}

		return mgr.m_vkTransBufferDescriptorSetLayout;
	}


	// 获取Trans Buffer描述符
	vk::DescriptorSet ShaderResourceManager::GetTransBufferDescriptorSet(FlightFrameIndex nFlightIndex) {
		ShaderResourceManager& mgr = GetInstance();

		if (!mgr.m_arrTransBufferDescriptorSet[0])
		{
			vk::Device& device = vkContext::GetVkDevice();

			vk::DescriptorSetAllocateInfo dsai = {};
			dsai
				.setDescriptorPool(mgr.m_vkDescriptorPool)
				.setDescriptorSetCount(1)
				.setPSetLayouts(&mgr.m_vkTransBufferDescriptorSetLayout)
				;

			for (int i = 0; i < mgr.m_arrTransBufferDescriptorSet.size(); i++)
			{
				std::vector<vk::DescriptorSet> descSets = device.allocateDescriptorSets(dsai);
				mgr.m_arrTransBufferDescriptorSet[i]= descSets[0];
			}
		}

		return mgr.m_arrTransBufferDescriptorSet[nFlightIndex];
	}

	void ShaderResourceManager::UpdateDeviceMtlPropBuffer() {
		auto& mgr = GetInstance();
		for (auto& [eMaterialType, pool] : mgr.m_mapBuffers) {
			pool.UpdateDeviceConstBuffer();
		}
	}




} // namespace LT