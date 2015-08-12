#pragma once

#include "Core/Globals.h"

struct RenderStateComponent : public ff::Component
{
	RenderStateComponent();

	void Init(Rendering::Type renderType);

	Rendering::Type _renderType;
	size_t _counter;
};
