// 材质基类
#pragma once
#include "IRenderStage.hpp"
#include "RenderPass.hpp"

#include "MaterialPropDef.hpp"

#include "ShaderResourceManager.hpp"

namespace LT {
	struct MaterialSlot {
		vk::DescriptorType eDescType;
		// 如果是纹理 nSrcID是一个ImageID
		// 如果是const buffer, nSrcID是一个ConstBufferHandle，由ShaderResource控制
		int64_t nSrcID;
	};

	

	using MaterialPropDataLayout = std::map<MtlProp, size_t>;

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

		virtual RenderPass* GetRenderPass(RenderStageType eStage, RenderPassFlag nFlag) = 0;
		virtual void UpdateMtlResource(RenderStageType eStage, RenderPassFlag nFlag, FlightFrameIndex nFlightFrameIndex) = 0;
		
		virtual EngineResult SetMtlProp(MtlProp eProp, const MtlPropVar& value) = 0;

		MaterialType GetMaterialType() const { return m_eMtlType; }

		ResultSetter SetSlotSrc(const BindingInfo& sBindingInfo, int64_t nSrcID);
	};

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	class BaseMaterial : public IMaterial
	{
		friend class MaterialManager;
	protected:
		BaseMaterial(MaterialID nID) :IMaterial(nID, eTypeMaterial)
		{
			ConstBufferHandle nBufferHandle = ShaderResourceManager::CreateConstBufferHandle<BaseMaterial<DerivedMaterial, eTypeMaterial>>();
			m_mapSlots[BindingInfo(2u, BindingSpace::eVertAndFragShader)] = MaterialSlot(vk::DescriptorType::eUniformBuffer, -1);
		}

		void RegisterStage(RenderStageType eStage) override
		{
			if (s_mapRenderPasses.find(eStage) == s_mapRenderPasses.end())
			{
				s_mapRenderPasses[eStage] = _RenderPassMap();
			}
		}
		// ---- static -----

	protected:
		static RenderPassMap s_mapRenderPasses;

	protected:
		static MaterialPropDataLayout s_mapMtlPropDataLayout;
		static std::vector<MtlProp> s_vecMtlPropDataLayout;
		static std::set<MtlProp> s_setImage;
		static size_t s_nPropBufferSize;

		static void RegisterMaterialProp();
		static void AddMaterialProp(MtlProp eProp) {
			s_mapMtlPropDataLayout[eProp] = 0;
		}

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
			s_setImage = std::move(mapType2Props[MtlPropDataType::eImage]);

			// Log
			{
				std::ostringstream oss;
				oss << "======= Variant =======\n";
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
				oss << "======= Image =======\n";
				for (MtlProp eProp : s_setImage)
				{
					oss << ToString(eProp) << std::endl;
				}

				LOG_INFO("Init Material Prop Buffer Layout:\n%s", oss.str().c_str());
			}
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
	std::set<MtlProp> BaseMaterial<DerivedMaterial, eTypeMaterial>::s_setImage;

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	std::vector<MtlProp> BaseMaterial<DerivedMaterial, eTypeMaterial>::s_vecMtlPropDataLayout;

	template<typename DerivedMaterial, MaterialType eTypeMaterial>
	size_t BaseMaterial<DerivedMaterial, eTypeMaterial>::s_nPropBufferSize = 0;
} // namespace LT