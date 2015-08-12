#pragma once

struct InputStateComponent : public ff::Component
{
	InputStateComponent();
	void Reset();

	static const InputStateComponent& Null();

	ff::PointFixedInt _pressDir;
	bool _pressingJump;
	bool _pressingRun;
	bool _pressedJump;
	bool _pressedRun;
};
