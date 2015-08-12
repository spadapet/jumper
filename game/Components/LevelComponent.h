#pragma once

#include "Core/CameraArea.h"

class ILevelService;
struct LevelData;

class LevelComponent : public ff::Component
{
public:
	LevelComponent();
	~LevelComponent();

	void Init(ILevelService* levelService, const LevelData& levelData);

	const ff::RectFixedInt& GetCameraBounds();
	const ff::Vector<CameraArea>& GetCameraAreas();

private:
	ILevelService* _levelService;
	ff::RectFixedInt _cameraBounds;
	ff::Vector<CameraArea> _cameraAreas;
	ff::Vector<ff::WeakEntity> _layerEntities;
};
