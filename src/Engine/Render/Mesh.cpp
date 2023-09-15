#include "Mesh.h"
#include <glm/gtx/quaternion.hpp>

Mesh::Mesh()
{
}

void Mesh::SetEulerAngle(glm::vec3 euler)
{
    rotation = glm::quat(euler);
}

void Mesh::AddEulerAngle(glm::vec3 euler)
{
    rotation = glm::quat(euler) * rotation;
}

void Mesh::UpdatePushConstants()
{
    transform = glm::scale(glm::mat4(1.0), scale);
    transform = glm::translate(transform, translation);
    transform = transform * toMat4(rotation);
    pushConstants.transform = transform;

}
