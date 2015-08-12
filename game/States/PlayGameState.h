#pragma once

#include "Core/Options.h"
#include "Core/Player.h"
#include "Core/Vars.h"
#include "Services/GameService.h"
#include "State/State.h"

namespace ff
{
	class AppGlobals;
	class StateWrapper;
}

class IAppService;

class PlayGameState
	: public ff::State
	, public IGameService
{
public:
	PlayGameState(IAppService* appService, GameOptions gameOptions);
	~PlayGameState();

	// State
	virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
	virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;
	virtual void OnFrameStarted(ff::AppGlobals* globals, ff::AdvanceType type) override;
	virtual void SaveState(ff::AppGlobals* globals) override;
	virtual void LoadState(ff::AppGlobals* globals) override;

	// IGameService
	virtual IAppService* GetAppService() const override;
	virtual size_t GetPlayerCount() const override;
	virtual Player* GetPlayer(size_t index) override;
	virtual const GameOptions& GetGameOptions() const override;
	virtual const Vars& GetVars() const override;
	virtual Vars& GetCheatingVars() override;

private:
	void InitPlayers();
	void PlayLevel();
	void OnAppEvent(ff::hash_t eventId);

	enum class State
	{
		Init,
		PlayLevel,
		DebugRestartGame,
	} _state;

	IAppService* _appService;
	GameOptions _gameOptions;
	Vars _vars;
	ff::EventCookie _appEventCookie;

	ff::Vector<Player> _players;
	std::shared_ptr<ff::StateWrapper> _playState;
};
