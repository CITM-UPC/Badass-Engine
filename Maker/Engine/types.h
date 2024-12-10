#pragma once

#include <glm/glm.hpp>

using mat4 = glm::dmat4;
using vec4 = glm::dvec4;
using vec3 = glm::dvec3;

using color1 = glm::u8vec1;
using color2 = glm::u8vec2;
using color3 = glm::u8vec3;
using color4 = glm::u8vec4;

namespace Colors {
	const color4 White(255, 255, 255, 255);
	const color4 Red(255, 0, 0, 255);
	const color4 Green(0, 255, 0, 255);
	const color4 Blue(0, 0, 255, 255);
};