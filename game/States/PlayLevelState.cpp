#include "pch.h"
#include "Components/CameraComponent.h"
#include "Components/LevelComponent.h"
#include "Core/Vars.h"
#include "Globals/AppGlobals.h"
#include "Graph/Render/RenderAnimation.h"
#include "Graph/Render/Renderer.h"
#include "Graph/RenderTarget/RenderDepth.h"
#include "Graph/RenderTarget/RenderTarget.h"
#include "Graph/Sprite/Sprite.h"
#include "Graph/Texture/Texture.h"
#include "Input/Joystick/JoystickDevice.h"
#include "Input/Joystick/JoystickInput.h"
#include "Input/Keyboard/KeyboardDevice.h"
#include "Input/Pointer/PointerDevice.h"
#include "Services/AppService.h"
#include "Services/EntityFactoryService.h"
#include "Services/EntityLogicService.h"
#include "Services/EntityRenderService.h"
#include "Services/GameService.h"
#include "Systems/BoundingBoxSystem.h"
#include "Systems/CameraViewSystem.h"
#include "Systems/CollisionSystem.h"
#include "Systems/DebugSystem.h"
#include "Systems/EntityCollisionSystem.h"
#include "Systems/EntityDeleteSystem.h"
#include "Systems/InputEventsSystem.h"
#include "Systems/LogicSystem.h"
#include "Systems/RenderSystem.h"
#include "Systems/TileCollisionSystem.h"
#include "Systems/TransformSystem.h"
#include "States/PlayLevelState.h"
#include "Types/Timer.h"

static ff::StaticString s_advancePerfName(L"Level frame perf");
static ff::StaticString s_collisionPerfName(L"Level collision perf");
static const size_t ADVANCE_PERF_INDEX = 0;
static const size_t COLLISION_PERF_INDEX = 1;

PlayLevelState::PlayLevelState(IGameService* gameService, LevelData&& levelData)
	: _gameService(gameService)
	, _inputResource(L"controls")
	, _levelData(std::move(levelData))
	, _timerFrequency((double)ff::Timer::GetRawFreqStatic())
{
	_entityFactoryService = std::make_unique<EntityFactoryService>(this);
	_entityLogicService = std::make_unique<EntityLogicService>(this);
	_entityRenderService = std::make_unique<EntityRenderService>(this);
	_renderService = std::make_unique<SystemRenderService>(this);

	InitInputDevices();
	InitSystems();
	InitEntities();

	_gameService->GetAppService()->GetAppGlobals().AddDebugPage(this);
}

PlayLevelState::~PlayLevelState()
{
	_gameService->GetAppService()->GetAppGlobals().RemoveDebugPage(this);
}

void PlayLevelState::InitInputDevices()
{
	ff::AppGlobals& globals = _gameService->GetAppService()->GetAppGlobals();

	_inputDevices._keys.Push(globals.GetKeys());
	_inputDevices._mice.Push(globals.GetPointer());

	for (size_t i = 0; i < globals.GetJoysticks()->GetCount(); i++)
	{
		_inputDevices._joys.Push(globals.GetJoysticks()->GetJoystick(i));
	}

	_noKeyboardInputDevices._mice = _inputDevices._mice;
	_noKeyboardInputDevices._joys = _inputDevices._joys;
}

void PlayLevelState::InitSystems()
{
	auto boundingBoxSystem = SystemBase::New<BoundingBoxSystem>(&_domain);
	auto cameraViewSystem = SystemBase::New<CameraViewSystem>(&_domain);
	auto collisionSystem = SystemBase::New<CollisionSystem>(&_domain, Constants::COLLISION_TILE_SIZE);
	auto debugSystem = SystemBase::New<DebugSystem>(this);
	auto entityCollisionSystem = SystemBase::New<EntityCollisionSystem>(this);
	auto entityDeleteSystem = SystemBase::New<EntityDeleteSystem>(&_domain);
	auto inputEventsSystem = SystemBase::New<InputEventsSystem>(&_domain);
	auto logicSystem = SystemBase::New<LogicSystem>(this);
	auto logicSystemAfterCollisions = SystemBase::New<LogicSystemAfterCollisions>(logicSystem);
	auto renderSystem = SystemBase::New<RenderSystem>(this);
	auto tileCollisionSystem = SystemBase::New<TileCollisionSystem>(this);
	auto transformSystem = SystemBase::New<TransformSystem>(&_domain);

	// Advance systems

	_advanceSystems.Push(inputEventsSystem);
	_advanceSystems.Push(logicSystem);
	_advanceSystems.Push(tileCollisionSystem);
	_advanceSystems.Push(entityCollisionSystem);
	_advanceSystems.Push(logicSystemAfterCollisions);
	_advanceSystems.Push(renderSystem);
	_advanceSystems.Push(debugSystem);
	_advanceSystems.Push(entityDeleteSystem);

	// Render systems

	_renderSystems.Push(renderSystem);
	_renderSystems.Push(debugSystem);

	// Keep alive systems

	_keepAliveSystems.Push(transformSystem);
	_keepAliveSystems.Push(boundingBoxSystem);
	_keepAliveSystems.Push(collisionSystem);
	_keepAliveSystems.Push(cameraViewSystem);

	_collisionService = collisionSystem;
	_entityCollisionSystem = entityCollisionSystem;
	_tileCollisionSystem = tileCollisionSystem;
}

