#include "pch.h"
#include "Components/EntityTypeComponent.h"
#include "Core/Globals.h"
#include "Core/Helpers.h"

using EntityTypeI = std::underlying_type<EntityType>::type;

EntityTypeComponent::EntityTypeComponent()
	: _type(EntityType::None)
{
}

void EntityTypeComponent::Set(ff::Entity entity, EntityType type)
{
	if (type != _type)
	{
		EntityEvents::TypeChangedEventArgs args;
		args._oldType = _type;
		args._oldType = type;

		_type = type;

		if (entity != ff::INVALID_ENTITY)
		{
			entity->TriggerEvent(EntityEvents::ID_TYPE_CHANGED, &args);
		}
	}
}

bool EntityTypeComponent::HasAnyType(EntityType type) const
{
	return Helpers::HasAnyFlags(_type, type);
}

bool EntityTypeComponent::HasAllTypes(EntityType type) const
{
	return Helpers::HasAllFlags(_type, type);
}
