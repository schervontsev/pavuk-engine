#include "Scene.h"
#include "Mesh.h"
#include "../ECS/ECSManager.h"
#include "../ECS/Components/TransformComponent.h"
#include "../ECS/Components/RenderComponent.h"

Scene::Scene()
{
	Mesh room;
	room.loadModel(0, "models/viking_room.obj", "models/", "textures/viking_room.png");
	models.push_back(room);
	ecsManager.CreateEntity();

	Mesh elf;
	elf.loadModel(room.materials.size(), "models/elf/Elf01_posed.obj", "models/elf/", "textures/elf/");
	elf.scale = glm::vec3(0.01f);
	elf.SetEulerAngle(glm::vec3(glm::radians(90.f), 0.0, 0.0));
	models.push_back(elf);
}

void Scene::Init()
{
	auto entity = ecsManager.CreateEntity();
	ecsManager.AddComponent(entity, RenderComponent{});
	ecsManager.AddComponent(entity, TransformComponent{});
}

void Scene::Update(float dt)
{

	{
		//TODO: Debug logic
		timeFromStart += dt;
		if (models.empty()) {
			return;
		}
		models.back().AddEulerAngle(glm::vec3(0.f, 0.f, dt * 10.0));
		if (timeFromStart > 4.f && timeFromStart < 10.f && models.size() == 2) {
			models.erase(models.begin(), models.begin() + 1);
			SetDirty(true);
		}
		else if (models.size() == 1 && timeFromStart > 10.f) {
			Mesh room;
			room.loadModel(0, "models/viking_room.obj", "models/", "textures/viking_room.png");
			models.push_back(room);
			SetDirty(true);
		}
	}
	for (auto& model : models) {
		model.UpdatePushConstants();
	}
}
