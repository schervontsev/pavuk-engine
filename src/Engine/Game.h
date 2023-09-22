#pragma once
#include "Render/Renderer.h"
#include "Scene/Scene.h"
//systems
#include "ECS/Systems/RenderSystem.h"

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
};

