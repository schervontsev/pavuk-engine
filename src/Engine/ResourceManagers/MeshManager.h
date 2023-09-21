#pragma once
#include <mutex>

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
};

