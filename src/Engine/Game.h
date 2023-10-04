#pragma once
#include "Render/Renderer.h"
#include "Scene/Scene.h"
//systems
#include "ECS/Systems/CameraControllerSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/UpdateTransformSystem.h"
#include "ECS/Systems/TestSystem.h"
#include "ECS/Systems/RotateSystem.h"
#include "ECS/Systems/SetInputSystem.h"

class Game
{
public:
	Game();
	void run();
	void cleanup();
	void mainLoop();
	void InitECS();
private:
	std::unique_ptr<Renderer> renderer;
	std::shared_ptr<Scene> scene;

	//systems
	std::shared_ptr<RenderSystem> renderSystem;
	std::shared_ptr<UpdateTransformSystem> updateTransformSystem;
	std::shared_ptr<RotateSystem> rotateSystem;
	std::shared_ptr<TestSystem> testSystem;
	std::shared_ptr<SetInputSystem> setInputSystem;
	std::shared_ptr<CameraControllerSystem> setInputSystem;
};

