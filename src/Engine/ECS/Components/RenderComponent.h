#pragma once
#include "../../Scene/Mesh.h"

struct RenderComponent {
	//TODO: move to resource manager
	std::string modelPath;
	std::string modelDir;
	std::string texturesDir;

	MeshPushConstants pushConstants;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	std::vector<uint32_t> materials;
};