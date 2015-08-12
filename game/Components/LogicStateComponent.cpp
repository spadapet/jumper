#include "pch.h"
#include "Components/LogicStateComponent.h"
#include "Core/Helpers.h"

LogicStateComponent::LogicStateComponent()
	: _type(Advancing::Type::None)
	, _state(Advancing::State::None)
	, _stateCounter(0)
{
}

const LogicStateComponent& LogicStateComponent::Null()
{
	static LogicStateComponent value;
	return value;
}

const LogicStateComponent& LogicStateComponent::SafeGet(const LogicStateComponent* obj)
{
	return obj ? *obj : LogicStateComponent::Null();
}

void LogicStateComponent::Init(Advancing::Type type)
{
	_type = type;
	_state = Advancing::State::None;
	_stateCounter = 0;
}

void LogicStateComponent::SetState(ff::Entity entity, Advancing::State state)
{
	if (state != _state)
	{
		EntityEvents::StateChangedEventArgs args;
		args._oldState = _state;
		args._oldState = state;

		_state = state;
		_stateCounter = 0;

		if (entity != ff::INVALID_ENTITY)
		{
			entity->TriggerEvent(EntityEvents::ID_STATE_CHANGED, &args);
		}
	}
}

void LogicStateComponent::AddState(ff::Entity entity, Advancing::State state)
{
	SetState(entity, Helpers::CombineFlags(_state, state));
}

void LogicStateComponent::RemoveState(ff::Entity entity, Advancing::State state)
{
	SetState(entity, Helpers::ClearFlags(_state, state));
}

bool LogicStateComponent::HasAll(Advancing::State state) const
{
	return Helpers::HasAllFlags(_state, state);
}

bool LogicStateComponent::IsDeleting() const
{
	return Helpers::HasAllFlags(_state, Advancing::State::Deleting);
}
