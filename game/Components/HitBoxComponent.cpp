#include "pch.h"
#include "Components/HitBoxComponent.h"
#include "Core/Globals.h"

HitBoxComponent::HitBoxComponent()
{
	_entityChangeEvent = EntityEvents::ID_HIT_BOX_CHANGED;
}
