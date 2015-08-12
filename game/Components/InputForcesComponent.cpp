#include "pch.h"
#include "Components/InputForcesComponent.h"

InputForcesComponent::InputForcesComponent()
{
	ff::ZeroObject(*this);
}

void InputForcesComponent::InitForPlayer(ff::FixedInt maxVelocity, ff::FixedInt friction)
{
	_gravityScale = 1_f;
	_groundFriction = friction;
	_groundMoveForce = friction;
	_jumpFriction = friction / 2;
	_jumpMoveForce = friction / 2;
	_jumpVelocity = maxVelocity;
	_jumpBoostFrames = 16;
	_jumpRunningBoostFrames = 24;
	_maxFallVelocity = maxVelocity;
	_maxRunVelocity = maxVelocity;
	_maxWalkVelocity = maxVelocity * 2 / 3;
	_pressScale = ff::PointFixedInt(1_f, 0_f);
}

ff::PointFixedInt InputForcesComponent::GetPressDir(ff::PointFixedInt pressDir) const
{
	// Dead zone for user input

	if (std::abs(pressDir.x) < 0.5_f)
	{
		pressDir.x = 0;
	}

	if (std::abs(pressDir.y) < 0.5_f)
	{
		pressDir.y = 0;
	}

	return pressDir * _pressScale;
}

ff::FixedInt InputForcesComponent::GetMoveForce(bool standing) const
{
	return standing ? _groundMoveForce : _jumpMoveForce;
}

ff::FixedInt InputForcesComponent::GetFriction(bool standing, bool rampUp, bool rampDown) const
{
	if (standing)
	{
		if (rampUp)
		{
			return _groundFriction * 2;
		}

		if (rampDown)
		{
			return _groundFriction * 3 / 4;
		}

		return _groundFriction;
	}

	return _jumpFriction;
}

ff::PointFixedInt InputForcesComponent::GetJumpVelocity(bool running, size_t jumpFrames, ff::FixedInt gravity) const
{
	return jumpFrames < GetJumpBoostFrames(running)
		? ff::PointFixedInt(0_f, _jumpVelocity + gravity)
		: ff::PointFixedInt::Zeros();
}

size_t InputForcesComponent::GetJumpBoostFrames(bool running) const
{
	return running ? _jumpRunningBoostFrames : _jumpBoostFrames;
}

ff::PointFixedInt InputForcesComponent::GetMaxVelocity(bool running, bool rampUp, bool rampDown) const
{
	if (running || rampDown)
	{
		return ff::PointFixedInt(_maxRunVelocity, _maxFallVelocity);
	}

	if (rampUp)
	{
		return ff::PointFixedInt(_maxWalkVelocity / 2, _maxFallVelocity);
	}

	return ff::PointFixedInt(_maxWalkVelocity, _maxFallVelocity);
}

ff::PointFixedInt InputForcesComponent::GetGravity(ff::PointFixedInt worldGravity) const
{
	return worldGravity * _gravityScale;
}

InputForcesRefComponent::InputForcesRefComponent()
	: _forces(nullptr)
{
}
