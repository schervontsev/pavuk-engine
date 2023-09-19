#include "RenderSystem.h"
#include "../ECSManager.h"
#include "../Components/TransformComponent.h"
#include "../Components/RenderComponent.h"
#include "../../Render/Vertex.h"

void RenderSystem::Update(float dt)
{
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);

		render.pushConstants.transform = transform.transform;

	}

}

std::vector<Vertex> RenderSystem::GetVertices()
{
	std::vector<Vertex> result;
	for (auto const& entity : entities) {
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);
		result.insert(result.end(), render.vertices.begin(), render.vertices.end());
	}
	return result;
}

std::vector<uint32_t> RenderSystem::GetIndices()
{
	std::vector<uint32_t> result;
	for (auto const& entity : entities) {
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);
		result.insert(result.end(), render.indices.begin(), render.indices.end());
	}
	return result;
}
