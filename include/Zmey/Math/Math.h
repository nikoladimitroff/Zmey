#pragma once

#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Zmey
{

using UVector2 = glm::uvec2;
using Vector2 = glm::vec2;
using Vector3 = glm::vec3;
using Vector4 = glm::vec4;
using Quaternion = glm::quat;
using Matrix4x4 = glm::mat4x4;

inline bool FloatClose(float x, float y, float epsilon = 0.001f)
{
	return fabsf(x - y) < epsilon;
}

template<typename NumberType>
inline NumberType Clamp(NumberType value, NumberType min, NumberType max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

}