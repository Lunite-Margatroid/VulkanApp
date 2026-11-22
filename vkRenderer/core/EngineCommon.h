#pragma once
// 深度映射到[0,1]
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// 使用左手坐标系
#define GLM_FORCE_LEFT_HANDED
// 使用实验特性
// 欧拉角支持
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"
#include "glm/matrix.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
// 扩展 欧拉角
#include "glm/gtx/euler_angles.hpp"

#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <map>
#include <variant>
#include <array>


#include "TypeDef.hpp"
#include "MaterialPropDef.hpp"

#include "logger.hpp"


constexpr uint32_t DEFAULT_WIDTH = 1280u;
constexpr uint32_t DEFAULT_HEIGHT = 720u;


constexpr glm::vec3 VEC3_AXIS_X = glm::vec3(1.f, 0.f, 0.f);
constexpr glm::vec3 VEC3_AXIS_Y = glm::vec3(0.f, 1.f, 0.f);
constexpr glm::vec3 VEC3_AXIS_Z = glm::vec3(0.f, 0.f, 1.f);

// Vulkan 屏幕空间默认Y轴朝下
// 引擎屏幕空间Y轴朝上
// 这里变换一下
// 该值会被相机类使用
constexpr glm::mat4 MAT4_SCREEN_TRANS_MAT = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));

namespace LT
{
	enum class BlendMode
	{
		Default
	};


	// 3D 渲染使用的push constant
	struct ModelNormalMat {
		glm::mat4 modelMat;
// 列主序
// 第四行无用 内存对齐 std140
// 实际是一个3x3矩阵
		glm::vec4 normalMatCol[3];

		ModelNormalMat() 
			: modelMat(glm::identity<glm::mat4>()) {
			normalMatCol[0] = glm::vec4(1.f, 0.f, 0.f, 0.f);
			normalMatCol[1] = glm::vec4(0.f, 1.f, 0.f, 0.f);
			normalMatCol[2] = glm::vec4(0.f, 0.f, 1.f, 0.f);
		}
	};

	// std140
	struct VPMatrixBuffer {
		
		glm::mat4 viewMat;
		glm::mat4 projectionMat;
		glm::mat4 vpMat;

		// cameraPos.w 无用 内存对齐
		glm::vec4 cameraPos;

		VPMatrixBuffer()
			: viewMat(glm::identity<glm::mat4>())
			, projectionMat(glm::identity<glm::mat4>())
			, cameraPos(0.f, 0.f, 0.f, 0.f)
			, vpMat(1.0f)
		{

		}
	};
	// std140
	struct VPMatrixBuffer2D {
		// 列主序
		glm::mat2 vpMat;
		glm::vec2 offset;
		VPMatrixBuffer2D() 
			: vpMat(glm::identity<glm::mat2>())
			, offset(0.f, 0.f)
		{
			
		}
	};

	// std140
	// 2D渲染使用push constant
	struct ModelMat2D {
		glm::mat2 modelMat;
		glm::vec2 offset;

		ModelMat2D() 
			: modelMat(1.0f)
			, offset(0.f, 0.f)
		{};
	};
}