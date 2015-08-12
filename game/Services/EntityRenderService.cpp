#include "pch.h"
#include "Core/Globals.h"
#include "Render/LayerRender.h"
#include "Render/PlayerRender.h"
#include "Render/RenderBase.h"
#include "Services/EntityRenderService.h"
#include "Services/LevelService.h"

EntityRenderService::EntityRenderService(ILevelService* levelService)
	: _levelService(levelService)
{
}

EntityRenderService::~EntityRenderService()
{
}

IEntityRender* EntityRenderService::GetRender(Rendering::Type type)
{
	return GetRenderBase(type);
}

RenderBase* EntityRenderService::GetRenderBase(Rendering::Type type)
{
	auto i = _renders.GetKey(type);
	if (!i)
	{
		i = _renders.SetKey(std::move(type), std::shared_ptr<RenderBase>());
	}

	std::shared_ptr<RenderBase>& value = i->GetEditableValue();

	if (value == nullptr)
	{
		switch (type)
		{
		case Rendering::Type::Layer:
			value = std::make_shared<LayerRender>(_levelService->GetCameraService());
			break;

		case Rendering::Type::Player:
			value = std::make_shared<PlayerRender>();
			break;

		default:
			value = std::make_shared<RenderBase>();
			break;
		}
	}

	return value.get();
}
