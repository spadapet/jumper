#pragma once

#include "Render/RenderBase.h"

class RenderWrapper;
template<typename T> struct RenderQueueData;

class PlayerRender : public RenderBase
{
public:
	virtual void Render(ff::Entity entity, const LogicStateComponent& state, const RenderStateComponent& renderState, IRenderQueueService* render) override;

private:
	void DrawPlayer(RenderQueueData<PlayerRender>* cookie, RenderWrapper& render);
};
