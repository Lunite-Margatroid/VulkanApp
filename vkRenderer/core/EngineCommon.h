#include "glm/glm.hpp"
#include "glm/matrix.hpp"
#include "glm/gtc/quaternion.hpp"

constexpr glm::vec3 VEC3_AXIS_X = glm::vec3(1.f, 0.f, 0.f);
constexpr glm::vec3 VEC3_AXIS_Y = glm::vec3(0.f, 1.f, 0.f);
constexpr glm::vec3 VEC3_AXIS_Z = glm::vec3(0.f, 0.f, 1.f);

constexpr glm::mat4 MAT4_SCREEN_TRANS_MAT = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));