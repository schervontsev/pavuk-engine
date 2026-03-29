cmake_minimum_required(VERSION 3.16)
project(Pavuk LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Sources (from Pavuk.vcxproj)
set(PAVUK_SOURCES
    src/main.cpp
    src/Scene/Scene.cpp
    src/Engine/Game.cpp
    src/Engine/Render/Renderer.cpp
    src/Engine/Render/Vertex.cpp
    src/Engine/ECS/ECSManager.cpp
    src/Engine/ECS/ComponentMap.cpp
    src/Engine/ECS/EntityManager.cpp
    src/Engine/ECS/ComponentManager.cpp
    src/Engine/ECS/SystemManager.cpp
    src/Engine/ECS/Systems/System.cpp
    src/Engine/ECS/Systems/RenderSystem.cpp
    src/Engine/ECS/Systems/UpdateTransformSystem.cpp
    src/Engine/ECS/Systems/UpdateVisibilitySystem.cpp
    src/Engine/ECS/Systems/UpdateLightSystem.cpp
    src/Engine/ECS/Systems/ShadowSystem.cpp
    src/Engine/ECS/Systems/RotateSystem.cpp
    src/Engine/ECS/Systems/SetInputSystem.cpp
    src/Engine/ECS/Systems/SimpleControllerSystem.cpp
    src/Engine/ECS/Systems/LightControllerSystem.cpp
    src/Engine/ECS/Systems/TestSystem.cpp
    src/Engine/ECS/Components/TransformComponent.cpp
    src/Engine/ResourceManagers/MeshManager.cpp
    src/Engine/ResourceManagers/MaterialManager.cpp
    src/Engine/Input/InputManager.cpp
)

# Vulkan
find_package(Vulkan REQUIRED)

# GLFW (submodule)
add_subdirectory(glfw)

# GLM: try find_package first, then FetchContent
find_package(glm QUIET)
if(NOT glm_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
    )
    set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLM_BUILD_INFO OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glm)
endif()

# Executable
add_executable(Pavuk ${PAVUK_SOURCES})

target_include_directories(Pavuk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Pavuk PRIVATE Vulkan::Vulkan glfw)

# GLM: find_package provides glm::glm, FetchContent may provide glm
if(TARGET glm::glm)
    target_link_libraries(Pavuk PRIVATE glm::glm)
elseif(TARGET glm)
    target_link_libraries(Pavuk PRIVATE glm)
else()
    message(FATAL_ERROR "GLM target not found")
endif()

target_compile_definitions(Pavuk PRIVATE GLM_ENABLE_EXPERIMENTAL)

if(WIN32)
    target_compile_definitions(Pavuk PRIVATE WIN32 _CONSOLE)
endif()
