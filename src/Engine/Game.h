#pragma once
#include "Render/Renderer.h"
class Game
{
public:
	Game();
	void run();
	void cleanup();
	void mainLoop();
private:
	std::unique_ptr<Renderer> renderer;
};

