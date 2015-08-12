#pragma once

struct InputForcesComponent : public ff::Component
{
	InputForcesComponent();

	void InitForPlayer(ff::FixedInt maxVelocity, ff::FixedInt friction);

	ff::PointFixedInt GetPressDir(ff::PointFixedInt pressDir) const;
	ff::FixedInt GetMoveForce(bool standing) const;
	ff::FixedInt GetFriction(bool standing, bool rampUp, bool rampDown) const;
	ff::PointFixedInt GetJumpVelocity(bool running, size_t jumpFrames, ff::FixedInt gravity) const;
	size_t GetJumpBoostFrames(bool running) const;
	ff::PointFixedInt GetMaxVelocity(bool running, bool rampUp, bool rampDown) const;
	ff::PointFixedInt GetGravity(ff::PointFixedInt worldGravity) const;

	// Data

	ff::PointFixedInt _pressScale;

	ff::FixedInt _groundMoveForce;
	ff::FixedInt _groundFriction;

	ff::FixedInt _jumpMoveForce;
	ff::FixedInt _jumpFriction;
	ff::FixedInt _jumpVelocity;
	size_t _jumpBoostFrames;
	size_t _jumpRunningBoostFrames;

	ff::FixedInt _maxWalkVelocity;
	ff::FixedInt _maxRunVelocity;
	ff::FixedInt _maxFallVelocity;
	ff::FixedInt _gravityScale;
};

struct InputForcesRefComponent : public ff::Component
{
	InputForcesRefComponent();

	InputForcesComponent* _forces;
};
