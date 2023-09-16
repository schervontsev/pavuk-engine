#include "World.h"
#include "Mesh.h"

World::World()
{
	Mesh room;
	room.loadModel(0, "models/viking_room.obj", "models/", "textures/viking_room.png");
	//room.scale = glm::vec3(0.1f);
	models.push_back(room);

	Mesh elf;
	elf.loadModel(room.materials.size(), "models/elf/Elf01_posed.obj", "models/elf/", "textures/elf/");
	elf.scale = glm::vec3(0.01f);
	elf.SetEulerAngle(glm::vec3(glm::radians(90.f), 0.0, 0.0));
	models.push_back(elf);
}

void World::Update(float dt)
{
	if (models.empty()) {
		return;
	}
	models[1].AddEulerAngle(glm::vec3(0.f, 0.f, dt * 10.0));
	//models[1].translation.x += dt * 10.f;
	for (auto& model : models) {
		model.UpdatePushConstants();
	}
}
