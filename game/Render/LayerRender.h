#pragma once

#include "Render/RenderBase.h"

class ICameraService;
class RenderWrapper;
template<typename T> struct RenderQueueData;

class LayerRender : public RenderBase
{
public:
	LayerRender(ICameraService* cameraService);
	~LayerRender();

	virtual void Render(ff::Entity entity, const LogicStateComponent& state, const RenderStateComponent& renderState, IRenderQueueService* render) override;

private:
	void DrawLayer(RenderQueueData<LayerRender>* cookie, RenderWrapper& render);

	ICameraService* _cameraService;
};
