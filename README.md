# Pavuk Engine
Prototype ECS-based engine with rendering done in Vulkan.

## Features
Keep in mind, it's just a prototype, mostly for my own amusment and learning about graphic pipelines.
 - Rendering multiple meshes with multiple textures
 - Diffuse lighting with multiple colored point light sources
 - All the meshes and textures can be set and configured in json files in resources/data folder.
 - Game logic goes into Scene.cpp for now.
 - Game architecture is based about components and systems. Look for examples in Scene.cpp. Systems are set inside Game.cpp for now.

## Prerequisites
 - Visual studio. CMake is in future plans.
 - [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/). Project uses environment variable VULKAN_SDK
 - GLFW. Provided as a submodule.

## Plans for the near future
 - Shadowmapping
 - Converting project to static library or otherwise separating engine code from the game.
 - Refactoring Renderer class according to OOP principles.
 - CMake integration
