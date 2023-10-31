#pragma once
#include <mutex>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>


struct Vertex;

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	std::vector<uint32_t> materials;
};

class MeshManager
{
private:
	static MeshManager* _instance;
	static std::mutex _mutex;
protected:
	MeshManager() = default;
public:
	MeshManager(MeshManager& other) = delete;
	void operator=(const MeshManager&) = delete;

	static MeshManager* Instance();

	void LoadMeshes();
	void LoadModel(Mesh& mesh, const std::string& path, const std::vector<uint32_t>& materials);

	Mesh* GetMesh(uint32_t handle) { return meshes[meshesByHandle[handle]].get(); }
	uint32_t GetMeshHandle(const std::string& id) { return meshHandlesById[id]; }
private:
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::unordered_map<uint32_t, size_t> meshesByHandle;
	std::unordered_map<std::string, uint32_t> meshHandlesById;

	uint32_t nextId;
};

