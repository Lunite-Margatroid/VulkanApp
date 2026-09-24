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
			CompSprite3D* pSprite3D = dynamic_cast<CompSprite3D*>(pNode->GetComponent(ComponentType::eSprite3D));
			if (pSprite3D)
			{
				vecEntity.push_back(reinterpret_cast<EntityRender*>(pSprite3D->GetRenderEntity().GetPtr()));
			}
			};

		Node* pRoot = SceneManager::GetNode(m_nMainScene);
		if (pRoot)
		{
			pRoot->ForEach(func);
		}

		return vecEntity;
	}
} // namespace LT
