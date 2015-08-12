#include "pch.h"
#include "Components/BoundingBoxComponent.h"
#include "Components/EntityTypeComponent.h"
#include "Components/HitBoxComponent.h"
#include "Components/PositionComponent.h"
#include "Components/TilesComponent.h"
#include "Components/VelocityComponent.h"
#include "Core/CameraView.h"
#include "Core/Globals.h"
#include "Core/Helpers.h"
#include "Core/TileSet.h"
#include "Core/Vars.h"
#include "Globals/AppGlobals.h"
#include "Globals/AppGlobalsHelper.h"
#include "Graph/Render/MatrixStack.h"
#include "Graph/Render/RendererActive.h"
#include "Input/Pointer/PointerDevice.h"
#include "Render/RenderWrapper.h"
#include "Services/AppService.h"
#include "Services/CameraService.h"
#include "Services/CollisionService.h"
#include "Services/GameService.h"
#include "Services/LevelService.h"
#include "Services/RenderQueueService.h"
#include "Services/ViewService.h"
#include "Systems/DebugSystem.h"

static const float ENTITY_COLLISION_LIFETIME = 2;
static const float TILE_COLLISION_LIFETIME = 2;

struct DebugBoundingBoxSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS()

	BoundingBoxComponent* _box;
};

BEGIN_ENTRY_COMPONENTS(DebugBoundingBoxSystemEntry)
HAS_COMPONENT(BoundingBoxComponent)
END_ENTRY_COMPONENTS(DebugBoundingBoxSystemEntry)

struct DebugHitBoxSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS()

	HitBoxComponent* _hitBox;
};

BEGIN_ENTRY_COMPONENTS(DebugHitBoxSystemEntry)
HAS_COMPONENT(HitBoxComponent)
END_ENTRY_COMPONENTS(DebugHitBoxSystemEntry)

struct DebugPositionSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS()

	PositionComponent* _pos;
};

BEGIN_ENTRY_COMPONENTS(DebugPositionSystemEntry)
HAS_COMPONENT(PositionComponent)
END_ENTRY_COMPONENTS(DebugPositionSystemEntry)

struct DebugVelocitySystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS()

	PositionComponent* _pos;
	VelocityComponent* _vel;
};

BEGIN_ENTRY_COMPONENTS(DebugVelocitySystemEntry)
HAS_COMPONENT(PositionComponent)
HAS_COMPONENT(VelocityComponent)
END_ENTRY_COMPONENTS(DebugVelocitySystemEntry)

DebugSystem::DebugSystem(ILevelService* levelService)
	: _bucketBox(levelService->GetDomain().GetBucket<DebugBoundingBoxSystemEntry>())
	, _bucketHit(levelService->GetDomain().GetBucket<DebugHitBoxSystemEntry>())
	, _bucketPos(levelService->GetDomain().GetBucket<DebugPositionSystemEntry>())
	, _bucketVel(levelService->GetDomain().GetBucket<DebugVelocitySystemEntry>())
	, _levelService(levelService)
	, _vars(&levelService->GetGameService()->GetVars())
{
	_collisions.SetBucketCount(Constants::REASONABLE_ENTITY_BUCKET_SIZE);

	_levelService->GetDomain().AddEventHandler(EntityEvents::ID_BEFORE_ENTITY_COLLISIONS, this);
	_levelService->GetDomain().AddEventHandler(EntityEvents::ID_BEFORE_ENTITY_TILE_COLLISIONS, this);
}

DebugSystem::~DebugSystem()
{
	_levelService->GetDomain().RemoveEventHandler(EntityEvents::ID_BEFORE_ENTITY_COLLISIONS, this);
	_levelService->GetDomain().RemoveEventHandler(EntityEvents::ID_BEFORE_ENTITY_TILE_COLLISIONS, this);
}

