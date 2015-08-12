#include "pch.h"
#include "Core/Globals.h"
#include "Logic/DebugCameraLogic.h"
#include "Logic/CameraLogic.h"
#include "Logic/PlayerLogic.h"
#include "Services/EntityLogicService.h"
#include "Services/GameService.h"
#include "Services/LevelService.h"

EntityLogicService::EntityLogicService(ILevelService* levelService)
	: _levelService(levelService)
{
}

EntityLogicService::~EntityLogicService()
{
}

IEntityMoveLogic* EntityLogicService::GetMoveLogic(Advancing::Type type)
{
	return GetLogicBase(type);
}

IEntityCollisionLogic* EntityLogicService::GetCollisionLogic(Advancing::Type type)
{
	return GetLogicBase(type);
}

IEntityTileCollisionLogic* EntityLogicService::GetTileCollisionLogic(Advancing::Type type)
{
	return GetLogicBase(type);
}

LogicBase* EntityLogicService::GetLogicBase(Advancing::Type type)
{
	auto i = _logics.GetKey(type);
	if (!i)
	{
		i = _logics.SetKey(std::move(type), std::shared_ptr<LogicBase>());
	}

	std::shared_ptr<LogicBase>& logic = i->GetEditableValue();

	if (logic == nullptr)
	{
		switch (type)
		{
		case Advancing::Type::Camera:
			logic = std::make_shared<CameraLogic>(_levelService->GetCameraService(), &_levelService->GetGameService()->GetVars());
			break;

		case Advancing::Type::DebugCamera:
			logic = std::make_shared<DebugCameraLogic>();
			break;

		case Advancing::Type::Player:
			logic = std::make_shared<PlayerLogic>(_levelService);
			break;

		default:
			logic = std::make_shared<LogicBase>();
			break;
		}
	}

	return logic.get();
}
