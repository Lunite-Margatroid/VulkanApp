#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "View.hpp"
#include "CompSprite3D.hpp"

namespace LT {
	View::View() 
		: m_pMainScene(nullptr)
	{
		
	}

	std::vector<EntityRender*> View::GetRenderEntity() const {
		std::vector<EntityRender*> vecEntity;
		CompSprite3D* pSprite3D = dynamic_cast<CompSprite3D*>(m_pMainScene->GetComponent(ComponentType::eSprite3D));
		if (pSprite3D)
		{
			vecEntity.push_back(reinterpret_cast<EntityRender*>(pSprite3D->GetRenderEntity().GetPtr()));
		}
	}
} // namespace LT
