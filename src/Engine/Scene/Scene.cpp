#include "Scene.h"
#include "../ECS/ECSManager.h"
#include "../ECS/Components/TransformComponent.h"
#include "../ECS/Components/RenderComponent.h"
#include "../ECS/Components/RotateComponent.h"
#include "../ECS/Components/GirlComponent.h"
#include "../ECS/Components/PointLightComponent.h"
#include "../ECS/Components/CameraComponent.h"
#include "../ECS/Components/InputComponent.h"

Scene::Scene()
{
}

void Scene::Init()
{
	//Game code goes here.
	//Following is just placeholder code for testing purposes
	if (false) {
		auto room = ecsManager.CreateEntity();
		RenderComponent render;
		render.meshName = "viking_room";
		ecsManager.AddComponent(room, render);
		TransformComponent transform;
		//transform.translation.y = 1.f;
		transform.SetEulerAngle(glm::vec3{ glm::radians(-90.f), 0.f, 0.f });
		ecsManager.AddComponent(room, transform);
	}
	const int birdNum = 256;
	for (int i = 0; i < birdNum; i++) {
		auto bird = ecsManager.CreateEntity();
		RenderComponent render;
		render.meshName = "bird";
		ecsManager.AddComponent(bird, render);
		TransformComponent transform;
		transform.translation = { std::rand() / float(RAND_MAX / 4) - 2, std::rand() / float(RAND_MAX / 0.5), std::rand() / float(RAND_MAX / 4) - 4};
		transform.SetEulerAngle(glm::vec3{ 0.f, std::rand() / float(RAND_MAX / 1.6), 0.f });
		ecsManager.AddComponent(bird, transform);
	}
	if (true) {
		auto levelMesh = ecsManager.CreateEntity();
		RenderComponent render;
		render.meshName = "maphome";
		ecsManager.AddComponent(levelMesh, render);
		TransformComponent transform;
		transform.scale = { 0.1f, 0.1f, 0.1f };
		ecsManager.AddComponent(levelMesh, transform);
	}

	//lights

	if (true) {
		auto light = ecsManager.CreateEntity();
		ecsManager.AddComponent(light, PointLightComponent({ glm::vec4(0.0, 0.0, 1.0, 1.0) }));
		TransformComponent transform;
		transform.translation = glm::vec3(10.0, -0.2, 0.0);
		ecsManager.AddComponent(light, transform);
	}
	if (true) {
		auto light = ecsManager.CreateEntity();
		ecsManager.AddComponent(light, PointLightComponent({ glm::vec4(1.0, 0.5, 0.0, 1.0) }));
		TransformComponent transform;
		transform.translation = glm::vec3(-10.0, -0.2, 0.0);
		ecsManager.AddComponent(light, transform);
	}
	if (true) {
		auto light1 = ecsManager.CreateEntity();
		ecsManager.AddComponent(light1, PointLightComponent({ glm::vec4(0.0, 0.2, 0.0, 1.0) }));
		TransformComponent transform;
		transform.translation = glm::vec3(0.0, -0.2, 2.0);
		ecsManager.AddComponent(light1, transform);
	}
	for (int i = 0; i < 12; i++) {
		auto light1 = ecsManager.CreateEntity();
		ecsManager.AddComponent(light1, PointLightComponent({ glm::vec4(0.2, 0.2, 0.2, 1.0) }));
		TransformComponent transform;
		transform.translation = { std::rand() % 100 - 50, std::rand() % 100 - 50, std::rand() % 100 - 50 };
		ecsManager.AddComponent(light1, transform);
	}

	const int girlNum = 1;
	for (int i = 0; i < girlNum; i++) {
		auto elf = ecsManager.CreateEntity();
		RenderComponent render1;
		render1.meshName = "elf";
		render1.instances = 1;
		ecsManager.AddComponent(elf, render1);
		TransformComponent transform;
		transform.scale = { 0.01f, 0.01f, 0.01f };
		transform.translation = { 0.f, 0.f, -2.f };
		//ecsManager.AddComponent(elf, InputComponent{});
		//transform.translation = { std::rand() % 100 - 50, std::rand() % 100 - 50, std::rand() % 100 - 50 };
		//transform.SetEulerAngle(glm::vec3{ glm::radians(90.f), 0.f, 0.f });
		ecsManager.AddComponent(elf, transform);
		//ecsManager.AddComponent(elf, RotateComponent{ glm::vec3(0.f, 1.f, 0.f), -2.f });
		//ecsManager.AddComponent(elf, GirlComponent{});
	}

	InitCamera();
}

void Scene::InitCamera() {
	mainCamera = ecsManager.CreateEntity();
	ecsManager.AddComponent(mainCamera, CameraComponent{glm::radians(45.f)});
	ecsManager.AddComponent(mainCamera, TransformComponent{glm::vec3(0.f,-0.01f,0.f)});
	ecsManager.AddComponent(mainCamera, InputComponent{});

	//TODO: test
	//ecsManager.AddComponent(mainCamera, RotateComponent{ glm::vec3(0.f, 1.0f, 0.f), 2.f });
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
