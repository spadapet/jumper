#include "pch.h"
#include "Components/EntityTypeComponent.h"
#include "Components/LogicStateComponent.h"
#include "Core/CameraView.h"
#include "Services/CameraService.h"
#include "Services/EntityLogicService.h"
#include "Services/LevelService.h"
#include "Systems/EntityCollisionSystem.h"

struct EntityCollisionSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS2(EntityCollisionSystemEntry)

	LogicStateComponent* _state;

	Advancing::Type _type;
	IEntityCollisionLogic* _logic;
};

BEGIN_ENTRY_COMPONENTS(EntityCollisionSystemEntry)
HAS_COMPONENT(LogicStateComponent)
END_ENTRY_COMPONENTS(EntityCollisionSystemEntry)

EntityCollisionSystemEntry::EntityCollisionSystemEntry()
	: _type(Advancing::Type::None)
	, _logic(nullptr)
{
}

EntityCollisionSystem::EntityCollisionSystem(ILevelService* levelService)
	: _bucket(levelService->GetDomain().GetBucket<EntityCollisionSystemEntry>())
	, _levelService(levelService)
{
	_collisions.Reserve(Constants::REASONABLE_ENTITY_BUFFER_SIZE);
	_collisionEntries.Reserve(Constants::REASONABLE_ENTITY_BUFFER_SIZE);
}

const EntityCollisionStats& EntityCollisionSystem::GetStats() const
{
	return _stats;
}

bool EntityCollisionSystem::CollisionEntry::operator<(const CollisionEntry& rhs) const
{
	if (_entity1 == rhs._entity1)
	{
		return _entity2->GetId() < rhs._entity2->GetId();
	}

	return _entity1->GetId() < rhs._entity1->GetId();
}

void EntityCollisionSystem::Advance(ff::EntityDomain* domain)
{
	BuildCollisions();

	for (const CollisionEntry& pair : _collisionEntries)
	{
		EntityCollisionSystemEntry* entry1 = pair._entry1;
		EntityCollisionSystemEntry* entry2 = pair._entry2;

		if ((entry1 && entry1->_state->IsDeleting()) || (entry2 && entry2->_state->IsDeleting()))
		{
			continue;
		}

		if (entry1 && (entry1->_logic == nullptr || entry1->_type != entry1->_state->_type))
		{
			entry1->_logic = _levelService->GetEntityLogicService()->GetCollisionLogic(entry1->_state->_type);
			entry1->_type = entry1->_state->_type;
		}

		if (entry2 && (entry2->_logic == nullptr || entry2->_type != entry2->_state->_type))
		{
			entry2->_logic = _levelService->GetEntityLogicService()->GetCollisionLogic(entry2->_state->_type);
			entry2->_type = entry2->_state->_type;
		}

		if (entry1)
		{
			entry1->_logic->HandleCollision(pair._entity1, *entry1->_state, pair._entity2);
		}

		if (entry2)
		{
			entry2->_logic->HandleCollision(pair._entity2, *entry2->_state, pair._entity1);
		}
	}
}

void EntityCollisionSystem::BuildCollisions()
{
	_collisions.Clear();

	const CameraView& cameraView = _levelService->GetCameraService()->GetAdvanceCameraView();
	_levelService->GetCollisionService()->GetEntityCollisions(cameraView._worldUpdateBounds, EntityType::CollidesWithEachOther, _collisions.GetCollector(), _stats);

	EntityEvents::EntityCollisionsEventArgs eventArgs;
	eventArgs._collisions = _collisions.Size() ? _collisions.ConstData() : nullptr;
	eventArgs._count = _collisions.Size();
	_levelService->GetDomain().TriggerEvent(EntityEvents::ID_BEFORE_ENTITY_COLLISIONS, &eventArgs);

	_collisionEntries.Clear();
	_collisionEntries.Reserve(_collisions.Size());

	for (EntityCollision ec : _collisions)
	{
		EntityCollisionSystemEntry* entry1 = _bucket->GetEntry(ec._entity1);
		EntityCollisionSystemEntry* entry2 = _bucket->GetEntry(ec._entity2);

		if (entry2 && (!entry1 || entry2->_state->_type < entry1->_state->_type))
		{
			std::swap(entry1, entry2);
			std::swap(ec._entity1, ec._entity2);
		}

		CollisionEntry pair;
		pair._entry1 = entry1;
		pair._entry2 = entry2;
		pair._entity1 = ec._entity1;
		pair._entity2 = ec._entity2;

		_collisionEntries.Push(pair);
	}

	std::sort(_collisionEntries.begin(), _collisionEntries.end());
}
