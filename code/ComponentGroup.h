#pragma once

#include "Entity.h"

template <class T>
struct ComponentGroup
{
	Entity entity = 0;

	T* components;
	uint32 numComponents;
};
