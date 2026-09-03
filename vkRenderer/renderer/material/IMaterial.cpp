// 材质基类
#include "vkRendererCommon.h"
#include "IMaterial.hpp"

namespace LT {
	IMaterial::IMaterial(MaterialID nID) 
		:m_nID(nID)
	{}

	IMaterial::~IMaterial()
	{
		for (auto& passes : m_mapRenderPasses)
		{
			for (auto& pass : passes.second)
			{
				delete pass.second;
			}
		}
	}

	void IMaterial::RegisterStage(RenderStageType eStage)
	{
		if (m_mapRenderPasses.find(eStage) == m_mapRenderPasses.end())
		{
			m_mapRenderPasses[eStage] = RenderPassMap();
		}
	}

} // namespace