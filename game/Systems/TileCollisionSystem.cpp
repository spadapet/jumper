#include "pch.h"
#include "Components/EntityTypeComponent.h"
#include "Components/LogicStateComponent.h"
#include "Core/CameraView.h"
#include "Core/Helpers.h"
#include "Core/TileSet.h"
#include "Services/CameraService.h"
#include "Services/EntityLogicService.h"
#include "Services/LevelService.h"
#include "Systems/TileCollisionSystem.h"

struct TileCollisionSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS2(TileCollisionSystemEntry)

	LogicStateComponent* _state;

	Advancing::Type _type;
	IEntityTileCollisionLogic* _logic;
};

BEGIN_ENTRY_COMPONENTS(TileCollisionSystemEntry)
HAS_COMPONENT(LogicStateComponent)
END_ENTRY_COMPONENTS(TileCollisionSystemEntry)

TileCollisionSystemEntry::TileCollisionSystemEntry()
	: _type(Advancing::Type::None)
	, _logic(nullptr)
{
}

TileCollisionSystem::TileCollisionSystem(ILevelService* levelService)
	: _bucket(levelService->GetDomain().GetBucket<TileCollisionSystemEntry>())
	, _levelService(levelService)
{
	_collisions.Reserve(Constants::REASONABLE_ENTITY_BUFFER_SIZE);
	_collisionEntries.Reserve(Constants::REASONABLE_ENTITY_BUFFER_SIZE);
}

const TileCollisionStats& TileCollisionSystem::GetStats() const
{
	return _stats;
}

void TileCollisionSystem::Advance(ff::EntityDomain* domain)
{
	BuildCollisionEntries();

	for (CollisionEntry& collision : _collisionEntries)
	{
		TileCollisionSystemEntry& entry = *collision._entry;

		if (entry._logic == nullptr || entry._type != entry._state->_type)
		{
			entry._logic = _levelService->GetEntityLogicService()->GetTileCollisionLogic(entry._state->_type);
			entry._type = entry._state->_type;
		}

		if (!entry._state->IsDeleting())
		{
			entry._logic->HandleTileCollisions(
				entry._entity,
				*entry._state,
				collision._collisionCount ? _collisions.ConstData(collision._collisionStart) : nullptr,
				collision._collisionCount,
				collision._collisionTileTypes,
				collision._collisionHitRectTypes);
		}
	}
}

void TileCollisionSystem::BuildCollisionEntries()
{
	_collisions.Clear();
	_collisionEntries.Clear();

	const CameraView& cameraView = _levelService->GetCameraService()->GetAdvanceCameraView();
	_levelService->GetCollisionService()->GetTileCollisions(cameraView._worldUpdateBounds, EntityType::CollidesWithTiles, _collisions.GetCollector(), _stats);
	std::sort(_collisions.begin(), _collisions.end());

	EntityEvents::EntityTileCollisionsEventArgs eventArgs;
	eventArgs._collisions = _collisions.Size() ? _collisions.ConstData() : nullptr;
	eventArgs._count = _collisions.Size();
	_levelService->GetDomain().TriggerEvent(EntityEvents::ID_BEFORE_ENTITY_TILE_COLLISIONS, &eventArgs);

	ff::Entity prevEntity = ff::INVALID_ENTITY;
	CollisionEntry collision{};

	for (size_t i = 0; i < _collisions.Size(); i++)
	{
		const EntityTileCollision& etc = _collisions[i];
		if (prevEntity != etc._entity)
		{
			if (collision._collisionCount)
			{
				_collisionEntries.Push(collision);
				collision = CollisionEntry{};
			}

			prevEntity = etc._entity;
			collision._entry = _bucket->GetEntry(etc._entity);

			if (collision._entry)
			{
				collision._collisionStart = i;
			}
		}

		if (collision._entry)
		{
			collision._collisionCount++;
			collision._collisionTileTypes = Helpers::SetFlags(collision._collisionTileTypes, etc._tile->_type);
			collision._collisionHitRectTypes = Helpers::SetFlags(collision._collisionHitRectTypes, etc._tileHitRect->_type);
		}
	}

	if (collision._collisionCount)
	{
		_collisionEntries.Push(collision);
	}
}
