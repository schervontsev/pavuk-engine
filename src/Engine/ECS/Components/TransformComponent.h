#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct TransformComponent
{
	glm::vec3 translation = glm::vec3(0.f);
	glm::quat rotation = glm::quat(0.f, 0.f, 0.f, 1.f);
	glm::vec3 scale = glm::vec3(1.f);

	glm::mat4 transform = glm::mat4(1.f);

	void SetEulerAngle(glm::vec3 euler);
	void AddEulerAngle(glm::vec3 euler);
	glm::vec3 GetForwardVector();
};

