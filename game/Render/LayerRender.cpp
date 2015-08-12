#include "pch.h"
#include "Components/PositionComponent.h"
#include "Components/TilesComponent.h"
#include "Core/CameraView.h"
#include "Core/TileSet.h"
#include "Core/Globals.h"
#include "Core/Helpers.h"
#include "Graph/Anim/AnimKeys.h"
#include "Graph/Render/RenderAnimation.h"
#include "Graph/Render/RendererActive.h"
#include "Render/LayerRender.h"
#include "Render/RenderWrapper.h"
#include "Services/CameraService.h"
#include "Services/RenderQueueService.h"

struct LayerRenderData : public RenderQueueData<LayerRender>
{
	ff::Entity _entity;
	ff::RectFixedInt _worldRenderBounds;
	const PositionComponent* _pos;
	const TilesComponent* _tiles;
};

LayerRender::LayerRender(ICameraService* cameraService)
	: _cameraService(cameraService)
{
}

LayerRender::~LayerRender()
{
}

void LayerRender::Render(ff::Entity entity, const LogicStateComponent& state, const RenderStateComponent& renderState, IRenderQueueService* render)
{
	LayerRenderData* data = render->Draw<LayerRender, LayerRenderData>(Rendering::Layer::LevelTiles, this, &LayerRender::DrawLayer);
	data->_entity = entity;
	data->_worldRenderBounds = _cameraService->GetFinalCameraView()._worldBounds;
	data->_pos = entity->GetComponent<PositionComponent>();
	data->_tiles = entity->GetComponent<TilesComponent>();
}

void LayerRender::DrawLayer(RenderQueueData<LayerRender>* cookie, RenderWrapper& render)
{
	LayerRenderData* data = (LayerRenderData*)cookie;
	assertRet(data->_pos && data->_tiles);

	const TilesComponent& tiles = *data->_tiles;

	ff::RectFixedInt worldBounds = data->_worldRenderBounds - data->_pos->_pos;
	ff::RectInt cells(
		Helpers::PointToCell(worldBounds.TopLeft(), tiles._tileSize),
		Helpers::PointToCell(worldBounds.BottomRight(), tiles._tileSize));

	int y1 = std::max(cells.top, 0);
	int y2 = std::min(cells.bottom, tiles._size.y - 1);

	int x1 = std::max(cells.left, 0);
	int x2 = std::min(cells.right, tiles._size.x - 1);

	noAssertRet(x2 >= x1 && y2 >= y1);

	for (int y = y1; y <= y2; y++)
	{
		ff::PointFixedInt pos(
			data->_pos->_pos.x + tiles._tileSize.x * x1,
			data->_pos->_pos.y + tiles._tileSize.y * y);

		const TilesComponent::TileInfo* tileInfo = &tiles._tiles[y * tiles._size.x + x1];
		for (int x = x1; x <= x2; x++, tileInfo++, pos.x += tiles._tileSize.x)
		{
			if (tileInfo->_tile && tileInfo->_tile->_anim)
			{
				render.DrawAnimationFirstFrame(tileInfo->_tile->_anim, pos, ff::PointFixedInt::Ones(), 0_f, tiles._color);
			}
		}
	}
}
