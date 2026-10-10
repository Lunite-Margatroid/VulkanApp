#pragma once
#include "CameraPerspective.h"
#include "CameraOrtho.h"

#include "DisplaySurface.hpp"

namespace LT {
	class Renderer;
	class View;

	class Engine {
	private:
		Renderer* m_pDebugRenderer;
		CameraOrtho m_camera;
		CameraPerspective m_persCamera;


		uint64_t m_nFrameIndex;

		unsigned int m_nWidth;
		unsigned int m_nHeight;

		bool m_bRenderingPaused;

		std::map<CameraID, Camera*> m_mapCamera;


		// debugScene
		CameraID m_nCameraID;
		NodeID m_nMainScene;
		ImageID m_nImageID;
		MaterialID m_nMtlID;
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

		// ---------------------- Scene -------------------------
		EngineResult CreateSceneNode(NodeID& nOutNodeID, NodeType nNodeType);
		EngineResult SceneNodeRebase(NodeID nParient, NodeID nChild);
		// 删除节点会连带子节点一起释放
		EngineResult DeleteSceneNode(NodeID nNodeID);

		EngineResult SetSceneNodePosition(NodeID nNodeID, const std::array<float, 3>& arrPosition);
		// 旋转顺序XYZ 外旋
		// P' = RotateZ * RotateY * RotateX * P
		EngineResult SetSceneNodeEulerRotation(NodeID nNodeID, const std::array<float, 3>& arrEulerRotation);

		EngineResult SetSceneNodeScale(NodeID nNodeID, const std::array<float, 3>& arrScale);
		
		EngineResult AddComponent(NodeID nNodeID, ComponentType eCompType);

		EngineResult CreateCamera(CameraID& nOutCameraID);
		EngineResult DeleteCamera(CameraID nCameraID);

		EngineResult CreateDisplaySurface(DisplaySurface*& pOutSurface, vk::SurfaceKHR, uint32_t nWidth, uint32_t nHeight, DisplayDeviceFlag nFlag);
		EngineResult DeleteDisplaySurface(DisplaySurface* pDiplaySurface);

		// ----------------- build-in static Mesh --------------------
		EngineResult CreateCubeMeshNode(NodeID& nOutNode, float fSideLength);
		EngineResult CreateSphereMeshNode(NodeID& nOutNode, float fRadius, uint32_t nLongSubdivision, uint32_t nLatSubdivision);

		// ------------ Renderer ---------------
		EngineResult CreateMaterial(MaterialID& nOutMtlID, MaterialType eMtlType);
		EngineResult DeleteMaterial(MaterialID nMtlID);

		EngineResult SetMaterialProp(MaterialID nMtlID, MtlProp eProp, const MtlPropVar& vVar);

		EngineResult SetMaterial(NodeID nNodeID, MaterialID nMtlID);


		// ------- static ----------
	private:
		static Renderer* s_pDefaultRenderer;
	public:
		static Renderer* GetDefaultRenderer();
	};
}