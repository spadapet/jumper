#pragma once

#include "Core/Globals.h"
#include "Systems/SystemBase.h"

class ILevelService;
struct RenderSystemEntry;

class RenderSystem : public SystemBase
{
public:
	RenderSystem(ILevelService* levelService);

	// SystemBase
	virtual void Advance(ff::EntityDomain* domain) override;
	virtual void Render(ff::EntityDomain* domain, IRenderQueueService* render) override;

private:
	const ff::Vector<RenderSystemEntry*>& BuildRenderEntries();

	typedef void (RenderSystem::* UpdateFunc)(ff::EntityDomain* domain, const RenderSystemEntry& entry);

	ff::IEntityBucket<RenderSystemEntry>* _bucket;
	ff::Vector<ff::Entity> _renderEntities;
	ff::Vector<RenderSystemEntry*> _renderEntries;
	ILevelService* _levelService;
};
