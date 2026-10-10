#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "View.hpp"
#include "SceneManager.hpp"

#include "CompSprite3D.hpp"

namespace LT {
	View::View() 
		: m_nMainScene(INVALID_NODE_ID)
		, m_pCamera(nullptr)
	{
		
	}

	std::vector<EntityRender*> View::GetRenderEntity() const {
		std::vector<EntityRender*> vecEntity;

		auto func = [&](Node* pNode) {
			CompSprite3D* pSprite3D = pNode->GetComponent<CompSprite3D>();
			CompTransform3D* pTrans = pNode->GetComponent<CompTransform3D>();

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
					vecEntity.push_back(pEntity);
				}
			}
			};

		Node* pRoot = SceneManager::GetNode(m_nMainScene);
		// 这里要确保m_nMainScene是一个根节点
		// 否则Transform可能出错
		if (pRoot)
		{
			pRoot->ForEachBFS_PreOrder(func);
		}

		return vecEntity;
	}
} // namespace LT
