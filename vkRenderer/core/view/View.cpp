#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "View.hpp"
#include "SceneManager.hpp"

#include "CompSprite3D.hpp"

#include "NodeLight.hpp"

namespace LT {
	View::View() 
		: m_nMainScene(INVALID_NODE_ID)
		, m_pCamera(nullptr)
	{
	}

	View::~View()
	{

	}

	void View::GetRenderEntityAndLight(
		std::vector<EntityRender*>& vecOutEntity,
		std::vector<std::pair<LightUniformBuffer, std::bitset<LIGHT_GROUP_MAX_COUNT>>>& vecOutLightBuffer,
		bool& bOutLightDirty) const {


		auto func = [&](Node* pNode) {
			CompSprite3D* pSprite3D = pNode->GetComponent<CompSprite3D>();
			CompTransform3D* pTrans = pNode->GetComponent<CompTransform3D>();
			NodeFlag nNodeFlag = pNode->GetNodeFlag();

			bool bIsLightNode = static_cast<uint32_t>(nNodeFlag) & static_cast<uint32_t>(NodeFlagBits::eIsLightNode);


			if (pTrans)
			{
				if (pNode->GetParent())
				{
					// 输入父节点的世界矩阵
					pTrans->UpdateGlobalMatrix(pNode->GetParent()->GetGlobalMat());
				}
				else
				{
					pTrans->UpdateGlobalMatrix(glm::mat4(1.0f));
				}
			}

			if (pSprite3D)
			{
				EntityRender* pEntity = reinterpret_cast<EntityRender*>(pSprite3D->GetRenderEntity().GetPtr());
				if (pEntity)
				{
					pEntity->SetModuleMat(pNode->GetGlobalMat());
					vecOutEntity.push_back(pEntity);
				}
			}

			if (bIsLightNode && bOutLightDirty)
			{
				// 处理灯光节点
				NodeLight* pNodeLight = 
#ifdef _DEBUG
					dynamic_cast<NodeLight*>(pNode);
				RENDERER_ASSERT(pNodeLight, "Node is marked as Light Node but cannot cast to NodeLight.");
#else
					reinterpret_cast<NodeLight*>(pNode);
#endif

				std::bitset<LIGHT_GROUP_MAX_COUNT> bitGroupFlag = pNodeLight->GetLightGroupBits();
				vecOutLightBuffer.push_back(std::make_pair(LightUniformBuffer{}, bitGroupFlag));
				pNodeLight->FillLightUniformBuffer(&vecOutLightBuffer.back().first);

				nNodeFlag = static_cast<uint32_t>(nNodeFlag) & ~static_cast<uint32_t>(NodeFlagBits::eLightDirty);
			}

			pNode->SetNodeFlag(nNodeFlag);

			};

		Node* pRoot = SceneManager::GetNode(m_nMainScene);
		bOutLightDirty = static_cast<uint32_t>(pRoot->GetNodeFlag()) & static_cast<uint32_t>(NodeFlagBits::eLightDirty);

		// 这里要确保m_nMainScene是一个根节点
		// 否则Transform和LightBuffer的更新可能出错
		if (pRoot)
		{
			pRoot->ForEachBFS_PreOrder(func);
		}
	}
} // namespace LT
