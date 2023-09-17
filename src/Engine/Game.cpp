#include "Game.h"
#include "ECS/ECSManager.h"
#include "ECS/Components/RenderComponent.h"
#include "ECS/Components/TransformComponent.h"

Game::Game()
{
    renderer = std::make_unique<Renderer>();
}

void Game::run()
{
    InitECS();
    scene = std::make_shared<Scene>();
    scene->Init();
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

        scene->Update(dt); //TODO: will be moved to ecs?

        updateTransformSystem->Update(dt);
        renderSystem->Update(dt);

        renderer->Update(dt);
        renderer->drawFrame();
    }

    renderer->WaitDevice();
}

void Game::InitECS()
{
    ecsManager.Init();

    ecsManager.RegisterComponent<RenderComponent>();
    ecsManager.RegisterComponent<TransformComponent>();

    renderSystem = ecsManager.RegisterSystem<RenderSystem>();
    updateTransformSystem = ecsManager.RegisterSystem<UpdateTransformSystem>();

    Signature renderSignature;
    renderSignature.set(ecsManager.GetComponentType<RenderComponent>());
    renderSignature.set(ecsManager.GetComponentType<TransformComponent>());
    ecsManager.SetSystemSignature<RenderSystem>(renderSignature);

    Signature transformSignature;
    ecsManager.SetSystemSignature<UpdateTransformSystem>(transformSignature);
}
