#pragma once
#include <glm/vec3.hpp>

struct RotateComponent
{
	glm::vec3 unitRotation = glm::vec3(0.f);
	float speed = 1.f;
};
