#include "pch.h"
#include "Components/BoundingBoxComponent.h"
#include "Components/CameraComponent.h"
#include "Components/EntityTypeComponent.h"
#include "Components/HitBoxComponent.h"
#include "Components/InputForcesComponent.h"
#include "Components/InputStateComponent.h"
#include "Components/LevelComponent.h"
#include "Components/LogicStateComponent.h"
#include "Components/PlayerComponent.h"
#include "Components/PositionComponent.h"
#include "Components/PositionToHitBoxComponent.h"
#include "Components/PositionToBoundingBoxComponent.h"
#include "Components/RenderStateComponent.h"
#include "Components/TilesComponent.h"
#include "Components/TransformComponent.h"
#include "Components/UpdateInputComponent.h"
#include "Components/VelocityComponent.h"
#include "Core/Globals.h"
#include "Core/LevelData.h"
#include "Services/EntityFactoryService.h"
#include "Services/LevelService.h"

EntityFactoryService::EntityFactoryService(ILevelService* levelService)
	: _levelService(levelService)
{
	_blankEntity = _levelService->GetDomain().CreateEntity();
	_blankEntity->AddComponent<PositionComponent>();
	_blankEntity->AddComponent<TransformComponent>();
	_blankEntity->AddComponent<PositionToBoundingBoxComponent>();
	_blankEntity->AddComponent<BoundingBoxComponent>();

	_blankCollisionEntity = _blankEntity->Clone();
	_blankCollisionEntity->AddComponent<PositionToHitBoxComponent>();
	_blankCollisionEntity->AddComponent<HitBoxComponent>();
}

EntityFactoryService::~EntityFactoryService()
{
}

ff::Entity EntityFactoryService::CreateCamera(ff::Entity followEntity)
{
	ff::Entity entity = _blankEntity->Clone();

	ff::RectFixedInt toBounding(
		Constants::LEVEL_RENDER_WIDTH / -2_f,
		Constants::LEVEL_RENDER_HEIGHT / -2_f,
		Constants::LEVEL_RENDER_WIDTH / 2_f,
		Constants::LEVEL_RENDER_HEIGHT / 2_f);

	ff::PointFixedInt pos = /*followEntity
		? followEntity->GetComponent<PositionComponent>()->_pos
		: */ toBounding.BottomRight();

	entity->GetComponent<PositionToBoundingBoxComponent>()->Set(entity, toBounding);
	entity->GetComponent<PositionComponent>()->Set(entity, pos, 1, 0);

	entity->AddComponent<CameraComponent>()->SetFollow(followEntity);
	entity->AddComponent<LogicStateComponent>()->Init(Advancing::Type::Camera);
	entity->AddComponent<VelocityComponent>();
	entity->AddComponent<EntityTypeComponent>()->Set(entity, EntityType::Camera);

	entity->Activate();

	return entity;
}

ff::Entity EntityFactoryService::CreateCamera(ff::Entity followEntity, DebugCameraType type)
{
	if (type == DebugCameraType::None)
	{
		return CreateCamera(followEntity);
	}

	ff::Entity entity = _blankEntity->Clone();
	ff::FixedInt zoomLevel = (type == DebugCameraType::Zoom2) ? 2_f : 4_f;

	ff::RectFixedInt toBounding(
		Constants::LEVEL_RENDER_WIDTH / (zoomLevel * -2),
		Constants::LEVEL_RENDER_HEIGHT / (zoomLevel * -2),
		Constants::LEVEL_RENDER_WIDTH / (zoomLevel * 2),
		Constants::LEVEL_RENDER_HEIGHT / (zoomLevel * 2));

	ff::PointFixedInt pos = followEntity
		? followEntity->GetComponent<PositionComponent>()->_pos
		: toBounding.BottomRight();

	entity->GetComponent<PositionToBoundingBoxComponent>()->Set(entity, toBounding);
	entity->GetComponent<PositionComponent>()->Set(entity, pos, 1, 0);

	entity->AddComponent<LogicStateComponent>()->Init(Advancing::Type::DebugCamera);
	entity->AddComponent<EntityTypeComponent>()->Set(entity, EntityType::Camera);

	CameraComponent* camera = entity->AddComponent<CameraComponent>();
	camera->_debugType = type;
	camera->SetFollow(followEntity);

	entity->Activate();

	return entity;
}

ff::Entity EntityFactoryService::CreateLayer(const LevelData& levelData, size_t layer)
{
	const LevelData::Layer& layerData = levelData._layers[layer];
	ff::PointFixedInt layerSize(
		levelData._tileSize.x * layerData._size.x,
		levelData._tileSize.y * layerData._size.y);

	ff::Entity entity = _blankCollisionEntity->Clone();

	entity->AddComponent<TilesComponent>()->Init(levelData, layer);
	entity->AddComponent<PositionComponent>()->Set(entity, layerData._position, 1, 0);
	entity->AddComponent<PositionToBoundingBoxComponent>()->Set(entity, ff::RectFixedInt(layerSize));
	entity->AddComponent<PositionToHitBoxComponent>()->Set(entity, ff::RectFixedInt(layerSize));

	entity->AddComponent<LogicStateComponent>()->Init(Advancing::Type::Layer);
	entity->AddComponent<RenderStateComponent>()->Init(Rendering::Type::Layer);
	entity->AddComponent<EntityTypeComponent>()->Set(entity, EntityType::Layer);

	entity->Activate();

	return entity;
}

ff::Entity EntityFactoryService::CreateLevel(const LevelData& levelData)
{
	ff::Entity entity = _levelService->GetDomain().CreateEntity();

	entity->AddComponent<LevelComponent>()->Init(_levelService, levelData);

	entity->Activate();

	return entity;
}

ff::Entity EntityFactoryService::CreatePlayer(Player* player, ff::PointFixedInt pos)
{
	ff::Entity entity = _blankCollisionEntity->Clone();
	ff::RectFixedInt toBounding(-10, -20, 10, 0);
	ff::RectFixedInt toHit(-8, -18, 8, 0);

	entity->GetComponent<PositionToBoundingBoxComponent>()->Set(entity, toBounding);
	entity->GetComponent<PositionToHitBoxComponent>()->Set(entity, toHit);
	entity->GetComponent<PositionComponent>()->Set(entity, pos, 1, 0);

	entity->AddComponent<UpdateInputComponent>()->_provider = _levelService->GetInputEventsProvider();
	entity->AddComponent<InputStateComponent>();
	entity->AddComponent<PlayerComponent>()->_player = player;
	entity->AddComponent<LogicStateComponent>()->Init(Advancing::Type::Player);
	entity->AddComponent<RenderStateComponent>()->Init(Rendering::Type::Player);
	entity->AddComponent<VelocityComponent>();
	entity->AddComponent<EntityTypeComponent>()->Set(entity, EntityType::Player);

	SetDefaultPlayerInputForces(entity);

	entity->Activate();

	return entity;
}

void EntityFactoryService::SetDefaultPlayerInputForces(ff::Entity entity)
{
	const ff::FixedInt friction = 0.25_f;
	const ff::FixedInt maxVelocity = 3_f;

	InputForcesComponent& forces = *entity->AddComponent<InputForcesComponent>();
	entity->AddComponent<InputForcesRefComponent>()->_forces = &forces;

	forces.InitForPlayer(maxVelocity, friction);
}
