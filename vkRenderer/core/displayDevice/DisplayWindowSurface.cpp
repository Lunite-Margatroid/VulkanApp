#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "DisplayWindowSurface.hpp"

#include "vkContext.h"

#include "SwapChain.h"

#include "Renderer.h"
#include "Engine.h"

#include "ImageManager.h"


#include "View.hpp"
#include "CameraPerspective.h"

namespace LT {
	DisplayWindowSurface::DisplayWindowSurface(vk::SurfaceKHR surface, uint32_t nWidth, uint32_t nHeight)
		: DisplaySurface(surface)
		, m_nWidth(nWidth)
		, m_nHeight(nHeight)
		, m_nSwapChainImageID(INVALID_IMAGE_ID)
		, m_nSwapChainImageIndex(0)
	{
		vk::Device& device = vkContext::GetVkDevice();

		m_pSwapChain = new SwapChain(nWidth, nHeight, device, vkContext::GetPhysicalDevice(), surface, vk::SharingMode::eExclusive);

		m_nSwapChainImageID = ImageManager::RegisterSwapChainImage(m_pSwapChain);

		for (int i = 0; i < RENDERER_DEFAULT_FLIGHT_FRAME_NUM; i++)
		{
			m_vecSemDrawing.push_back(device.createSemaphore(vk::SemaphoreCreateInfo({})));
			m_vecSemAcquiring.push_back(device.createSemaphore(vk::SemaphoreCreateInfo({})));
		}
	}

	DisplayWindowSurface::~DisplayWindowSurface()
	{
		ImageManager::UnregisterSwapChainImage(m_nSwapChainImageID);
		delete m_pSwapChain;

		vk::Device& device = vkContext::GetVkDevice();

		for (int i = 0; i < RENDERER_DEFAULT_FLIGHT_FRAME_NUM; i++)
		{
			device.destroySemaphore(m_vecSemDrawing[i]);
			device.destroySemaphore(m_vecSemAcquiring[i]);

			m_fences[i] = FenceRef(INVALID_FENCE_HANDLE);
		}
	}
	void DisplayWindowSurface::Resize(uint32_t width, uint32_t height)
	{
		if (m_nWidth != width || m_nHeight != height)
		{
			vkContext::WaitIdel();
			m_pSwapChain->Resize(width, height);
			m_nWidth = width;
			m_nHeight = height;
		}
	}

	void DisplayWindowSurface::Present()
	{
		if (m_bPause)
			return;

		FrameBegin();

		FlightFrameIndex nFlightFrameIndex = m_nFrameIndex % RENDERER_DEFAULT_FLIGHT_FRAME_NUM;
		Renderer* pRenderer = Engine::GetDefaultRenderer();

		if (pRenderer)
		{
			FrameInfo sFrameInfo;

			sFrameInfo.nFrameIndex = m_nFrameIndex;
			sFrameInfo.nFightFrameIndex = nFlightFrameIndex;
			sFrameInfo.vecRenderTargets.push_back(m_nSwapChainImageID);
			sFrameInfo.semAcquiring = m_vecSemAcquiring[nFlightFrameIndex];
			sFrameInfo.semDrawing = m_vecSemDrawing[nFlightFrameIndex];
			sFrameInfo.nWidth = m_nWidth;
			sFrameInfo.nHeight = m_nHeight;

			if (CameraPerspective* pCamera = dynamic_cast<CameraPerspective*>(m_pView->GetCamera()))
			{
				pCamera->SetAspect(static_cast<float>(m_nWidth) / m_nHeight);
			}

			pRenderer->SetView(m_pView);
			pRenderer->SetDisplayDevice(this);
			pRenderer->DrawFrame(sFrameInfo);

			m_fences[nFlightFrameIndex] = sFrameInfo.fenceDrawing;
		}

		FrameEnd();
	}

	void DisplayWindowSurface::FrameBegin()
	{
		DisplaySurface::FrameBegin();

		FlightFrameIndex nFlightFrameIndex = m_nFrameIndex % RENDERER_DEFAULT_FLIGHT_FRAME_NUM;
		vk::Device& device = vkContext::GetVkDevice();

		// 等待上一帧的绘制
		if (m_fences[nFlightFrameIndex])
		{
			vk::Result resultWaitFence = device.waitForFences(m_fences[nFlightFrameIndex].GetNativeFence(), vk::True, std::_Max_limit<uint64_t>());
			RENDERER_ASSERT(resultWaitFence == vk::Result::eSuccess, "Wait for FenceDrawing Failed.");
			// 重置Fence
			//device.resetFences(m_fences[nFlightFrameIndex].GetNativeFence());
		}
		// acquire swapchain image
		m_nSwapChainImageIndex = m_pSwapChain->AcquireNextImage(UINT64_MAX, m_vecSemAcquiring[nFlightFrameIndex], vk::Fence());
		RENDERER_ASSERT(m_nSwapChainImageIndex >= 0, "Acquire Swapchain Image Failed.");

	}
	void DisplayWindowSurface::FrameEnd()
	{
		FlightFrameIndex nFlightFrameIndex = m_nFrameIndex % RENDERER_DEFAULT_FLIGHT_FRAME_NUM;
		vk::Device& device = vkContext::GetVkDevice();

		// 交换链交换缓冲
		uint32_t nImageIndex = static_cast<uint32_t>(m_nSwapChainImageIndex);
		vk::SwapchainKHR swapchain = m_pSwapChain->NativeVKSwapChain();

		vk::PresentInfoKHR pi;
		pi
			.setSwapchainCount(1)
			.setPSwapchains(&swapchain)
			.setPImageIndices(&nImageIndex)
			.setPWaitSemaphores(&m_vecSemDrawing[nFlightFrameIndex])
			.setWaitSemaphoreCount(1)
			;
		// 提交交换链命令
		vk::Result resultPresent = vkContext::GetCmdQueueForSurface().presentKHR(pi);

		if (resultPresent == vk::Result::eErrorOutOfDateKHR || resultPresent == vk::Result::eSuboptimalKHR)
		{
			vkContext::WaitIdel();
			m_pSwapChain->Resize(m_nWidth, m_nHeight);
		}
		else
		{
			RENDERER_ASSERT(resultPresent == vk::Result::eSuccess, "Present Failed.");
		}

		DisplaySurface::FrameEnd();

	}
}