void DebugSystem::Advance(ff::EntityDomain* domain)
{
	_mouseOverEntities.Clear();

	if (_vars->GetDebugShowHitBox())
	{
		ff::AppGlobals& globals = _levelService->GetGameService()->GetAppService()->GetAppGlobals();
		ff::PointFixedInt screenPoint = globals.GetPointer()->GetPos().ToType<ff::FixedInt>();
		_levelService->GetViewService()->EntityHitTest(ff::RectFixedInt(screenPoint, screenPoint), EntityType::Any, _mouseOverEntities.GetCollector());
	}

	if (_collisions.Size())
	{
		ff::AppGlobals& globals = _levelService->GetGameService()->GetAppService()->GetAppGlobals();
		bool deleteOldCollisions = (globals.GetAppGlobalsHelper()->GetAdvanceType(&globals) == ff::AdvanceType::SingleStep);

		for (auto i = _collisions.begin(); i != _collisions.end(); )
		{
			const ff::KeyValue<ff::RectFixedInt, CollisionRectInfo>& kvp = *i;
			i++;

			CollisionRectInfo& info = kvp.GetEditableValue();
			if ((deleteOldCollisions && info._count) || (++info._count >= info._life))
			{
				_collisions.DeleteKey(kvp);
			}
		}
	}
}

void DebugSystem::Render(ff::EntityDomain* domain, IRenderQueueService* render)
{
	if (_vars->GetDebugShowLevelGrid())
	{
		render->Draw(Rendering::Layer::DebugGrid, this, &DebugSystem::RenderLevelGrid);
	}

	if (_vars->GetDebugShowBoundingBox())
	{
		render->Draw(Rendering::Layer::ScreenResDebugBoxes, this, &DebugSystem::RenderBoundingBox);
	}

	if (_vars->GetDebugShowHitBox())
	{
		render->Draw(Rendering::Layer::ScreenResDebugBoxes, this, &DebugSystem::RenderHitBox);
	}

	if (_vars->GetDebugShowPosition())
	{
		render->Draw(Rendering::Layer::ScreenResDebugBoxes, this, &DebugSystem::RenderPosition);
	}

	if (_vars->GetDebugShowVelocity())
	{
		render->Draw(Rendering::Layer::ScreenResDebugBoxes, this, &DebugSystem::RenderVelocity);
	}

	if (_vars->GetDebugShowCameraFocus())
	{
		render->Draw(Rendering::Layer::ScreenResDebugBoxes, this, &DebugSystem::RenderCameraFocus);
	}

	if (_vars->GetDebugShowCollisions())
	{
		render->Draw(Rendering::Layer::ScreenResDebugCollisions, this, &DebugSystem::RenderCollisions);
	}
}

void DebugSystem::OnEntityDeleted(ff::Entity entity)
{
}

void DebugSystem::OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs)
{
	noAssertRet(_vars->GetDebugShowCollisions());

	if (eventId == EntityEvents::ID_BEFORE_ENTITY_COLLISIONS)
	{
		const EntityEvents::EntityCollisionsEventArgs& myArgs = *(EntityEvents::EntityCollisionsEventArgs*)eventArgs;
		for (size_t i = 0; i < myArgs._count; i++)
		{
			DebugHitBoxSystemEntry* box1 = _bucketHit->GetEntry(myArgs._collisions[i]._entity1);
			DebugHitBoxSystemEntry* box2 = _bucketHit->GetEntry(myArgs._collisions[i]._entity2);
			AddCollision(box1->_hitBox->_box.Intersect(box2->_hitBox->_box), CollisionRectInfo::Type::Entity);
		}
	}
	else if (eventId == EntityEvents::ID_BEFORE_ENTITY_TILE_COLLISIONS)
	{
		const EntityEvents::EntityTileCollisionsEventArgs& myArgs = *(EntityEvents::EntityTileCollisionsEventArgs*)eventArgs;
		for (size_t i = 0; i < myArgs._count; i++)
		{
			const EntityTileCollision& etc = myArgs._collisions[i];
			AddCollision(etc._worldRect, CollisionRectInfo::Type::Entity);

			ff::PointFixedInt tilePos = etc._worldRect.TopLeft() - etc._tileRect.TopLeft();
			CollisionRectInfo::Type type = CollisionRectInfo::Type::Tile;

			if (etc._tileHitRect->HasAny(TileHitRectType::RampUp, TileHitRectType::HeadUp))
			{
				type = CollisionRectInfo::Type::TileAngleUp;
			}
			else if (etc._tileHitRect->HasAny(TileHitRectType::RampDown, TileHitRectType::HeadDown))
			{
				type = CollisionRectInfo::Type::TileAngleDown;
			}

			AddCollision(etc._tileHitRect->_rect + tilePos, type);
		}
	}
}

