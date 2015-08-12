#include "pch.h"
#include "Components/LogicStateComponent.h"
#include "Components/PositionComponent.h"
#include "Core/Globals.h"
#include "Core/Helpers.h"
#include "Graph/Render/RendererActive.h"
#include "Render/PlayerRender.h"
#include "Render/RenderWrapper.h"
#include "Services/RenderQueueService.h"

struct PlayerRenderData : public RenderQueueData<PlayerRender>
{
	ff::Entity _entity;
	const LogicStateComponent* _logic;
	const RenderStateComponent* _render;
	const PositionComponent* _pos;
};

void PlayerRender::Render(ff::Entity entity, const LogicStateComponent& state, const RenderStateComponent& renderState, IRenderQueueService* render)
{
	PlayerRenderData* data = render->Draw<PlayerRender, PlayerRenderData>(Rendering::Layer::Player, this, &PlayerRender::DrawPlayer);
	data->_entity = entity;
	data->_logic = &state;
	data->_render = &renderState;
	data->_pos = entity->GetComponent<PositionComponent>();
}

void PlayerRender::DrawPlayer(RenderQueueData<PlayerRender>* cookie, RenderWrapper& render)
{
	PlayerRenderData* data = (PlayerRenderData*)cookie;
	ff::PointFixedInt pos = data->_pos->_pos;
	render.DrawFilledCircle(pos.Offset(0_f, -10_f), 8_f, ff::GetColorYellow());
}
