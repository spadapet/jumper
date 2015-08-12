#pragma once

struct VelocityComponent : public ff::Component
{
	VelocityComponent();

	ff::PointFixedInt _posVelocity;
	ff::FixedInt _rotateVelocity; // degrees
};
