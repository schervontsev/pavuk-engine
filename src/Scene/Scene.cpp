#include "Scene.h"
#include "../Engine/ECS/ECSManager.h"
#include "../Engine/ECS/Components/TransformComponent.h"
#include "../Engine/ECS/Components/RenderComponent.h"
#include "../Engine/ECS/Components/RotateComponent.h"
#include "../Engine/ECS/Components/GirlComponent.h"
#include "../Engine/ECS/Components/PointLightComponent.h"
#include "../Engine/ECS/Components/ShadowComponent.h"
#include "../Engine/ECS/Components/CameraComponent.h"
#include "../Engine/ECS/Components/InputComponent.h"

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
	const int birdNum = 1;
	for (int i = 0; i < birdNum; i++) {
		auto bird = ecsManager.CreateEntity();
		RenderComponent render;
		render.meshName = "bird";
		ecsManager.AddComponent(bird, render);
		TransformComponent transform;
		transform.translation = { std::rand() / float(RAND_MAX / 4) - 2, std::rand() / float(RAND_MAX / 0.5), std::rand() / float(RAND_MAX / 4) - 4};
		//transform.translation = { 0.15f, 0.4f, -2.0f };
		transform.SetEulerAngle(glm::vec3{ 0.f, std::rand() / float(RAND_MAX / 1.6), 0.f });
		ecsManager.AddComponent(bird, transform);
	}
	if (true) {
		auto levelMesh = ecsManager.CreateEntity();
		RenderComponent render;
		render.meshName = "mine";
		ecsManager.AddComponent(levelMesh, render);
		TransformComponent transform;
		transform.scale = { 0.01f, 0.01f, 0.01f };
		transform.translation = glm::vec3(0.0, 0.0, -3.0f);
		ecsManager.AddComponent(levelMesh, transform);
		//ecsManager.AddComponent(levelMesh, RotateComponent{ glm::vec3(0.f, 1.f, 0.f), 0.5f });
	}

	// Camera (WASD, arrows, Q/E) — separate from light
	if (true) {
		auto camera = ecsManager.CreateEntity();
		TransformComponent camTransform;
		camTransform.translation = { 0.f, 0.f, 0.f };
		ecsManager.AddComponent(camera, camTransform);
		ecsManager.AddComponent(camera, CameraComponent{ glm::radians(45.f) });
		ecsManager.AddComponent(camera, InputComponent{});
		mainCamera = camera;
	}

	// Point light — move in world space with I K J L U O (see InputManager)
	if (true) {
		auto pointLight = ecsManager.CreateEntity();
		ecsManager.AddComponent(pointLight, PointLightComponent({ glm::vec4(1.0, 1.0, 1.0, 1.0) }));
		ecsManager.AddComponent(pointLight, ShadowComponent{});
		TransformComponent lightTransform;
		lightTransform.translation = glm::vec3(0.5f, 0.5f, 1.0f);
		ecsManager.AddComponent(pointLight, lightTransform);
	}
	for (int i = 0; i < 0; i++) {
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
		ecsManager.AddComponent(elf, RotateComponent{ glm::vec3(0.f, 1.f, 0.f), 1.f });
		//ecsManager.AddComponent(elf, GirlComponent{});
	}

	//InitCamera();
}

void Scene::InitCamera() {
	mainCamera = ecsManager.CreateEntity();
	ecsManager.AddComponent(mainCamera, CameraComponent{glm::radians(45.f)});
	ecsManager.AddComponent(mainCamera, TransformComponent{glm::vec3(0.f,-0.01f,0.f)});
	ecsManager.AddComponent(mainCamera, InputComponent{});
}

void Scene::Update(float dt)
{

}
