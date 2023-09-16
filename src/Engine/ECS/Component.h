#pragma once
#include <cstdint>
#include <array>
#include <bitset>
enum class ComponentType { 
	Transform,
	Child,
	Mesh
};


template<typename T, auto Type>
class Component
{
public:
	uint32_t entity;

};