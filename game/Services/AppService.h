#pragma once

#include "Dict/Dict.h"
#include "Types/Event.h"

namespace ff
{
	class AppGlobals;
	class IResources;
	class ProcessGlobals;
	class XamlGlobalState;
}

class IGameService;
struct GameOptions;
struct SystemOptions;

class IAppService
{
public:
	// Globals
	virtual ff::ProcessGlobals& GetProcessGlobals() = 0;
	virtual ff::AppGlobals& GetAppGlobals() = 0;
	virtual ff::XamlGlobalState& GetXamlGlobals() = 0;
	virtual ff::IResources* GetLevelResources() = 0;

	// Events
	virtual void PostEvent(ff::hash_t eventId) = 0;
	virtual ff::Event<ff::hash_t>& GetEventHandler() = 0;

	// Options
	virtual const SystemOptions& GetSystemOptions() const = 0;
	virtual const GameOptions& GetGameOptions() const = 0;
	virtual void SetSystemOptions(const SystemOptions& options) = 0;
	virtual void SetGameOptions(const GameOptions& options) = 0;
	virtual ff::Dict GetDefaultVars() const = 0;

	// Games
	virtual void RegisterGame(IGameService* game) = 0;
	virtual void UnregisterGame(IGameService* game) = 0;
	virtual const ff::Vector<IGameService*>& GetGames() const = 0;
	virtual IGameService* GetActiveGame() const = 0;

	// Debug
	virtual void ShowDebugUI(bool show) = 0;
	virtual bool IsShowingDebugUI() const = 0;
};
