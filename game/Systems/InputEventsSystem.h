#pragma once

#include "Systems/SystemBase.h"

struct InputEventsSystemEntry;

class InputEventsSystem : public SystemBase
{
public:
	InputEventsSystem(ff::EntityDomain* domain);

	virtual void Advance(ff::EntityDomain* domain) override;

private:
	ff::IEntityBucket<InputEventsSystemEntry>* _bucket;
};
