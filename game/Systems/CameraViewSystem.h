#pragma once

#include "Systems/SystemBase.h"

struct CameraViewSystemEntry;

class CameraViewSystem
	: public SystemBase
	, public ff::IEntityEventHandler
	, public ff::IEntityBucketListener<CameraViewSystemEntry>
{
public:
	CameraViewSystem(ff::EntityDomain* domain);
	virtual ~CameraViewSystem();

	// IEntityEventHandler
	virtual void OnEntityDeleted(ff::Entity entity) override;
	virtual void OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs) override;

	// IEntityBucketListener
	virtual void OnEntryAdded(CameraViewSystemEntry& entry) override;
	virtual void OnEntryRemoving(CameraViewSystemEntry& entry) override;

private:
	void UpdateEntity(ff::Entity entity);
	void UpdateEntity(CameraViewSystemEntry& entry);

	ff::EntityDomain* _domain;
	ff::IEntityBucket<CameraViewSystemEntry>* _bucket;
};
