#pragma once

namespace LogicHelpers
{
	ff::FixedInt AdjustTowardsLimit(ff::FixedInt value, ff::FixedInt limit, ff::FixedInt adjustAmount);
	ff::FixedInt AdjustTowardsValue(ff::FixedInt value, ff::FixedInt destValue, ff::FixedInt adjustAmount);
	ff::FixedInt Clamp(ff::FixedInt value, ff::FixedInt minValue, ff::FixedInt maxValue);
	ff::PointFixedInt AdjustTowardsLimit(ff::PointFixedInt value, ff::PointFixedInt limit, ff::FixedInt adjustAmount);
	ff::PointFixedInt AdjustTowardsValue(ff::PointFixedInt value, ff::PointFixedInt destValue, ff::FixedInt adjustAmount);
	ff::PointFixedInt Clamp(ff::PointFixedInt value, ff::RectFixedInt limits);
}
