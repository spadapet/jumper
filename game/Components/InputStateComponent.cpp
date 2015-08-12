#include "pch.h"
#include "Components/InputStateComponent.h"

InputStateComponent::InputStateComponent()
{
	Reset();
}

void InputStateComponent::Reset()
{
	ff::ZeroObject(*this);
}

const InputStateComponent& InputStateComponent::Null()
{
	static InputStateComponent nullState;
	return nullState;
}
