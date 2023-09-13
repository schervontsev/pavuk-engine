#include "Mesh.h"


Mesh::Mesh()
{
}

void Mesh::UpdatePushConstants()
{
    transform = glm::scale(glm::mat4(1.0), scale);
    transform = glm::translate(transform, translation);
    transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
    transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
    transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
    pushConstants.transform = transform;

}