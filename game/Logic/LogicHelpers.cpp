#include "pch.h"
#include "Components/HitBoxComponent.h"
#include "Core/Helpers.h"
#include "Core/TileSet.h"
#include "Logic/LogicHelpers.h"
#include "Services/CollisionService.h"

ff::FixedInt LogicHelpers::AdjustTowardsLimit(ff::FixedInt value, ff::FixedInt limit, ff::FixedInt adjustAmount)
{
	assert(limit >= 0_f && adjustAmount >= 0_f);

	if (value > limit)
	{
		value = std::max(value - adjustAmount, limit);
	}
	else if (value < -limit)
	{
		value = std::min(value + adjustAmount, -limit);
	}

	return value;
}

ff::FixedInt LogicHelpers::AdjustTowardsValue(ff::FixedInt value, ff::FixedInt destValue, ff::FixedInt adjustAmount)
{
	assert(adjustAmount >= 0_f);

	if (value > destValue)
	{
		value = std::max(value - adjustAmount, destValue);
	}
	else if (value < destValue)
	{
		value = std::min(value + adjustAmount, destValue);
	}

	return value;
}

ff::FixedInt LogicHelpers::Clamp(ff::FixedInt value, ff::FixedInt minValue, ff::FixedInt maxValue)
{
	value = std::max(value, minValue);
	value = std::min(value, maxValue);
	return value;
}

ff::PointFixedInt LogicHelpers::AdjustTowardsLimit(ff::PointFixedInt value, ff::PointFixedInt limit, ff::FixedInt adjustAmount)
{
	return ff::PointFixedInt(
		LogicHelpers::AdjustTowardsLimit(value.x, limit.x, adjustAmount),
		LogicHelpers::AdjustTowardsLimit(value.y, limit.y, adjustAmount));
}

ff::PointFixedInt LogicHelpers::AdjustTowardsValue(ff::PointFixedInt value, ff::PointFixedInt destValue, ff::FixedInt adjustAmount)
{
	return ff::PointFixedInt(
		LogicHelpers::AdjustTowardsValue(value.x, destValue.x, adjustAmount),
		LogicHelpers::AdjustTowardsValue(value.y, destValue.y, adjustAmount));
}

ff::PointFixedInt LogicHelpers::Clamp(ff::PointFixedInt value, ff::RectFixedInt limits)
{
	return ff::PointFixedInt(
		LogicHelpers::Clamp(value.x, limits.left, limits.right),
		LogicHelpers::Clamp(value.y, limits.top, limits.bottom));
}
