#include "Game.h"

#include "ECS/ECSManager.h"
#include "ECS/Components/RenderComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/GirlComponent.h"
#include "ECS/Components/RotateComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/InputComponent.h"
#include "ECS/Components/PointLightComponent.h"
#include "ResourceManagers/MaterialManager.h"
#include "ResourceManagers/MeshManager.h"
#include "Input/InputManager.h"

Game::Game()
{
    renderer = std::make_unique<Renderer>();
}

void Game::run()
{
    InitECS();

    MaterialManager::Instance()->LoadMaterials();
    MeshManager::Instance()->LoadMeshes();
    
    scene = std::make_shared<Scene>();
    scene->Init();

    renderSystem->UpdateMeshHandles();

    renderer->SetScene(scene);
    renderer->SetRenderSystem(renderSystem);
    renderer->SetUpdateLightSystem(updateLightSystem);

	InputManager::Instance()->Init(renderer->initWindow());

	renderer->initVulkan();
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

        setInputSystem->SetInput();

        simpleControllerSystem->Update(dt);

        //TODO: some testing
        rotateSystem->Update(dt);
        testSystem->Update(dt, scene.get());

        updateTransformSystem->UpdateTransform();
        renderSystem->UpdateTransform();

        renderer->Update(dt);
        renderer->UpdateBuffers();
        renderer->drawFrame();
    }

    renderer->WaitDevice();
}

void Game::InitECS()
{
    ecsManager.Init();

    //TODO: use preprocessor to generate or something
    ecsManager.RegisterComponent<RenderComponent>();
    ecsManager.RegisterComponent<TransformComponent>();
    ecsManager.RegisterComponent<GirlComponent>();
    ecsManager.RegisterComponent<RotateComponent>();
    ecsManager.RegisterComponent<CameraComponent>();
    ecsManager.RegisterComponent<InputComponent>();
    ecsManager.RegisterComponent<PointLightComponent>();

    renderSystem = ecsManager.RegisterSystem<RenderSystem>();
    updateLightSystem = ecsManager.RegisterSystem<UpdateLightSystem>();
    updateTransformSystem = ecsManager.RegisterSystem<UpdateTransformSystem>();
    rotateSystem = ecsManager.RegisterSystem<RotateSystem>();
    testSystem = ecsManager.RegisterSystem<TestSystem>();
    setInputSystem = ecsManager.RegisterSystem<SetInputSystem>();
    simpleControllerSystem = ecsManager.RegisterSystem<SimpleControllerSystem>();

    Signature renderSignature;
    renderSignature.set(ecsManager.GetComponentType<RenderComponent>());
    renderSignature.set(ecsManager.GetComponentType<TransformComponent>());
    ecsManager.SetSystemSignature<RenderSystem>(renderSignature);

    Signature updateLightSignature;
    updateLightSignature.set(ecsManager.GetComponentType<PointLightComponent>());
    updateLightSignature.set(ecsManager.GetComponentType<TransformComponent>());
    ecsManager.SetSystemSignature<UpdateLightSystem>(updateLightSignature);

    Signature transformSignature;
    transformSignature.set(ecsManager.GetComponentType<TransformComponent>());
    ecsManager.SetSystemSignature<UpdateTransformSystem>(transformSignature);

    Signature rotateSignature;
    rotateSignature.set(ecsManager.GetComponentType<TransformComponent>());
    rotateSignature.set(ecsManager.GetComponentType<RotateComponent>());
    ecsManager.SetSystemSignature<RotateSystem>(rotateSignature);

    Signature testSignature;
    testSignature.set(ecsManager.GetComponentType<GirlComponent>());
    ecsManager.SetSystemSignature<TestSystem>(testSignature);

    Signature inputSignature;
    inputSignature.set(ecsManager.GetComponentType<InputComponent>());
    ecsManager.SetSystemSignature<SetInputSystem>(inputSignature);

    Signature cameraControllerSignature;
    cameraControllerSignature.set(ecsManager.GetComponentType<InputComponent>());
    cameraControllerSignature.set(ecsManager.GetComponentType<TransformComponent>());
    ecsManager.SetSystemSignature<SimpleControllerSystem>(cameraControllerSignature);
}
