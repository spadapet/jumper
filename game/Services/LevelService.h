#pragma once

namespace ff
{
	class EntityDomain;
	class IInputEventsProvider;
}

class ICameraService;
class ICollisionService;
class IEntityFactoryService;
class IEntityLogicService;
class IEntityRenderService;
class IGameService;
class IViewService;

class ILevelService
{
public:
	virtual ff::EntityDomain& GetDomain() = 0;
	virtual ff::IInputEventsProvider* GetInputEventsProvider() = 0;
	virtual void GetUpdateEntities(ff::ItemCollector<ff::Entity>& entities) = 0;

	virtual ICameraService* GetCameraService() = 0;
	virtual ICollisionService* GetCollisionService() = 0;
	virtual IEntityFactoryService* GetEntityFactoryService() = 0;
	virtual IEntityLogicService* GetEntityLogicService() = 0;
	virtual IEntityRenderService* GetEntityRenderService() = 0;
	virtual IGameService* GetGameService() = 0;
	virtual IViewService* GetViewService() = 0;
};
