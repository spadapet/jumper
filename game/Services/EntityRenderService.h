#pragma once

#include "Core/Globals.h"

class IEntityRender;
class ILevelService;
class IRenderQueueService;
class RenderBase;
struct LogicStateComponent;
struct RenderStateComponent;

class IEntityRender
{
public:
	virtual void Advance(ff::Entity entity, const LogicStateComponent& state, RenderStateComponent& renderState) = 0;
	virtual void Render(ff::Entity entity, const LogicStateComponent& state, const RenderStateComponent& renderState, IRenderQueueService* render) = 0;
};

class IEntityRenderService
{
public:
	virtual IEntityRender* GetRender(Rendering::Type type) = 0;
};

class EntityRenderService : public IEntityRenderService
{
public:
	EntityRenderService(ILevelService* levelService);
	~EntityRenderService();

	virtual IEntityRender* GetRender(Rendering::Type type) override;

private:
	RenderBase* GetRenderBase(Rendering::Type type);

	ILevelService* _levelService;
	ff::Map<Rendering::Type, std::shared_ptr<RenderBase>> _renders;
};
