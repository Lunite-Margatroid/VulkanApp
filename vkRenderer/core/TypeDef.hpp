#pragma once

namespace LT {
	constexpr int64_t INVALID_ITEM_ID = -1;
	using RenderFlagType = uint64_t;
	using FrameIndex = int64_t;
	using FlightFrameIndex = int64_t;


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
	};

	// 这个枚举要作为索引
	enum class ComponentType : int {
		eUnknown = -1,
		eSprite3D,
		eTransform3D,
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
		eInvalidParam
	};

	using ConstBufferHandle = int64_t;
	constexpr ConstBufferHandle INVALID_CONST_BUFFER_HANDLE = -1;


	struct BufferBinding {
		BufferID nBufferID;
		size_t nOffset;
		size_t nSize;
	};

	using MtlPropVar = std::variant<int, float, ImageID, std::array<float, 2>, std::array<float, 3>, std::array<float, 4>, std::array<int, 2>, std::array<int, 3>, std::array<int, 4>>;
}