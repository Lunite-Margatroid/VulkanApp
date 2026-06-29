#pragma once
// 深度映射到[0,1]
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// 使用左手坐标系
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/matrix.hpp"
#include "glm/gtc/quaternion.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <filesystem>
#include <fstream>

#include "logger.h"

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