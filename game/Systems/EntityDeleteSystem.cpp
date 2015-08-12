#include "pch.h"
#include "Components/LogicStateComponent.h"
#include "Core/Globals.h"
#include "Core/Helpers.h"
#include "Systems/EntityDeleteSystem.h"

EntityDeleteSystem::EntityDeleteSystem(ff::EntityDomain* domain)
	: _domain(domain)
{
	_domain->AddEventHandler(EntityEvents::ID_DELETE_ENTITY, this);
	_domain->AddEventHandler(EntityEvents::ID_STATE_CHANGED, this);
	_domain->AddEventHandler(ff::GetEntityEventDeleted(), this);

	_deletedEntities.SetBucketCount(Constants::REASONABLE_ENTITY_BUCKET_SIZE);
	_deletedEntitiesCopy.Reserve(Constants::REASONABLE_ENTITY_BUFFER_SIZE);
}

EntityDeleteSystem::~EntityDeleteSystem()
{
	_domain->RemoveEventHandler(EntityEvents::ID_DELETE_ENTITY, this);
	_domain->RemoveEventHandler(EntityEvents::ID_STATE_CHANGED, this);
	_domain->RemoveEventHandler(ff::GetEntityEventDeleted(), this);
}

void EntityDeleteSystem::Advance(ff::EntityDomain* domain)
{
	noAssertRet(_deletedEntities.Size());

	_deletedEntitiesCopy.Reserve(_deletedEntities.Size());

	for (ff::Entity entity : _deletedEntities)
	{
		_deletedEntitiesCopy.Push(entity);
	}

	_deletedEntities.Clear();

	for (ff::Entity entity : _deletedEntitiesCopy)
	{
		entity->Delete();
	}

	_deletedEntitiesCopy.Clear();
}

void EntityDeleteSystem::OnEntityDeleted(ff::Entity entity)
{
}

void EntityDeleteSystem::OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs)
{
	if (eventId == EntityEvents::ID_DELETE_ENTITY)
	{
		LogicStateComponent* logic = entity->GetComponent<LogicStateComponent>();
		if (logic)
		{
			logic->AddState(entity, Advancing::State::Deleting);
			assert(_deletedEntities.KeyExists(entity));
		}
		else
		{
			_deletedEntities.SetKey(entity);
		}
	}
	else if (eventId == EntityEvents::ID_STATE_CHANGED)
	{
		const EntityEvents::StateChangedEventArgs& stateChanged = *(const EntityEvents::StateChangedEventArgs*)eventArgs;
		bool newDelete = Helpers::HasAllFlags(stateChanged._newState, Advancing::State::Deleting);
		bool oldDelete = Helpers::HasAllFlags(stateChanged._oldState, Advancing::State::Deleting);

		if (newDelete && !oldDelete)
		{
			assert(!_deletedEntities.KeyExists(entity));
			_deletedEntities.SetKey(entity);
		}
		else if (oldDelete && !newDelete)
		{
			// Coming back to life somehow...
			verify(_deletedEntities.UnsetKey(entity));
		}
	}
	else if (eventId == ff::GetEntityEventDeleted())
	{
		_deletedEntities.UnsetKey(entity);
	}
}
