#include "pch.h"
#include "Components/RenderStateComponent.h"

RenderStateComponent::RenderStateComponent()
	: _renderType(Rendering::Type::None)
	, _counter(0)
{
}

void RenderStateComponent::Init(Rendering::Type renderType)
{
	if (_renderType != renderType)
	{
		_renderType = renderType;
		_counter = 0;
	}
}
