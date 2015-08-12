#include "pch.h"
#include "Components/PositionComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Globals.h"
#include "Systems/TransformSystem.h"

struct TransformSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS2(TransformSystemEntry)

	PositionComponent* _pos;
	TransformComponent* _transform;

	ff::hash_t _posHash;
};

BEGIN_ENTRY_COMPONENTS(TransformSystemEntry)
HAS_COMPONENT(PositionComponent)
HAS_COMPONENT(TransformComponent)
END_ENTRY_COMPONENTS(TransformSystemEntry)

TransformSystemEntry::TransformSystemEntry()
	: _posHash(0)
{
}

TransformSystem::TransformSystem(ff::EntityDomain* domain)
	: _domain(domain)
	, _bucket(domain->GetBucket<TransformSystemEntry>())
{
	_domain->AddEventHandler(EntityEvents::ID_POSITION_CHANGED, this);
	_bucket->AddListener(this);
}

TransformSystem::~TransformSystem()
{
	_domain->RemoveEventHandler(EntityEvents::ID_POSITION_CHANGED, this);
	_bucket->RemoveListener(this);
}

void TransformSystem::OnEntityDeleted(ff::Entity entity)
{
}

void TransformSystem::OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs)
{
	UpdateEntity(entity);
}

void TransformSystem::OnEntryAdded(TransformSystemEntry& entry)
{
	UpdateEntity(entry);
}

void TransformSystem::OnEntryRemoving(TransformSystemEntry& entry)
{
}

void TransformSystem::UpdateEntity(ff::Entity entity)
{
	TransformSystemEntry* entry = _bucket->GetEntry(entity);
	noAssertRet(entry);
	UpdateEntity(*entry);
}

void TransformSystem::UpdateEntity(TransformSystemEntry& entry)
{
	if (entry._posHash != entry._pos->_hash)
	{
		entry._posHash = entry._pos->_hash;
		entry._transform->Set(entry._entity, *entry._pos);
	}
}
