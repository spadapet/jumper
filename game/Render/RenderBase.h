#pragma once

#include "Services/EntityRenderService.h"

namespace ff
{
	class AppGlobals;
}

class IRenderQueueService;
struct LogicStateComponent;
struct RenderStateComponent;

class RenderBase : public IEntityRender
{
public:
	virtual ~RenderBase();

	virtual void Advance(ff::Entity entity, const LogicStateComponent& state, RenderStateComponent& renderState) override;
	virtual void Render(ff::Entity entity, const LogicStateComponent& state, const RenderStateComponent& renderState, IRenderQueueService* render) override;
};
