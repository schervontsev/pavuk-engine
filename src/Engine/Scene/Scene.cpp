#include "Scene.h"
#include "../ECS/ECSManager.h"
#include "../ECS/Components/TransformComponent.h"
#include "../ECS/Components/RenderComponent.h"
#include "../ECS/Components/RotateComponent.h"
#include "../ECS/Components/GirlComponent.h"
#include "../ECS/Components/CameraComponent.h"

Scene::Scene()
{
}

void Scene::Init()
{
	auto room = ecsManager.CreateEntity();
	RenderComponent render;
	render.meshName = "viking_room";
	ecsManager.AddComponent(room, render);
	ecsManager.AddComponent(room, TransformComponent{});
	
	const int girlNum = 1;
	for (int i = 0; i < girlNum; i++) {
		auto elf = ecsManager.CreateEntity();
		RenderComponent render1;
		render1.meshName = "elf";
		render1.instances = 1;
		ecsManager.AddComponent(elf, render1);
		TransformComponent transform;
		transform.scale = { 0.01f, 0.01f, 0.01f };
		transform.translation = { std::rand() % 100 - 50, std::rand() % 100 - 50, std::rand() % 100 - 50 };
		transform.SetEulerAngle(glm::vec3{ glm::radians(90.f), 0.f, 0.f });
		ecsManager.AddComponent(elf, transform);
		ecsManager.AddComponent(elf, RotateComponent{ glm::vec3(0.f, 1.f, 0.f), 2.f });
		ecsManager.AddComponent(elf, GirlComponent{});
	}

	InitCamera();
}

void Scene::InitCamera() {
	mainCamera = ecsManager.CreateEntity();
	ecsManager.AddComponent(mainCamera, CameraComponent{glm::radians(45.f)});
	ecsManager.AddComponent(mainCamera, TransformComponent{});

	//TODO: test
	ecsManager.AddComponent(mainCamera, RotateComponent{ glm::vec3(0.f, 0.f, 1.f), 2.f });
}

void Scene::Update(float dt)
{

	/*{
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
	}*/
}
