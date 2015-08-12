#pragma once

// Different areas of a level can have different camera behavior
struct CameraArea
{
	enum class Behavior
	{
		Normal,
	};

	ff::RectFixedInt _worldBounds;
	Behavior _behavior;
};
