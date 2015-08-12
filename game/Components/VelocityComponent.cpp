#include "pch.h"
#include "Components/VelocityComponent.h"

VelocityComponent::VelocityComponent()
	: _posVelocity(ff::PointFixedInt::Zeros())
	, _rotateVelocity(0)
{
}
