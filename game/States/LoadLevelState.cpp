#include "pch.h"
#include "Core/Globals.h"
#include "Core/LevelData.h"
#include "Core/TileSet.h"
#include "Graph/Render/RenderAnimation.h"
#include "Module/Module.h"
#include "Resource/Resources.h"
#include "Services/AppService.h"
#include "Services/GameService.h"
#include "States/LoadLevelState.h"
#include "States/PlayLevelState.h"

LoadLevelState::LoadLevelState(IGameService* gameService, ff::StringRef levelId)
	: _gameService(gameService)
{
	ff::ComPtr<ff::IResources> levelResources = _gameService->GetAppService()->GetLevelResources();
	assert(levelResources->HasResource(levelId));

	_levelData.Init(levelResources, levelId);
	_levelName = ff::GetThisModule().GetString(ff::String::format_new(L"/Levels/%s/Name", levelId.c_str()));
}

std::shared_ptr<ff::State> LoadLevelState::Advance(ff::AppGlobals* globals)
{
	if (_levelData.HasObject() && _levelData->IsLevelDataFullyLoaded())
	{
		return std::make_shared<PlayLevelState>(_gameService, _levelData->GetLevelData());
	}

	return nullptr;
}
