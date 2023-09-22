#include "TransformComponent.h"


void TransformComponent::SetEulerAngle(glm::vec3 euler)
{
    rotation = glm::quat(euler);
}

void TransformComponent::AddEulerAngle(glm::vec3 euler)
{
    rotation = glm::quat(euler) * rotation;
}
