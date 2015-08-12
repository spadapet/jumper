#include "pch.h"
#include "Core/CameraArea.h"
#include "Core/Globals.h"
#include "Core/LevelData.h"
#include "Globals/AppGlobals.h"
#include "Input/Joystick/JoystickDevice.h"
#include "Input/Keyboard/KeyboardDevice.h"
#include "Input/Pointer/PointerDevice.h"
#include "Services/AppService.h"
#include "State/StateWrapper.h"
#include "States/LoadLevelState.h"
#include "States/PlayGameState.h"
#include "Types/Timer.h"

PlayGameState::PlayGameState(IAppService* appService, GameOptions gameOptions)
	: _appService(appService)
	, _gameOptions(gameOptions)
	, _vars(appService)
	, _playState(std::make_shared<ff::StateWrapper>())
	, _state(State::Init)
	, _appEventCookie(nullptr)
{
	_appService->RegisterGame(this);

	InitPlayers();
}

PlayGameState::~PlayGameState()
{
	_appService->GetEventHandler().Remove(_appEventCookie);
	_appService->UnregisterGame(this);
}

std::shared_ptr<ff::State> PlayGameState::Advance(ff::AppGlobals* globals)
{
	if (!_appEventCookie)
	{
		auto weakOwner = weak_from_this();
		_appEventCookie = _appService->GetEventHandler().Add([weakOwner](ff::hash_t eventId)
			{
				auto owner = std::static_pointer_cast<PlayGameState>(weakOwner.lock());
				if (owner)
				{
					owner->OnAppEvent(eventId);
				}
			});
	}

	switch (_state)
	{
	case State::Init:
		PlayLevel();
		_state = State::PlayLevel;
		break;

	case State::DebugRestartGame:
		return std::make_shared<PlayGameState>(_appService, _gameOptions);
	}

	_playState->Advance(globals);

	return nullptr;
}

void PlayGameState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_playState->Render(globals, target, depth);
}

void PlayGameState::OnFrameStarted(ff::AppGlobals* globals, ff::AdvanceType type)
{
	_playState->OnFrameStarted(globals, type);
}

void PlayGameState::SaveState(ff::AppGlobals* globals)
{
#ifdef _DEBUG
	globals->SetState(Strings::ID_VARS, _vars.Save());
#endif
}

void PlayGameState::LoadState(ff::AppGlobals* globals)
{
#ifdef _DEBUG
	_vars.Load(globals->GetState(Strings::ID_VARS));
#endif
}

IAppService* PlayGameState::GetAppService() const
{
	return _appService;
}

size_t PlayGameState::GetPlayerCount() const
{
	return _players.Size();
}

Player* PlayGameState::GetPlayer(size_t index)
{
	return &_players[index];
}

const GameOptions& PlayGameState::GetGameOptions() const
{
	return _gameOptions;
}

const Vars& PlayGameState::GetVars() const
{
	return _vars;
}

Vars& PlayGameState::GetCheatingVars()
{
	return _vars;
}

void PlayGameState::InitPlayers()
{
	switch (_gameOptions._type)
	{
	case GameType::Adventure:
		switch (_gameOptions._players)
		{
		case GamePlayers::Single:
			_players.Push(Player());
			break;

		default:
			assert(false);
		}
		break;

	default:
		assert(false);
	}
}

void PlayGameState::PlayLevel()
{
	ff::String levelName(L"Test.level");
	*_playState = std::make_shared<LoadLevelState>(this, levelName);
}

void PlayGameState::OnAppEvent(ff::hash_t eventId)
{
	if (eventId == GameEvents::ID_DEBUG_RESTART_GAME)
	{
		_state = State::DebugRestartGame;
	}
	else if (eventId == GameEvents::ID_DEBUG_RESTART_LEVEL && _state == State::PlayLevel)
	{
		PlayLevel();
	}
}
