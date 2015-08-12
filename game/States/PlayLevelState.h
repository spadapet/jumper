#pragma once

#include "Core/LevelData.h"
#include "Input/InputMapping.h"
#include "Services/CameraService.h"
#include "Services/CollisionService.h"
#include "Services/LevelService.h"
#include "Services/SystemRenderService.h"
#include "Services/ViewService.h"
#include "Resource/ResourceValue.h"
#include "State/IDebugPages.h"
#include "State/State.h"

namespace ff
{
	class IGraphTexture;
	class IInputMapping;
	class IRenderDepth;
	class IRenderTarget;
}

class EntityCollisionSystem;
class EntityFactoryService;
class EntityLogicService;
class EntityRenderService;
class IGameService;
class Level;
class LevelDataResource;
class SystemBase;
class TileCollisionSystem;

class PlayLevelState
	: public ff::State
	, public ff::IInputEventsProvider
	, public ff::IDebugPages
	, public ICameraService
	, public ILevelService
{
public:
	PlayLevelState(IGameService* gameService, LevelData&& levelData);
	~PlayLevelState();

	// State
	virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
	virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;
	virtual void OnFrameStarted(ff::AppGlobals* globals, ff::AdvanceType type) override;

	// IInputEventsProvider
	virtual const ff::IInputEvents* GetInputEvents() override;
	virtual const ff::InputDevices& GetInputDevices() override;

	// IDebugPages
	virtual size_t GetDebugPageCount() const override;
	virtual void DebugUpdateStats(ff::AppGlobals* globals, size_t page, bool updateFastNumbers) override;
	virtual ff::String GetDebugName(size_t page) const override;
	virtual size_t GetDebugInfoCount(size_t page) const override;
	virtual ff::String GetDebugInfo(size_t page, size_t index, DirectX::XMFLOAT4& color) const override;
	virtual size_t GetDebugToggleCount(size_t page) const override;
	virtual ff::String GetDebugToggle(size_t page, size_t index, int& value) const override;
	virtual void DebugToggle(size_t page, size_t index) override;

	// ICameraService
	virtual const ff::RectFixedInt& GetCameraBounds() override;
	virtual const ff::Vector<CameraArea>& GetCameraAreas() override;
	virtual const CameraView& GetAdvanceCameraView() override;
	virtual const CameraView& GetRenderCameraView() override;
	virtual const CameraView& GetFinalCameraView() override;

	// ILevelService
	virtual ff::EntityDomain& GetDomain() override;
	virtual ff::IInputEventsProvider* GetInputEventsProvider() override;
	virtual void GetUpdateEntities(ff::ItemCollector<ff::Entity>& entities) override;
	virtual ICameraService* GetCameraService() override;
	virtual ICollisionService* GetCollisionService() override;
	virtual IEntityFactoryService* GetEntityFactoryService() override;
	virtual IEntityLogicService* GetEntityLogicService() override;
	virtual IEntityRenderService* GetEntityRenderService() override;
	virtual IGameService* GetGameService() override;
	virtual IViewService* GetViewService() override;

private:
	void InitInputDevices();
	void InitSystems();
	void InitEntities();
	void CreateLevel();
	void CreatePlayer();
	void CreateCamera();
	void UpdateDebugCamera();

	IGameService* _gameService;
	LevelData _levelData;

	// Level entity domain
	ff::EntityDomain _domain;
	ff::WeakEntity _levelEntity;
	ff::WeakEntity _playerEntity;
	ff::WeakEntity _cameraEntity;
	ff::WeakEntity _debugCameraEntity;

	// Perf stats
	ff::Vector<UINT64> _advanceSystemTicks;
	ff::Vector<UINT64> _renderSystemTicks;
	EntityCollisionStats _entityCollisionStats;
	TileCollisionStats _tileCollisionStats;
	SystemRenderServiceStats _renderServiceStats;
	double _timerFrequency;

	// Input
	ff::TypedResource<ff::IInputMapping> _inputResource;
	ff::InputDevices _noKeyboardInputDevices;
	ff::InputDevices _inputDevices;

	// Services
	std::shared_ptr<ICollisionService> _collisionService;
	std::shared_ptr<EntityCollisionSystem> _entityCollisionSystem;
	std::shared_ptr<TileCollisionSystem> _tileCollisionSystem;
	std::unique_ptr<EntityFactoryService> _entityFactoryService;
	std::unique_ptr<EntityLogicService> _entityLogicService;
	std::unique_ptr<EntityRenderService> _entityRenderService;
	std::unique_ptr<SystemRenderService> _renderService;

	// Systems
	ff::Vector<std::shared_ptr<SystemBase>> _advanceSystems;
	ff::Vector<std::shared_ptr<SystemBase>> _renderSystems;
	ff::Vector<std::shared_ptr<SystemBase>> _keepAliveSystems;
};