void PlayLevelState::InitEntities()
{
	CreateLevel();
	CreatePlayer();
	CreateCamera();
}

void PlayLevelState::CreateLevel()
{
	_levelEntity = _entityFactoryService->CreateLevel(_levelData);
}

void PlayLevelState::CreatePlayer()
{
	assertRet(_gameService->GetPlayerCount());

	ff::PointFixedInt startPos = _levelData._playerStart;
	Player* player = _gameService->GetPlayer(0);
	_playerEntity = _entityFactoryService->CreatePlayer(player, startPos);
}

void PlayLevelState::CreateCamera()
{
	assert(_playerEntity);

	_cameraEntity = _entityFactoryService->CreateCamera(_playerEntity);
}

void PlayLevelState::UpdateDebugCamera()
{
	DebugCameraType type = _gameService->GetVars().GetDebugCameraType();
	switch (type)
	{
	default:
		if (_debugCameraEntity)
		{
			_debugCameraEntity.Delete();
		}
		break;

	case DebugCameraType::Zoom2:
	case DebugCameraType::Zoom4:
		if (_debugCameraEntity && _debugCameraEntity->GetComponent<CameraComponent>()->_debugType != type)
		{
			_debugCameraEntity.Delete();
		}

		if (!_debugCameraEntity)
		{
			_debugCameraEntity = _entityFactoryService->CreateCamera(_playerEntity, type);
		}
		break;
	}
}

std::shared_ptr<ff::State> PlayLevelState::Advance(ff::AppGlobals* globals)
{
	noAssertRetVal(_inputResource.HasObject(), nullptr);
	_inputResource->Advance(GetInputDevices(), globals->GetGlobalTime()._secondsPerAdvance);

	SystemBase::AdvanceHelper(&_domain, _advanceSystems);
	_domain.Advance();

	return nullptr;
}

void PlayLevelState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_renderService->Render(target, depth, _renderSystems.cbegin(), _renderSystems.cend());
}

void PlayLevelState::OnFrameStarted(ff::AppGlobals* globals, ff::AdvanceType type)
{
	UpdateDebugCamera();
}

const ff::IInputEvents* PlayLevelState::GetInputEvents()
{
	return _inputResource.GetObject();
}

const ff::InputDevices& PlayLevelState::GetInputDevices()
{
	return _gameService->GetAppService()->IsShowingDebugUI()
		? _noKeyboardInputDevices
		: _inputDevices;
}

size_t PlayLevelState::GetDebugPageCount() const
{
	return 2;
}

void PlayLevelState::DebugUpdateStats(ff::AppGlobals* globals, size_t page, bool updateFastNumbers)
{
	noAssertRet(updateFastNumbers);

	_entityCollisionStats = _entityCollisionSystem->GetStats();
	_tileCollisionStats = _tileCollisionSystem->GetStats();
	_renderServiceStats = _renderService->GetStats();

	_advanceSystemTicks.Resize(_advanceSystems.Size());
	_renderSystemTicks.Resize(_renderSystems.Size());

	for (size_t i = 0; i < _advanceSystems.Size(); i++)
	{
		_advanceSystemTicks[i] = _advanceSystems[i]->GetAdvanceTime();
	}

	for (size_t i = 0; i < _renderSystems.Size(); i++)
	{
		_renderSystemTicks[i] = _renderSystems[i]->GetRenderQueueTime();
	}
}

ff::String PlayLevelState::GetDebugName(size_t page) const
{
	switch (page)
	{
	case ADVANCE_PERF_INDEX:
		return s_advancePerfName;

	case COLLISION_PERF_INDEX:
		return s_collisionPerfName;

	default:
		return ff::String();
	}
}

size_t PlayLevelState::GetDebugInfoCount(size_t page) const
{
	switch (page)
	{
	case ADVANCE_PERF_INDEX:
		return _advanceSystemTicks.Size() + _renderSystemTicks.Size() + 3;

	case COLLISION_PERF_INDEX:
		return 4;

	default:
		return 0;
	}
}

