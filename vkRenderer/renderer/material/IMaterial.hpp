// 材质基类
#pragma once
#include "IRenderStage.hpp"
#include "RenderPass.hpp"

#include "MaterialPropDef.hpp"

#include "ShaderResourceManager.hpp"
#include "SamplerManager.h"

namespace LT {
	struct MaterialSlot {
		vk::DescriptorType eDescType;
		// 如果是纹理 nSrcID是一个ImageID
		// 如果是const buffer, nSrcID是一个ConstBufferHandle，由ShaderResource控制
		int64_t nSrcID;

		MaterialSlot(vk::DescriptorType desc, int64_t srcID)
			: eDescType(desc)
			, nSrcID(srcID)
		{}

		MaterialSlot()
			: eDescType(vk::DescriptorType::eUniformBuffer)
			, nSrcID(-1)
		{
		}
	};

	struct MaterialBindInfo {
		RenderStageType eStage;
		RenderPassFlag nFlag;
		FlightFrameIndex nFlightIndex;
	};

	using MaterialPropDataLayout = std::map<MtlProp, size_t>;

	// 储存材质属性的Desc Set的结构体
	struct MtlPropDescriptorSets {
		using _DescSets = std::array<vk::DescriptorSet, static_cast<size_t>(ShaderResSpace::ShaderResSpaceCount)>;

		std::array<_DescSets, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_descriptorSets;

		MtlPropDescriptorSets();

		void Init(const std::array<vk::DescriptorSetLayout, static_cast<size_t>(ShaderResSpace::ShaderResSpaceCount)>& layouts);

		vk::DescriptorSet GetDescriptorSet(FlightFrameIndex nFlightIndex, ShaderResSpace eSpace);

		std::array<vk::DescriptorSet, static_cast<size_t>(ShaderResSpace::ShaderResSpaceCount)> GetDescriptorSet(FlightFrameIndex nFlightIndex);

		operator bool() const;
	};

	class IMaterial {
		friend class MaterialManager;
	protected:
		MaterialID m_nID;
		MaterialType m_eMtlType;
		std::unordered_map<BindingInfo, MaterialSlot, BindingInfoHash> m_mapSlots;

	protected:
		IMaterial(MaterialID nID, MaterialType eType);

		virtual void RegisterStage(RenderStageType eStage) = 0;
	public:
		using _RenderPassMap = std::map<RenderPassFlag, RenderPass*>;
		using RenderPassMap = std::map<RenderStageType, _RenderPassMap>;

		virtual ~IMaterial();

		virtual RenderPass* Bind(const MaterialBindInfo& sMtlBindInfo) = 0;
		//virtual void UpdateMtlResource(RenderStageType eStage, RenderPassFlag nFlag) = 0;
		
		virtual EngineResult SetMtlProp(MtlProp eProp, const MtlPropVar& value) = 0;

		EngineResult SetTransBuffer(ConstBufferHandle nHandle);

		MaterialType GetMaterialType() const { return m_eMtlType; }

