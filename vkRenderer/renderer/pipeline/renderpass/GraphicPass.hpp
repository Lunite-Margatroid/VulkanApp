// 编码utf-8
#pragma once
#include "RenderPass.hpp"
#include "DeviceImage.h"

#include "SlangCompiler.hpp"

namespace LT {

	class SwapChain;
	class GraphicPass : public RenderPass
	{
	protected:
		std::vector<std::string> m_vecShaderModuleSrc;
		RenderPassFlag m_nFlag;
		vk::ShaderModule m_vkShaderModule;
		ShaderModuleInfo m_sShaderModuleInfo;

	public:
		~GraphicPass();


		void Init();

		void AddShaderModule(const char* strShaderModule);
		void SetRenderPassFlag(RenderPassFlag nFlag);
		RenderPassFlag GetRenderPassFlag() const;
		
		void SetDepthStencilBuffer(ImageID nDepthStencilID);
		void SetDepthBuffer(ImageID nDepthID);
		void SetRenderTarget(ImageID nColorAttachmentID, uint32_t nBindingIndex);

		void SetVertexBuffer(BufferID nVertexBufferID);
		void SetIndexBuffer(BufferID nIndexBufferID);

		void SetSwapChain(SwapChain* pSwapChain);

		void Execute() override;

	public:
		static void GenVertexAttributeDesc(VertexChannelFlag nVertexChannelFlag, std::vector<vk::VertexInputBindingDescription>& vecInputBindDesc, std::vector<vk::VertexInputAttributeDescription>& vertDesc);

	};
} // namespace LT