void DebugSystem::AddCollision(const ff::RectFixedInt& rect, CollisionRectInfo::Type type)
{
	const ff::KeyValue<ff::RectFixedInt, CollisionRectInfo>* kvp = _collisions.GetKey(rect);
	if (kvp && kvp->GetValue()._type == type)
	{
		kvp->GetEditableValue()._count = 0;
	}
	else
	{
		CollisionRectInfo info;
		info._type = type;
		info._life = (type == CollisionRectInfo::Type::Entity) ? ENTITY_COLLISION_LIFETIME : TILE_COLLISION_LIFETIME;
		info._count = 0;

		_collisions.SetKey(rect, info);
	}
}

void DebugSystem::RenderBoundingBox(RenderWrapper& render)
{
	for (const DebugBoundingBoxSystemEntry& entry : _bucketBox->GetEntries())
	{
		ff::PointFixedInt points[5] =
		{
			entry._box->_points[0],
			entry._box->_points[1],
			entry._box->_points[2],
			entry._box->_points[3],
			entry._box->_points[0],
		};

		render.DrawLineStrip(points, 5, ff::GetColorWhite(), 1_f, true);
	}
}

void DebugSystem::RenderHitBox(RenderWrapper& render)
{
	for (const DebugHitBoxSystemEntry& entry : _bucketHit->GetEntries())
	{
		ff::PointFixedInt points[5] =
		{
			entry._hitBox->_points[0],
			entry._hitBox->_points[1],
			entry._hitBox->_points[2],
			entry._hitBox->_points[3],
			entry._hitBox->_points[0],
		};

		bool mouseOver = _mouseOverEntities.Contains(entry._entity);
		render.DrawLineStrip(points, 5, ff::GetColorRed(), mouseOver ? 4_f : 1_f, true);
	}
}

void DebugSystem::RenderPosition(RenderWrapper& render)
{
	for (const DebugPositionSystemEntry& entry : _bucketPos->GetEntries())
	{
		render.DrawOutlineCircle(entry._pos->_pos, 2_f, ff::GetColorGreen(), 1_f, true);
	}
}

void DebugSystem::RenderVelocity(RenderWrapper& render)
{
	for (const DebugVelocitySystemEntry& entry : _bucketVel->GetEntries())
	{
		if (!entry._vel->_posVelocity.IsNull())
		{
			float length = DirectX::XMVectorGetX(
				DirectX::XMVector2Length(Helpers::PointToXMVECTOR(entry._vel->_posVelocity)));

			ff::PointFloat normal;
			DirectX::XMStoreFloat2((DirectX::XMFLOAT2*) & normal,
				DirectX::XMVector2Normalize(Helpers::PointToXMVECTOR(entry._vel->_posVelocity)));

			normal *= length * 8;
			ff::PointFixedInt normal_f = normal.ToType<ff::FixedInt>();

			render.DrawLine(entry._pos->_pos, entry._pos->_pos + normal_f, ff::GetColorGreen(), 1_f, true);
			render.DrawOutlineCircle(entry._pos->_pos + normal_f, 2_f, ff::GetColorGreen(), 1_f, true);
		}
	}
}

void DebugSystem::RenderLevelGrid(RenderWrapper& render)
{
	const CameraView& camera = _levelService->GetCameraService()->GetFinalCameraView();
	ff::RectInt visibleTiles(
		Helpers::PointToCell(camera._worldBounds.TopLeft(), Constants::LEVEL_TILE_SIZE),
		Helpers::PointToCell(camera._worldBounds.BottomRight(), Constants::LEVEL_TILE_SIZE));

	render.GetRenderer()->PushNoOverlap();

	for (ff::PointInt xy = visibleTiles.TopLeft(); xy.y <= visibleTiles.bottom; xy.y++)
	{
		for (xy.x = visibleTiles.left; xy.x <= visibleTiles.right; xy.x++)
		{
			if ((xy.x + xy.y) % 2 == 0)
			{
				ff::RectFixedInt rect(
					Constants::LEVEL_TILE_WIDTH * xy.x,
					Constants::LEVEL_TILE_HEIGHT * xy.y,
					Constants::LEVEL_TILE_WIDTH * (xy.x + 1),
					Constants::LEVEL_TILE_HEIGHT * (xy.y + 1));

				XMCOLOR color(
					0.125f,
					((xy.x * 8) % 256) / 255.0f * 0.125f,
					((xy.y * 8) % 256) / 255.0f * 0.125f,
					1.0f);

				render.DrawFilledRectangle(rect, color);
			}
		}
	}

	render.GetRenderer()->PopNoOverlap();
}

