#pragma once
#include <cstdint>
#include <array>
enum class ComponentType { 
	Transform,
	Child,
	Mesh
};

template<typename T, auto Type>
class Component
{
public:
	static constexpr auto type = static_cast<std::size_t>(Type);

	virtual ComponentType GetType() = 0;
	uint32_t entity;
};