#pragma once
#include "Render/Renderer.h"
#include "Scene/Scene.h"
class Game
{
public:
	Game();
	void run();
	void cleanup();
	void mainLoop();
private:
	std::unique_ptr<Renderer> renderer;
	std::shared_ptr<Scene> scene;
};

