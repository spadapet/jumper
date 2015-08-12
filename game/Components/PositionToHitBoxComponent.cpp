#include "pch.h"
#include "Components/PositionToHitBoxComponent.h"
#include "Core/Globals.h"

PositionToHitBoxComponent::PositionToHitBoxComponent()
{
	_entityChangeEvent = EntityEvents::ID_POSITION_TO_HIT_BOX_CHANGED;
}
