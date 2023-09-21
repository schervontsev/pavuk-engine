#include "Game.h"

#include "MaterialManager.h"
#include "ECS/ECSManager.h"
#include "ECS/Components/RenderComponent.h"
#include "ECS/Components/TransformComponent.h"

Game::Game()
{
    renderer = std::make_unique<Renderer>();
}

void Game::run()
{
    MaterialManager::Instance()->LoadMaterials();
    InitECS();
    scene = std::make_shared<Scene>();
    scene->Init();
    renderer->SetScene(scene);
    renderer->SetRenderSystem(renderSystem);
    renderer->init();
    renderer->UpdateBuffers();
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
        
        renderSystem->UpdateTransform(dt);

        renderer->Update(dt);
        renderer->UpdateBuffers();
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
    loadMeshSystem = ecsManager.RegisterSystem<LoadMeshSystem>();

    Signature renderSignature;
    renderSignature.set(ecsManager.GetComponentType<RenderComponent>());
    renderSignature.set(ecsManager.GetComponentType<TransformComponent>());
    ecsManager.SetSystemSignature<RenderSystem>(renderSignature);

    Signature loadMeshSignature;
    loadMeshSignature.set(ecsManager.GetComponentType<RenderComponent>());
    ecsManager.SetSystemSignature<LoadMeshSystem>(loadMeshSignature);
}
