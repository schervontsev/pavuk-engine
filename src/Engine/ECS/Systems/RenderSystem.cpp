#include "RenderSystem.h"

#include <glm/gtx/quaternion.hpp>

#include "../ECSManager.h"
#include "../Components/TransformComponent.h"
#include "../Components/RenderComponent.h"
#include "../../Render/Vertex.h"
#include "../../ResourceManagers/MeshManager.h"

void RenderSystem::UpdateTransform()
{
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);

		render.pushConstants.transform = transform.transform;
	}

}

void RenderSystem::UpdateCommandBuffer(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout)
{
	uint32_t indicesSize = 0;
	int32_t verticesSize = 0;
	for (auto const& entity : entities) {
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);
		if (render.isVisible) {
			auto mesh = MeshManager::Instance()->GetMesh(render.meshId);
			//upload the matrix to the GPU via push constants
			commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(MeshPushConstants), &render.pushConstants);
			commandBuffer.drawIndexed((uint32_t)mesh->indices.size(), 1, indicesSize, verticesSize, 0);
			indicesSize += mesh->indices.size();
			verticesSize += (uint32_t)mesh->vertices.size();
		}
	}
}

void RenderSystem::UpdateMeshHandles()
{

	for (auto const& entity : entities) {
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);
		render.meshId = MeshManager::Instance()->GetMeshHandle(render.meshName);
	}
}

std::vector<Vertex> RenderSystem::GetVertices()
{
	std::vector<Vertex> result;
	for (auto const& entity : entities) {
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);
		if (render.isVisible) {
			auto mesh = MeshManager::Instance()->GetMesh(render.meshId);
			result.insert(result.end(), mesh->vertices.begin(), mesh->vertices.end());
		}
	}
	return result;
}

std::vector<uint32_t> RenderSystem::GetIndices()
{
	std::vector<uint32_t> result;
	for (auto const& entity : entities) {
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);
		if (render.isVisible) {
			auto mesh = MeshManager::Instance()->GetMesh(render.meshId);
			result.insert(result.end(), mesh->indices.begin(), mesh->indices.end());
		}
	}
	return result;
}
