#pragma once
#include "System.h"
struct RenderComponent;

class LoadMeshSystem : public System
{
public:
	void Load();
	void LoadModel(RenderComponent& render, int& indexCount, int& textureCount);
};

