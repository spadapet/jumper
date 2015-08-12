#include "pch.h"
#include "Components/CameraComponent.h"
#include "Components/EntityTypeComponent.h"
#include "Components/LogicStateComponent.h"
#include "Components/RenderStateComponent.h"
#include "Render/RenderBase.h"
#include "Services/CameraService.h"
#include "Services/CollisionService.h"
#include "Services/EntityRenderService.h"
#include "Services/LevelService.h"
#include "Systems/RenderSystem.h"

struct RenderSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS2(RenderSystemEntry)

	RenderStateComponent* _renderState;
	LogicStateComponent* _logicState;

	Rendering::Type _type;
	IEntityRender* _render;
};

BEGIN_ENTRY_COMPONENTS(RenderSystemEntry)
HAS_COMPONENT(RenderStateComponent)
HAS_OPTIONAL_COMPONENT(LogicStateComponent)
END_ENTRY_COMPONENTS(RenderSystemEntry)

RenderSystemEntry::RenderSystemEntry()
	: _type(Rendering::Type::None)
{
}

RenderSystem::RenderSystem(ILevelService* levelService)
	: _bucket(levelService->GetDomain().GetBucket<RenderSystemEntry>())
	, _levelService(levelService)
{
}

void RenderSystem::Advance(ff::EntityDomain* domain)
{
	for (RenderSystemEntry* entry : BuildRenderEntries())
	{
		if (entry->_type != entry->_renderState->_renderType || entry->_render == nullptr)
		{
			entry->_type = entry->_renderState->_renderType;
			entry->_render = _levelService->GetEntityRenderService()->GetRender(entry->_type);
			assert(entry->_render != nullptr);
		}

		entry->_render->Advance(entry->_entity, LogicStateComponent::SafeGet(entry->_logicState), *entry->_renderState);
	}
}

void RenderSystem::Render(ff::EntityDomain* domain, IRenderQueueService* render)
{
	for (RenderSystemEntry* entry : _renderEntries)
	{
		if (entry->_render != nullptr) // before Advance is called
		{
			entry->_render->Render(entry->_entity, LogicStateComponent::SafeGet(entry->_logicState), *entry->_renderState, render);
		}
	}
}

const ff::Vector<RenderSystemEntry*>& RenderSystem::BuildRenderEntries()
{
	_renderEntities.Clear();
	_renderEntries.Clear();

	const CameraView& cameraView = _levelService->GetCameraService()->GetRenderCameraView();
	_levelService->GetCollisionService()->EntityHitTest(cameraView._worldRenderBounds, EntityType::Any, _renderEntities.GetCollector(), false);
	_renderEntries.Reserve(_renderEntities.Size());

	for (ff::Entity entity : _renderEntities)
	{
		RenderSystemEntry* entry = _bucket->GetEntry(entity);
		if (entry)
		{
			_renderEntries.Push(entry);
		}
	}

	return _renderEntries;
}
