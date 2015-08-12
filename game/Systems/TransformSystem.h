#pragma once

#include "Systems/SystemBase.h"

struct TransformSystemEntry;

class TransformSystem
	: public SystemBase
	, public ff::IEntityEventHandler
	, public ff::IEntityBucketListener<TransformSystemEntry>
{
public:
	TransformSystem(ff::EntityDomain* domain);
	virtual ~TransformSystem();

	// IEntityEventHandler
	virtual void OnEntityDeleted(ff::Entity entity) override;
	virtual void OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs) override;

	// IEntityBucketListener
	virtual void OnEntryAdded(TransformSystemEntry& entry) override;
	virtual void OnEntryRemoving(TransformSystemEntry& entry) override;


private:
	void UpdateEntity(ff::Entity entity);
	void UpdateEntity(TransformSystemEntry& entry);

	ff::EntityDomain* _domain;
	ff::IEntityBucket<TransformSystemEntry>* _bucket;
};
