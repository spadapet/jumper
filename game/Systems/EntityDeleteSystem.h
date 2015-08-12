#pragma once

#include "Systems/SystemBase.h"

class EntityDeleteSystem
	: public SystemBase
	, public ff::IEntityEventHandler
{
public:
	EntityDeleteSystem(ff::EntityDomain* domain);
	virtual ~EntityDeleteSystem();

	// SystemBase
	virtual void Advance(ff::EntityDomain* domain) override;

	// IEntityEventHandler
	virtual void OnEntityDeleted(ff::Entity entity) override;
	virtual void OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs) override;

private:
	ff::EntityDomain* _domain;
	ff::Set<ff::Entity> _deletedEntities;
	ff::Vector<ff::Entity> _deletedEntitiesCopy;
};
