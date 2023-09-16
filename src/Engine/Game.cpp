#include "Game.h"

Game::Game()
{
    renderer = std::make_unique<Renderer>();
}

void Game::run()
{
    world = std::make_unique<World>();
    renderer->prepareWorld(world.get());
    renderer->init();
    mainLoop();
    cleanup();
}

void Game::cleanup()
{
    renderer->cleanup();
}

void Game::mainLoop() {
    auto startTime = std::chrono::high_resolution_clock::now();
    glm::mat4 worldMatrix = glm::mat4(1.f);
    while (!renderer->WindowShouldClose()) {
        glfwPollEvents();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        startTime = currentTime;

        world->Update(dt);
        //world->UpdateTransform(worldMatrix);
        renderer->Update(dt);
        renderer->drawFrame();
    }

    renderer->WaitDevice();
}