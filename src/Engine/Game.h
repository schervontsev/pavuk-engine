#pragma once
#include "Render/Renderer.h"
#include "World/World.h"
class Game
{
public:
	Game();
	void run();
	void cleanup();
	void mainLoop();
private:
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<World> world;
};

