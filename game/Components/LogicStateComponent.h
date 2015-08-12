#pragma once

#include "Core/Globals.h"

struct LogicStateComponent : public ff::Component
{
	LogicStateComponent();

	static const LogicStateComponent& Null();
	static const LogicStateComponent& SafeGet(const LogicStateComponent* obj);

	void Init(Advancing::Type type);
	void SetState(ff::Entity entity, Advancing::State state);
	void AddState(ff::Entity entity, Advancing::State state);
	void RemoveState(ff::Entity entity, Advancing::State state);

	bool HasAll(Advancing::State state) const;
	bool IsDeleting() const;

	Advancing::Type _type;
	Advancing::State _state;
	size_t _stateCounter;
};
