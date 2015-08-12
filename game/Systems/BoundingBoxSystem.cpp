#include "pch.h"
#include "Components/BoundingBoxComponent.h"
#include "Components/HitBoxComponent.h"
#include "Components/PositionComponent.h"
#include "Components/PositionToBoundingBoxComponent.h"
#include "Components/PositionToHitBoxComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Globals.h"
#include "Globals/AppGlobals.h"
#include "Graph/Render/RendererActive.h"
#include "Input/Pointer/PointerDevice.h"
#include "Services/RenderQueueService.h"
#include "Services/ViewService.h"
#include "Systems/BoundingBoxSystem.h"

struct BoundingBoxSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS2(BoundingBoxSystemEntry)

	TransformComponent* _transform;
	PositionToBoundingBoxComponent* _posToBox;
	BoundingBoxComponent* _boundingBox;
	PositionToHitBoxComponent* _posToHitBox;
	HitBoxComponent* _hitBox;

	ff::hash_t _transformHash;
	ff::hash_t _posToBoxHash;
	ff::hash_t _posToHitBoxHash;
};

BEGIN_ENTRY_COMPONENTS(BoundingBoxSystemEntry)
HAS_COMPONENT(TransformComponent)
HAS_COMPONENT(PositionToBoundingBoxComponent)
HAS_COMPONENT(BoundingBoxComponent)
HAS_OPTIONAL_COMPONENT(PositionToHitBoxComponent)
HAS_OPTIONAL_COMPONENT(HitBoxComponent)
END_ENTRY_COMPONENTS(BoundingBoxSystemEntry)

BoundingBoxSystemEntry::BoundingBoxSystemEntry()
	: _transformHash(0)
	, _posToBoxHash(0)
	, _posToHitBoxHash(0)
{
}

BoundingBoxSystem::BoundingBoxSystem(ff::EntityDomain* domain)
	: _domain(domain)
	, _bucket(domain->GetBucket<BoundingBoxSystemEntry>())
{
	_domain->AddEventHandler(EntityEvents::ID_POSITION_TO_BOUNDING_BOX_CHANGED, this);
	_domain->AddEventHandler(EntityEvents::ID_POSITION_TO_HIT_BOX_CHANGED, this);
	_domain->AddEventHandler(EntityEvents::ID_TRANSFORM_CHANGED, this);
	_bucket->AddListener(this);
}

BoundingBoxSystem::~BoundingBoxSystem()
{
	_domain->RemoveEventHandler(EntityEvents::ID_POSITION_TO_BOUNDING_BOX_CHANGED, this);
	_domain->RemoveEventHandler(EntityEvents::ID_POSITION_TO_HIT_BOX_CHANGED, this);
	_domain->RemoveEventHandler(EntityEvents::ID_TRANSFORM_CHANGED, this);
	_bucket->RemoveListener(this);
}

void BoundingBoxSystem::OnEntityDeleted(ff::Entity entity)
{
}

void BoundingBoxSystem::OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs)
{
	UpdateEntity(entity);
}

void BoundingBoxSystem::OnEntryAdded(BoundingBoxSystemEntry& entry)
{
	UpdateEntity(entry);
}

void BoundingBoxSystem::OnEntryRemoving(BoundingBoxSystemEntry& entry)
{
}

void BoundingBoxSystem::UpdateEntity(ff::Entity entity)
{
	BoundingBoxSystemEntry* entry = _bucket->GetEntry(entity);
	noAssertRet(entry);
	UpdateEntity(*entry);
}

void BoundingBoxSystem::UpdateEntity(BoundingBoxSystemEntry& entry)
{
	bool transformChanged = false;
	if (entry._transformHash != entry._transform->_hash)
	{
		entry._transformHash = entry._transform->_hash;
		transformChanged = true;
	}

	if (transformChanged || entry._posToBoxHash != entry._posToBox->_hash)
	{
		entry._posToBoxHash = entry._posToBox->_hash;
		entry._boundingBox->Set(entry._entity, *entry._posToBox, *entry._transform);
	}

	if (entry._hitBox)
	{
		const PositionToBoundingBoxComponent* posToBox = !entry._posToHitBox ? entry._posToBox : entry._posToHitBox;
		if (transformChanged || entry._posToHitBoxHash != posToBox->_hash)
		{
			entry._posToHitBoxHash = posToBox->_hash;
			entry._hitBox->Set(entry._entity, *posToBox, *entry._transform);
		}
	}
}
