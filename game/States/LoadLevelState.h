#pragma once

#include "Resource/ResourceValue.h"
#include "State/State.h"

namespace ff
{
	class IRenderDepth;
	class IRenderTarget;
}

class IGameService;
class LevelDataResource;

class LoadLevelState : public ff::State
{
public:
	LoadLevelState(IGameService* playerService, ff::StringRef levelId);

	// State
	virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;

private:
	IGameService* _gameService;
	ff::String _levelName;
	ff::TypedResource<LevelDataResource> _levelData;
};
