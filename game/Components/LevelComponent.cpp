#include "pch.h"
#include "Components/LevelComponent.h"
#include "Core/CameraArea.h"
#include "Core/LevelData.h"
#include "Services/EntityFactoryService.h"
#include "Services/LevelService.h"

LevelComponent::LevelComponent()
{
}

LevelComponent::~LevelComponent()
{
	for (ff::WeakEntity& layerEntity : _layerEntities)
	{
		layerEntity.Delete();
	}
}

void LevelComponent::Init(ILevelService* levelService, const LevelData& levelData)
{
	_levelService = levelService;

	ff::PointFixedInt worldSize(
		levelData._tileSize.x * levelData._size.x,
		levelData._tileSize.y * levelData._size.y);
	_cameraBounds.SetRect(ff::PointFixedInt::Zeros(), worldSize);

	_layerEntities.Reserve(levelData._layers.Size());
	for (size_t i = 0; i < levelData._layers.Size(); i++)
	{
		_layerEntities.Push(_levelService->GetEntityFactoryService()->CreateLayer(levelData, i));
	}
}

const ff::RectFixedInt& LevelComponent::GetCameraBounds()
{
	return _cameraBounds;
}

const ff::Vector<CameraArea>& LevelComponent::GetCameraAreas()
{
	return _cameraAreas;
}