		EngineResult SetSlotSrc(const BindingInfo& sBindingInfo, int64_t nSrcID);

	};

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	class BaseMaterial : public IMaterial
	{
		friend class MaterialManager;
	protected:

		using TBaseMaterial = BaseMaterial<DerivedMaterial, eTypeMaterial>;

		ConstBufferHandle m_nPropBufferHandle;

		BaseMaterial(MaterialID nID) :IMaterial(nID, eTypeMaterial)
		{
			m_nPropBufferHandle = ShaderResourceManager::CreateMtlPropBufferHandle<TBaseMaterial>();
			// Mtl Prop Buffer
			m_mapSlots[BindingInfo(MTL_PROP_BINDING_INDEX, MTL_PROP_BINDING_SPACE, ShaderResSpace::eMtlProp)] = MaterialSlot(vk::DescriptorType::eUniformBuffer, static_cast<int64_t>(m_nPropBufferHandle));
			// Mtl Prop tex
			for (const auto& [eProp, nBinding] : s_setImage) {
				m_mapSlots[BindingInfo(nBinding, MTL_TEX_BINDING_SPACE, ShaderResSpace::eMtlProp)] = MaterialSlot(vk::DescriptorType::eCombinedImageSampler, INVALID_IMAGE_ID);
			}
		}

		void RegisterStage(RenderStageType eStage) override
		{
			if (s_mapRenderPasses.find(eStage) == s_mapRenderPasses.end())
			{
				s_mapRenderPasses[eStage] = _RenderPassMap();
			}
		}

		//void UpdateMtlResource(RenderStageType eStage, RenderPassFlag nFlag) override
		//{
		//	GraphicPass* pRenderPass = dynamic_cast<GraphicPass*>(GetRenderPass(eStage, nFlag));
		//	if (pRenderPass) {
		//		for (const auto& [bindingInfo, mtlSlot] : m_mapSlots)
		//		{
		//			if (mtlSlot.nSrcID >= 0)
		//			{
		//				switch (mtlSlot.eDescType)
		//				{
		//				case vk::DescriptorType::eUniformBuffer:
		//					pRenderPass->BindConstBuffer(mtlSlot.nSrcID, bindingInfo.eSpace, bindingInfo.nIndex);
		//					break;
		//				case vk::DescriptorType::eCombinedImageSampler:
		//					pRenderPass->BindImage2D(mtlSlot.nSrcID, bindingInfo.eSpace, bindingInfo.nIndex);
		//					break;
		//				default:
		//					break;
		//				};
		//			}
		//		}
		//	}
		//}

	public:
		EngineResult SetMtlProp(MtlProp eProp, const MtlPropVar& value) override {
			MtlPropDataType eDataType = GetMtlPropDataType(eProp);
			// 纹理类型的属性
			if (eDataType >= MtlPropDataType::eImage && eDataType < MtlPropDataType::MtlPropDataTypeCount)
			{
				auto iter = s_setImage.find(eProp);
				if (iter == s_setImage.end())
				{
					return EngineResult::eInvalidParam;
				}
				m_mapSlots[BindingInfo(iter->second, MTL_TEX_BINDING_SPACE, ShaderResSpace::eMtlProp)] = MaterialSlot(vk::DescriptorType::eSampledImage, std::get<ImageID>(value));
			}
			else
			{
				auto iter = s_mapMtlPropDataLayout.find(eProp);
				if (iter == s_mapMtlPropDataLayout.end())
				{
					return EngineResult::eInvalidParam;
				}

				// 变量类型的属性
				ShaderResourceManager::UpdateMtlPropBuffer<TBaseMaterial>(m_nPropBufferHandle, &value, s_mapMtlPropDataLayout[eProp], SizeOf(eProp));
			}
			return EngineResult::eSuccess;
		}

		void BindShaderResource(const MaterialBindInfo& sMtlBindInfo) {
			// 绑定资源
			std::vector<vk::WriteDescriptorSet> vecWDS;
			for (const auto& [bindings, mtlSlot] : m_mapSlots) {

				if (mtlSlot.nSrcID == INVALID_ITEM_ID)
					continue;

				vk::WriteDescriptorSet wds;
				bool bInvalidSrc = true;

				switch (mtlSlot.eDescType) {
				case vk::DescriptorType::eUniformBuffer:
				{
					BufferBinding binding = ShaderResourceManager::GetConstBufferBinding(mtlSlot.nSrcID);
					Buffer* pBuffer = BufferManager::GetBuffer(binding.nBufferID);

					vk::DescriptorBufferInfo dbi = {};
					dbi
						.setBuffer(pBuffer->GetNativeBuffer())
						.setOffset(binding.nOffset)
						.setRange(binding.nSize)
						;


					wds.setPBufferInfo(&dbi);
				}
				break;
				case vk::DescriptorType::eCombinedImageSampler:
				{
					vk::DescriptorImageInfo ddi = {};

					ddi
						.setImageView(ImageManager::GetNativeDeviceImageView(mtlSlot.nSrcID))
						.setSampler(SamplerManager::GetDefaultImageSampler()->GetNativeSampler())
						.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
						;

					wds.setPImageInfo(&ddi);
				}
				break;
				default:
					bInvalidSrc = false;
					break;
				}

				if (bInvalidSrc)
				{
					wds
						.setDstBinding(bindings.nIndex)
						.setDstArrayElement(0)
						.setDescriptorCount(1)
						.setDescriptorType(mtlSlot.eDescType)
						;

					switch (bindings.eResSpace)
					{
					case ShaderResSpace::eTransBuffer:
						{
							wds.setDstSet(ShaderResourceManager::GetTransBufferDescriptorSet(sMtlBindInfo.nFlightIndex));
							break;
						}
					case ShaderResSpace::eMtlProp:
						{
							wds.setDstSet(s_sDescriptorSets.GetDescriptorSet(sMtlBindInfo.nFlightIndex, bindings.eSpace));
							break;
						}
					default:
						break;
					}
					vecWDS.push_back(std::move(wds));
				}
			}

			if (!vecWDS.empty())
			{
				vkContext::GetVkDevice().updateDescriptorSets(vecWDS, {});
			}
		}

		// ---- static -----
	public:
		// 声明材质属性变量的着色器代码
		static std::string GenMtlPropShaderModule() {
			std::ostringstream oss;
			if (s_vecMtlPropDataLayout.size() > 0)
			{
				oss
					<< "struct MtlProps {" << std::endl;

				int paddingCounter = 0;

				for (MtlProp eProp : s_vecMtlPropDataLayout) {
					if (eProp == MtlProp::Padding)
					{
						oss << "float padding" << paddingCounter++ << ";" << std::endl;
					}
					else
					{
						oss << ToString(GetMtlPropDataType(eProp)) << " " << ToString(eProp) << ";" << std::endl;
					}
				}


				oss << "};" << std::endl
					<< "[[vk::binding(" << MTL_PROP_BINDING_INDEX << ", " << static_cast<int>(ShaderResSpace::eMtlProp) << ")]]" << std::endl
					<< "ConstantBuffer<MtlProps, Std140DataLayout> " << MTL_PROP_UNIFORM_NAME << "; " << std::endl;
			}

			for (const auto& [eProp, bindingIndex] : s_setImage)
			{
				oss << "[[vk::binding(" << bindingIndex << ", " << static_cast<int>(ShaderResSpace::eMtlProp) << ")]]" << std::endl
					<< "Sampler2D " << ToString(eProp) << ";" << std::endl;
			}

			return oss.str();
		}
	protected:

		// ------------- 静态变量 ------------
		static RenderPassMap s_mapRenderPasses;
		// 属性在数据块中的offset /字节
		static MaterialPropDataLayout s_mapMtlPropDataLayout;
		// 按照内存顺序记录的属性
		static std::vector<MtlProp> s_vecMtlPropDataLayout;
		// 纹理类型的属性和binding index
		static std::map<MtlProp, uint32_t> s_setImage;
		// 材质属性内存块的大小
		static size_t s_nPropBufferSize;
		// 材质属性相关的DescriptorSetLayout
		static std::array<vk::DescriptorSetLayout, static_cast<size_t>(BindingSpace::BindingSpaceCount)> s_arrDescriptorSetLayout;
		// 材质属性相关的Desc Set
		static MtlPropDescriptorSets s_sDescriptorSets;

		// ------------------------------------

		// 获取材质属性描述符的绑定索引
		static void GetDescriptorBindings(std::vector<std::pair<vk::DescriptorType, BindingInfo>>& bindings) {
			// Mtl Prop
			bindings.emplace_back(vk::DescriptorType::eUniformBuffer, BindingInfo(MTL_PROP_BINDING_INDEX, MTL_PROP_BINDING_SPACE));
			// Mtl Prop Texture
			for (const auto& [eProp, nBindingIndex] : s_setImage)
			{
				bindings.emplace_back(vk::DescriptorType::eCombinedImageSampler, BindingInfo(nBindingIndex, MTL_TEX_BINDING_SPACE));
			}
		}

		// 注册材质属性 由子类实现
		static void RegisterMaterialProp();
		// 添加属性 属性集合初始化后`InitMaterialPropDataLayout`处理布局
		static void AddMaterialProp(MtlProp eProp) {
			s_mapMtlPropDataLayout[eProp] = 0;
		}
		// 初始化材质属性内存布局
		static void InitMaterialPropDataLayout() {
			std::map<MtlPropDataType, std::set<MtlProp>> mapType2Props;
			for (const auto& [eProp, offset] : s_mapMtlPropDataLayout)
			{
				mapType2Props[GetMtlPropDataType(eProp)].insert(eProp);
			}
			s_mapMtlPropDataLayout.clear();

			size_t nOffset = 0;

			// int4
			if (mapType2Props.find(MtlPropDataType::eInt4) != mapType2Props.end())
			{
				auto& setProp = mapType2Props[MtlPropDataType::eInt4];
				for (MtlProp eProp : setProp)
				{
					s_mapMtlPropDataLayout[eProp] = nOffset;
					s_vecMtlPropDataLayout.push_back(eProp);
					nOffset += 16;
				}
			}
			// float4
			if (mapType2Props.find(MtlPropDataType::eFloat4) != mapType2Props.end())
			{
				auto& setProp = mapType2Props[MtlPropDataType::eFloat4];
				for (MtlProp eProp : setProp)
				{
					s_mapMtlPropDataLayout[eProp] = nOffset;
					s_vecMtlPropDataLayout.push_back(eProp);
					nOffset += 16;
				}
			}
			// int3 float3 int float bool
			// int2 float2
			{
				std::set<MtlProp> setd3;
				std::set<MtlProp> setd2;
				std::set<MtlProp> setd1;
				setd3.insert(mapType2Props[MtlPropDataType::eInt3].begin(), mapType2Props[MtlPropDataType::eInt3].end());
				setd3.insert(mapType2Props[MtlPropDataType::eFloat3].begin(), mapType2Props[MtlPropDataType::eFloat3].end());

				setd2.insert(mapType2Props[MtlPropDataType::eInt2].begin(), mapType2Props[MtlPropDataType::eInt2].end());
				setd2.insert(mapType2Props[MtlPropDataType::eFloat2].begin(), mapType2Props[MtlPropDataType::eFloat2].end());

				setd1.insert(mapType2Props[MtlPropDataType::eInt].begin(), mapType2Props[MtlPropDataType::eInt].end());
				setd1.insert(mapType2Props[MtlPropDataType::eFloat].begin(), mapType2Props[MtlPropDataType::eFloat].end());
				setd1.insert(mapType2Props[MtlPropDataType::eBool].begin(), mapType2Props[MtlPropDataType::eBool].end());

				// 先塞vec2
				for (MtlProp eProp : setd2)
				{
					s_mapMtlPropDataLayout[eProp] = nOffset;
					s_vecMtlPropDataLayout.push_back(eProp);
					nOffset += 8;
				}

				auto funcPad16 = [&]() {
					while (nOffset % 16 != 0)
					{
						if (setd1.size() > 0)
						{
							auto iter = setd1.begin();
							s_mapMtlPropDataLayout[*iter] = nOffset;
							s_vecMtlPropDataLayout.push_back(*iter);
							setd1.erase(iter);
							nOffset += 4;
						}
						else
						{
							s_vecMtlPropDataLayout.push_back(MtlProp::Padding);
							nOffset += 4;
						}
					}
				};

				// 再塞vec3
				for (MtlProp eProp : setd3)
				{
					// 对齐16字节
					funcPad16();
					s_mapMtlPropDataLayout[eProp] = nOffset;
					s_vecMtlPropDataLayout.push_back(eProp);
					nOffset += 12;
				}

				// 最后处理标量
				while (!setd1.empty())
				{
					auto iter = setd1.begin();
					s_mapMtlPropDataLayout[*iter] = nOffset;
					s_vecMtlPropDataLayout.push_back(*iter);
					nOffset += 4;
					setd1.erase(iter);
				}
				// 补齐到16字节
				funcPad16();

			}


			s_nPropBufferSize = nOffset;

			// Image
			s_setImage.clear();
			uint32_t imageBindingCounter = MTL_TEX_BINDING_MIN;
			for (MtlProp eProp : mapType2Props[MtlPropDataType::eImage])
			{
				s_setImage[eProp] = imageBindingCounter++;
			}

			// Log
			{
				std::ostringstream oss;
				oss << "=========== Variant ===========\n";
				for (MtlProp eProp : s_vecMtlPropDataLayout)
				{
					if (eProp == MtlProp::Padding)
					{
						oss << "Padding 4 bytes\n";
					}
					else
					{
						oss << ToString(GetMtlPropDataType(eProp)) << '\t' << ToString(eProp) << "\toffset: " << s_mapMtlPropDataLayout[eProp] << std::endl;
					}
				}
				oss << "============ Image ============\n";
				for (const auto& [eProp, bindingIndex] : s_setImage)
				{
					oss << ToString(eProp) << std::endl;
				}

				oss << "======== Shader Define ========\n";
				oss << GenMtlPropShaderModule() << std::endl;

				LOG_INFO("Init Material Prop Buffer Layout:\n%s", oss.str().c_str());
			}
		}

		static std::array<vk::DescriptorSetLayout, static_cast<size_t>(BindingSpace::BindingSpaceCount)> GetMtlPropDescriptorSetLayout() {

			for (const auto& layout : s_arrDescriptorSetLayout)
			{
				if (layout)
					return s_arrDescriptorSetLayout;
			}

			std::vector<std::pair<vk::DescriptorType, BindingInfo>> vecDescriptors;
			vk::Device& device = vkContext::GetVkDevice();

			// 收集MtlProp描述符
			GetDescriptorBindings(vecDescriptors);

			std::array<std::vector<vk::DescriptorSetLayoutBinding>, static_cast<size_t>(BindingSpace::BindingSpaceCount)> arrLayoutBindings;

			for (const auto& [eType, sBinding] : vecDescriptors)
			{
				vk::DescriptorSetLayoutBinding dslb = {};
				dslb
					.setBinding(sBinding.nIndex)
					.setDescriptorType(eType)
					.setStageFlags(GetShaderStageFlag(sBinding.eSpace))
					.setDescriptorCount(1)
					;

				arrLayoutBindings[static_cast<size_t>(sBinding.eSpace)].push_back(dslb);
			}


			for (int i = 0; i < arrLayoutBindings.size(); i++)
			{
				vk::DescriptorSetLayoutCreateInfo dslci = {};
				dslci.setBindings(arrLayoutBindings[i]);
				s_arrDescriptorSetLayout[i] = device.createDescriptorSetLayout(dslci);
			}

			return s_arrDescriptorSetLayout;
		}

		static void InitMtlPropDescriptorSet() {
			s_sDescriptorSets.Init(GetMtlPropDescriptorSetLayout());
		}

	public:
		static MaterialType GetMaterialType() { return eTypeMaterial; }
		static size_t GetPropBufferSize() { return s_nPropBufferSize; }

	};

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	IMaterial::RenderPassMap BaseMaterial<DerivedMaterial, eTypeMaterial>::s_mapRenderPasses;

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	MaterialPropDataLayout BaseMaterial<DerivedMaterial, eTypeMaterial>::s_mapMtlPropDataLayout;

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	std::map<MtlProp, uint32_t> BaseMaterial<DerivedMaterial, eTypeMaterial>::s_setImage;

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	std::vector<MtlProp> BaseMaterial<DerivedMaterial, eTypeMaterial>::s_vecMtlPropDataLayout;

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	size_t BaseMaterial<DerivedMaterial, eTypeMaterial>::s_nPropBufferSize = 0;

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	MtlPropDescriptorSets BaseMaterial<DerivedMaterial, eTypeMaterial>::s_sDescriptorSets;

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	std::array<vk::DescriptorSetLayout, static_cast<size_t>(BindingSpace::BindingSpaceCount)> BaseMaterial<DerivedMaterial, eTypeMaterial>::s_arrDescriptorSetLayout;
} // namespace LT