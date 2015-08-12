#pragma once

#include "Services/CollisionService.h"
#include "Systems/SystemBase.h"

class ILevelService;
struct EntityCollisionSystemEntry;

class EntityCollisionSystem : public SystemBase
{
public:
	EntityCollisionSystem(ILevelService* levelService);

	const EntityCollisionStats& GetStats() const;

	// SystemBase
	virtual void Advance(ff::EntityDomain* domain) override;

private:
	struct CollisionEntry
	{
		bool operator<(const CollisionEntry& rhs) const;

		// Never null
		ff::Entity _entity1;
		ff::Entity _entity2;

		// Can be null
		EntityCollisionSystemEntry* _entry1;
		EntityCollisionSystemEntry* _entry2;
	};

	void BuildCollisions();

	ILevelService* _levelService;
	ff::IEntityBucket<EntityCollisionSystemEntry>* _bucket;
	ff::Vector<EntityCollision> _collisions;
	ff::Vector<CollisionEntry> _collisionEntries;
	EntityCollisionStats _stats;
};
