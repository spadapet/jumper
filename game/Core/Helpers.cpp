#include "pch.h"
#include "Core/Globals.h"
#include "Core/Helpers.h"

void Helpers::RequestDeleteEntity(ff::Entity entity)
{
	if (entity->IsActive())
	{
		entity->TriggerEvent(EntityEvents::ID_DELETE_ENTITY);
	}
	else
	{
		entity->Delete();
	}
}
