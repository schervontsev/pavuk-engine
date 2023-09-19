#pragma once
#include "System.h"

struct Vertex;

class RenderSystem : public System
{
public:
	void Update(float dt);

    std::vector<Vertex> GetVertices();

	std::vector<uint32_t> GetIndices();

};

