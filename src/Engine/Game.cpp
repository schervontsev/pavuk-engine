#include "Game.h"

Game::Game()
{
    renderer = std::make_unique<Renderer>();
}

void Game::run()
{
    scene = std::make_shared<Scene>();
    renderer->SetScene(scene);
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
    glm::mat4 sceneMatrix = glm::mat4(1.f);
    while (!renderer->WindowShouldClose()) {
        glfwPollEvents();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        startTime = currentTime;

        scene->Update(dt);
        //scene->UpdateTransform(sceneMatrix);
        renderer->Update(dt);
        renderer->drawFrame();
    }

    renderer->WaitDevice();
}