void DebugSystem::RenderCameraFocus(RenderWrapper& render)
{
	const CameraView& cameraView = _levelService->GetCameraService()->GetRenderCameraView();

	render.GetRenderer()->GetWorldMatrixStack().PushMatrix();
	render.GetRenderer()->GetWorldMatrixStack().TransformMatrix(cameraView._viewToWorldTransform);

	ff::PointFixedInt viewCenter = cameraView._view.Center();
	const ff::FixedInt focusWidth = _vars->GetCameraViewFocusWidth() ? cameraView._view.Width() / _vars->GetCameraViewFocusWidth() : 0_f;
	const ff::FixedInt focusOffsetFromCenter = _vars->GetCameraViewFocusOffsetX() ? cameraView._view.Width() / _vars->GetCameraViewFocusOffsetX() : 0_f;

	render.DrawLine(
		ff::PointFixedInt(viewCenter.x - focusOffsetFromCenter, cameraView._view.top),
		ff::PointFixedInt(viewCenter.x - focusOffsetFromCenter, cameraView._view.bottom),
		ff::GetColorYellow(), 1_f, true);

	render.DrawLine(
		ff::PointFixedInt(viewCenter.x - focusOffsetFromCenter - focusWidth, cameraView._view.top),
		ff::PointFixedInt(viewCenter.x - focusOffsetFromCenter - focusWidth, cameraView._view.bottom),
		ff::GetColorYellow(), 1_f, true);

	render.DrawLine(
		ff::PointFixedInt(viewCenter.x + focusOffsetFromCenter, cameraView._view.top),
		ff::PointFixedInt(viewCenter.x + focusOffsetFromCenter, cameraView._view.bottom),
		ff::GetColorYellow(), 1_f, true);

	render.DrawLine(
		ff::PointFixedInt(viewCenter.x + focusOffsetFromCenter + focusWidth, cameraView._view.top),
		ff::PointFixedInt(viewCenter.x + focusOffsetFromCenter + focusWidth, cameraView._view.bottom),
		ff::GetColorYellow(), 1_f, true);

	render.GetRenderer()->GetWorldMatrixStack().PopMatrix();
}

void DebugSystem::RenderCollisions(RenderWrapper& render)
{
	for (auto& kvp : _collisions)
	{
		const ff::RectFixedInt& rect = kvp.GetKey();
		const CollisionRectInfo& info = kvp.GetValue();

		if (info._type == CollisionRectInfo::Type::Entity)
		{
			float opacity = 1.0f;
			if (info._count)
			{
				opacity -= (info._count - 1) / (info._life - 1);
			}

			XMCOLOR fillColor(1, 0, 1, 0.5f * opacity);
			XMCOLOR borderColor(1, 0, 1, opacity);

			if (rect.Area())
			{
				render.DrawFilledRectangle(rect, fillColor);
			}

			render.DrawOutlineRectangle(rect.Inflate(1, 1, 1, 1), borderColor, -1_f, true);
		}
		else
		{
			XMCOLOR borderColor(1, 1, 0, 1);

			render.DrawOutlineRectangle(rect, borderColor, 1_f, true);

			if (info._type == CollisionRectInfo::Type::TileAngleDown)
			{
				render.DrawLine(rect.TopLeft(), rect.BottomRight(), borderColor, 1_f, true);
			}
			else if (info._type == CollisionRectInfo::Type::TileAngleUp)
			{
				render.DrawLine(rect.BottomLeft(), rect.TopRight(), borderColor, 1_f, true);
			}
		}
	}
}
