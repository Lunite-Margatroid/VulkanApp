#pragma once

namespace LT {
	constexpr int64_t INVALID_ITEM_ID = -1;
	using RenderFlagType = uint64_t;
	using FrameIndex = int64_t;
	using FlightFrameIndex = int64_t;

	using FenceHandle = int64_t;
	constexpr FenceHandle INVALID_FENCE_HANDLE = -1;

	using BufferID = int64_t;
	constexpr int64_t INVALID_BUFFER_ID = INVALID_ITEM_ID;

	using ImageID = int64_t;

	constexpr ImageID INVALID_IMAGE_ID = INVALID_ITEM_ID;

	constexpr ImageID SWAPCHAIN_IMAGE_ID_MIN = -2;
	constexpr ImageID SWAPCHAIN_IMAGE_ID_MAX = -65534;

	using NodeID = int64_t;
	constexpr NodeID INVALID_NODE_ID = INVALID_ITEM_ID;
	enum class NodeType : int {
		eUnknown = -1,
		eNode,
		eNodeMesh,
		eNodePointLight,
		eNodeSphereLight,
		eNodeSpotLight,
		eNodeRectLight,
		eNodeDirectionalLight,
		eNodeAmbient
	};

	// 这个枚举要作为索引
	enum class ComponentType : int {
		eUnknown = -1,
		eTransform3D,
		eSprite3D,
		ComponentTypeCount,
		eCustomedComponent = 8192, // 自定义组件类型起始值
	};

	using EntityID = int64_t;
	constexpr EntityID INVALID_ENTITY_ID = -1;



	using MaterialID = int64_t;
	constexpr MaterialID INVALID_MATERIAL_ID = INVALID_ITEM_ID;

	enum class MaterialType : int{
		eUndifined = -1,
		eMainTexture,
		eExample,
		eError, // 着色器编译错误或者材质ID不存在则使用该材质
		MaterialTypeCount
	};


	using MeshID = int64_t;
	constexpr MeshID INVALID_MESH_ID = INVALID_ITEM_ID;

	using ImageSamplerID = int64_t;
	constexpr ImageSamplerID INVALID_SAMPLER_ID = INVALID_ITEM_ID;

	using CameraID = int64_t;
	constexpr CameraID INVALID_CAMERA_ID = INVALID_ITEM_ID;

	enum class EngineResult {
		eSuccess,
		eFailed,
		eInvalidParam, // 无效参数
		eNoComponent, // Component不存在
		eNoSceneNode, // Node 不存在
		eNoMaterial, // Mtl不存在
	};

	using ConstBufferHandle = int64_t;
	constexpr ConstBufferHandle INVALID_CONST_BUFFER_HANDLE = -1;


	struct BufferBinding {
		BufferID nBufferID;
		size_t nOffset;
		size_t nSize;
	};


	constexpr uint32_t LIGHT_GROUP_MAX_COUNT = 16u;
	constexpr uint32_t LIGHT_MAX_COUNT_PER_GROUP = 1024u;
	constexpr uint32_t LIGHT_MAX_COUNT_PER_GROUP_FORWARD = 16u;

	enum class LightType : int {
		eNone = -1,
		eLightPoint,
		eLightSphere,
		eLightSpot,
		eLightRect,
		eLightDirectional,
		eLightAmbient
	};

	//std::140
	// 64byte
	struct LightUniformBuffer {
		std::array<float, 3> position; // 12byte
		float attenuation0; // 4byte
		std::array<float, 3> direction; // 16byte
		float attenuation1; // 4byte
		std::array<float, 3> color; // 16byte
		LightType nLightType; // -1标志该块缓冲不起作用
		// 8byte
		union {
			struct {
				float rectU;
				float rectV;
			};
			struct {
				float radius;
				float padding;
			};
		};
		// 8byte
		float radianOuter;
		float radianInner;
	};

	using TPropVar = std::variant<int, float, ImageID, std::array<float, 2>, std::array<float, 3>, std::array<float, 4>, std::array<int, 2>, std::array<int, 3>, std::array<int, 4>>;

	using MtlPropVar = TPropVar;
	using SceneNodePropVar = TPropVar;

	struct RGBA_8 {
		unsigned char r, g, b, a;
	};

	struct RGBA_32 {
		float r, g, b, a;
	};

	struct RGB_8 {
		unsigned char r, g, b;
	};

	struct RGB_32 {
		float r, g, b;
	};
}