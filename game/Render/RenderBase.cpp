#include "pch.h"
#include "Render/RenderBase.h"

RenderBase::~RenderBase()
{
}

void RenderBase::Advance(ff::Entity entity, const LogicStateComponent& state, RenderStateComponent& renderState)
{
}

void RenderBase::Render(ff::Entity entity, const LogicStateComponent& state, const RenderStateComponent& renderState, IRenderQueueService* render)
{
}
