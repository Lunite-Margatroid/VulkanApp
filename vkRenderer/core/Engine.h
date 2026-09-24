#pragma once
#include "CameraPerspective.h"
#include "CameraOrtho.h"

#include "DisplaySurface.hpp"

namespace LT {
	class Renderer;
	class View;

	enum class EngineResult {
		eSuccess,
		eFailed,
	};

	class Engine {
	private:
		Renderer* m_pDebugRenderer;
		CameraOrtho m_camera;
		CameraPerspective m_persCamera;


		uint64_t m_nFrameIndex;

		unsigned int m_nWidth;
		unsigned int m_nHeight;

		bool m_bRenderingPaused;

		std::map<int64_t, Camera*> m_mapCamera;


		// debugScene
		int64_t m_nCameraID;
		int64_t m_nMainScene;
		int64_t m_nImageID;
		int64_t m_nMtlID;
		View* m_pView;

	private:
		int64_t m_nCameraCounter;
		int64_t GenCameraID();

	public:
		Engine();
		~Engine();

		vk::Instance CreateVulkanInstance(const std::vector<const char*>& extensions);
		void InitVulkanContext(vk::SurfaceKHR surface);
		void DestroyVulkanContext();
		vk::Instance GetVkInstance();


		void WaitIdel();

	public:
		void CreateDebugScene(DisplayDevice* pDisplayDivice);
		void UpdateDebugScene();
		void DestroyDebugScene();


		EngineResult CreateSceneNode(int64_t& nOutNodeID, int32_t nNodeType);
		EngineResult SceneNodeRebase(int64_t nParient, int64_t nChild);
		EngineResult DeleteSceneNode(int64_t nNodeID);
		EngineResult DeleteSceneNodeAndChildren(int64_t nNodeID);
		EngineResult CreateCamera(int64_t& nOutCameraID);
		EngineResult DeleteCamera(int64_t nCameraID);

		EngineResult CreateDisplaySurface(DisplaySurface*& pOutSurface, vk::SurfaceKHR, uint32_t nWidth, uint32_t nHeight, DisplayDeviceFlag nFlag);
		EngineResult DeleteDisplaySurface(DisplaySurface* pDiplaySurface);

		// ------- static ----------
	private:
		static Renderer* s_pDefaultRenderer;
	public:
		static Renderer* GetDefaultRenderer();
	};
}