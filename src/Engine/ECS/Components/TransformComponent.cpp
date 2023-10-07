#include "TransformComponent.h"


void TransformComponent::SetEulerAngle(glm::vec3 euler)
{
    rotation = glm::quat(euler);
}

void TransformComponent::AddEulerAngle(glm::vec3 euler)
{
    rotation = glm::quat(euler) * rotation;
}

glm::vec3 TransformComponent::GetForwardVector()
{
    return normalize(glm::vec3(glm::inverse(transform)[2]));
}