ff::String PlayLevelState::GetDebugInfo(size_t page, size_t index, DirectX::XMFLOAT4& color) const
{
	switch (page)
	{
	case ADVANCE_PERF_INDEX:
		if (index < _advanceSystemTicks.Size())
		{
			color = ff::GetColorGreen();

			ff::String name = _advanceSystems[index]->GetName();
			double ms = _advanceSystemTicks[index] / _timerFrequency * 1000.0;
			return ff::String::format_new(L"Update %s: %.2gms", name.c_str(), ms);
		}
		else if (index - _advanceSystemTicks.Size() < _renderSystemTicks.Size())
		{
			color = ff::GetColorMagenta();

			index -= _advanceSystemTicks.Size();
			ff::String name = _renderSystems[index]->GetName();
			double ms = _renderSystemTicks[index] / _timerFrequency * 1000.0;
			return ff::String::format_new(L"Render queue %s: %.2gms", name.c_str(), ms);
		}
		else if (index - _advanceSystemTicks.Size() - _renderSystemTicks.Size() < 3)
		{
			color = ff::GetColorYellow();
			index -= _advanceSystemTicks.Size() + _renderSystemTicks.Size();

			switch (index)
			{
			case 0:
				return ff::String::format_new(L"Render low: %.2gms, %.2gms, %.2gms",
					_renderServiceStats._lowResTicks / _timerFrequency * 1000.0,
					_renderServiceStats._lowResOverlayTicks / _timerFrequency * 1000.0,
					_renderServiceStats._lowResStretchTicks / _timerFrequency * 1000.0);

			case 1:
				return ff::String::format_new(L"Render high: %.2gms, %.2gms, %.2gms",
					_renderServiceStats._highResTicks / _timerFrequency * 1000.0,
					_renderServiceStats._highResOverlayTicks / _timerFrequency * 1000.0,
					_renderServiceStats._highResStretchTicks / _timerFrequency * 1000.0);

			case 2:
				return ff::String::format_new(L"Render screen: %.2gms, %.2gms",
					_renderServiceStats._screenResTicks / _timerFrequency * 1000.0,
					_renderServiceStats._screenResOverlayTicks / _timerFrequency * 1000.0);
			}
		}

		return ff::String();

	case COLLISION_PERF_INDEX:
		switch (index)
		{
		case 0:
		{
			color = ff::GetColorGreen();
			double ms = _entityCollisionStats._ticks / _timerFrequency * 1000.0;
			return ff::String::format_new(L"Entity collision time: %.2gms", ms);
		}

		case 1:
			color = ff::GetColorGreen();
			return ff::String::format_new(L"- Cells:%lu, Pairs:%lu, Checked:%lu, Hits:%lu",
				_entityCollisionStats._cellCheckCount,
				_entityCollisionStats._entitySameCellCount,
				_entityCollisionStats._entityCheckCount,
				_entityCollisionStats._entityHitCount);

		case 2:
		{
			color = ff::GetColorMagenta();
			double ms = _tileCollisionStats._ticks / _timerFrequency * 1000.0;
			return ff::String::format_new(L"Tile collision time: %.2gms", ms);
		}

		case 3:
			color = ff::GetColorMagenta();
			return ff::String::format_new(L"- Layers:%lu, Cells:%lu, Checked:%lu, EntityHits:%lu, TileHits:%lu",
				_tileCollisionStats._layerCount,
				_tileCollisionStats._cellCheckCount,
				_tileCollisionStats._entityCheckCount,
				_tileCollisionStats._entityHitCount,
				_tileCollisionStats._tileHitCount);

		default:
			return ff::String();
		}
		break;

	default:
		return ff::String();
	}
}

size_t PlayLevelState::GetDebugToggleCount(size_t page) const
{
	return 0;
}

ff::String PlayLevelState::GetDebugToggle(size_t page, size_t index, int& value) const
{
	return ff::String();
}

void PlayLevelState::DebugToggle(size_t page, size_t index)
{
}

const ff::RectFixedInt& PlayLevelState::GetCameraBounds()
{
	return _levelEntity->GetComponent<LevelComponent>()->GetCameraBounds();
}

const ff::Vector<CameraArea>& PlayLevelState::GetCameraAreas()
{
	return _levelEntity->GetComponent<LevelComponent>()->GetCameraAreas();
}

const CameraView& PlayLevelState::GetAdvanceCameraView()
{
	return GetRenderCameraView();
}

const CameraView& PlayLevelState::GetRenderCameraView()
{
	return _cameraEntity->GetComponent<CameraComponent>()->_view;
}

const CameraView& PlayLevelState::GetFinalCameraView()
{
	if (_debugCameraEntity)
	{
		return _debugCameraEntity->GetComponent<CameraComponent>()->_view;
	}

	return GetRenderCameraView();
}

ff::EntityDomain& PlayLevelState::GetDomain()
{
	return _domain;
}

ff::IInputEventsProvider* PlayLevelState::GetInputEventsProvider()
{
	return this;
}

void PlayLevelState::GetUpdateEntities(ff::ItemCollector<ff::Entity>& entities)
{
	entities.Push(_cameraEntity);

	if (_debugCameraEntity)
	{
		entities.Push(_debugCameraEntity);
	}
}

ICameraService* PlayLevelState::GetCameraService()
{
	return this;
}

ICollisionService* PlayLevelState::GetCollisionService()
{
	return _collisionService.get();
}

IEntityFactoryService* PlayLevelState::GetEntityFactoryService()
{
	return _entityFactoryService.get();
}

IEntityLogicService* PlayLevelState::GetEntityLogicService()
{
	return _entityLogicService.get();
}

IEntityRenderService* PlayLevelState::GetEntityRenderService()
{
	return _entityRenderService.get();
}

IGameService* PlayLevelState::GetGameService()
{
	return _gameService;
}

IViewService* PlayLevelState::GetViewService()
{
	return _renderService.get();
}

