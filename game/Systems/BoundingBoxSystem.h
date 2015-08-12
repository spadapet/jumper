#pragma once

#include "Systems/SystemBase.h"

class IViewService;
struct BoundingBoxSystemEntry;

class BoundingBoxSystem
	: public SystemBase
	, public ff::IEntityEventHandler
	, public ff::IEntityBucketListener<BoundingBoxSystemEntry>
{
public:
	BoundingBoxSystem(ff::EntityDomain* domain);
	virtual ~BoundingBoxSystem();

	// IEntityEventHandler
	virtual void OnEntityDeleted(ff::Entity entity) override;
	virtual void OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs) override;

	// IEntityBucketListener
	virtual void OnEntryAdded(BoundingBoxSystemEntry& entry) override;
	virtual void OnEntryRemoving(BoundingBoxSystemEntry& entry) override;

private:
	void UpdateEntity(ff::Entity entity);
	void UpdateEntity(BoundingBoxSystemEntry& entry);

	ff::EntityDomain* _domain;
	ff::IEntityBucket<BoundingBoxSystemEntry>* _bucket;
};
