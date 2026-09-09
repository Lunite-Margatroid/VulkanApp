#pragma once
// 深度映射到[0,1]
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// 使用左手坐标系
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/matrix.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <filesystem>
#include <fstream>

#include "logger.hpp"

constexpr glm::vec3 VEC3_AXIS_X = glm::vec3(1.f, 0.f, 0.f);
constexpr glm::vec3 VEC3_AXIS_Y = glm::vec3(0.f, 1.f, 0.f);
constexpr glm::vec3 VEC3_AXIS_Z = glm::vec3(0.f, 0.f, 1.f);

constexpr glm::mat4 MAT4_SCREEN_TRANS_MAT = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));

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

namespace LT
{

	// std130
	struct MVPMatrixBuffer {
		glm::mat4 modelMat;
		glm::mat4 viewMat;
		glm::mat4 projectionMat;
		glm::mat4 mvpMat;
		// 列主序
		// 第四行无用 内存对齐 std130
		// 实际是一个3x3矩阵
		glm::vec4 normalMatCol[3];
		// cameraPos.w 无用 内存对齐
		glm::vec4 cameraPos;

		MVPMatrixBuffer()
			: modelMat(glm::identity<glm::mat4>())
			, viewMat(glm::identity<glm::mat4>())
			, projectionMat(glm::identity<glm::mat4>())
			, mvpMat(glm::identity<glm::mat4>())
			, cameraPos(0.f, 0.f, 0.f, 0.f)
		{
			normalMatCol[0] = glm::vec4(1.f, 0.f, 0.f, 0.f);
			normalMatCol[1] = glm::vec4(0.f, 1.f, 0.f, 0.f);
			normalMatCol[2] = glm::vec4(0.f, 0.f, 1.f, 0.f);
		}
	};
}