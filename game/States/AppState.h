#pragma once

#include "Core/Options.h"
#include "Dict/ValueTable.h"
#include "Globals/AppGlobalsHelper.h"
#include "Input/InputMapping.h"
#include "Resource/Resources.h"
#include "Resource/ResourceValue.h"
#include "Services/AppService.h"
#include "State/State.h"

namespace ff
{
	class AppGlobals;
	class IRenderDepth;
	class IResources;
	class ProcessGlobals;
}

class DebugState;

class AppState
	: public ff::State
	, public ff::IAppGlobalsHelper
	, public ff::IResourceAccess
	, public ff::IValueAccess
	, public IAppService
{
public:
	AppState(ff::ProcessGlobals* processGlobals, ff::AppGlobals* globals);

	// State
	virtual void AdvanceDebugInput(ff::AppGlobals* globals) override;
	virtual void OnFrameRendered(ff::AppGlobals* globals, ff::AdvanceType type, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;
	virtual void SaveState(ff::AppGlobals* globals) override;

	// IAppService
	virtual void PostEvent(ff::hash_t eventId) override;
	virtual ff::Event<ff::hash_t>& GetEventHandler() override;
	virtual ff::ProcessGlobals& GetProcessGlobals() override;
	virtual ff::AppGlobals& GetAppGlobals() override;
	virtual ff::XamlGlobalState& GetXamlGlobals() override;
	virtual ff::IResources* GetLevelResources() override;
	virtual const SystemOptions& GetSystemOptions() const override;
	virtual const GameOptions& GetGameOptions() const override;
	virtual void SetSystemOptions(const SystemOptions& options) override;
	virtual void SetGameOptions(const GameOptions& options) override;
	virtual ff::Dict GetDefaultVars() const override;
	virtual void RegisterGame(IGameService* game) override;
	virtual void UnregisterGame(IGameService* game) override;
	virtual const ff::Vector<IGameService*>& GetGames() const override;
	virtual IGameService* GetActiveGame() const override;
	virtual void ShowDebugUI(bool show) override;
	virtual bool IsShowingDebugUI() const override;

	// IAppGlobalsHelper
	virtual bool OnInitialized(ff::AppGlobals* globals) override;
	virtual void OnGameThreadInitialized(ff::AppGlobals* globals) override;
	virtual void OnGameThreadShutdown(ff::AppGlobals* globals)  override;
	virtual std::shared_ptr<State> CreateInitialState(ff::AppGlobals* globals) override;
	virtual double GetTimeScale(ff::AppGlobals* globals) override;
	virtual ff::AdvanceType GetAdvanceType(ff::AppGlobals* globals) override;

	// IResourceAccess
	virtual ff::AutoResourceValue GetResource(ff::StringRef name) override;

	// IValueAccess
	virtual ff::ValuePtr GetValue(ff::StringRef name) const override;
	virtual ff::String GetString(ff::StringRef name) const override;

private:
	void InitOptions();
	void InitLevelResources();
	void InitInputDevices();

	const ff::InputDevices& GetDebugInputDevices();
	ff::ComPtr<ff::IResources> LoadLevelResources();
	void OnActiveGameChanged();

	ff::ProcessGlobals* _processGlobals;
	ff::AppGlobals* _globals;
	std::shared_ptr<ff::XamlGlobalState> _xamlGlobals;
	SystemOptions _systemOptions;
	GameOptions _gameOptions;
	ff::Vector<IGameService*> _games;
	ff::ComPtr<ff::IResources> _levelResources;
	ff::WinHandle _loadLevelResourcesEvent;
	ff::Event<ff::hash_t> _appEvents;

	// Debugging
	std::shared_ptr<DebugState> _debugState;
	ff::TypedResource<ff::IInputMapping> _debugInputResource;
	ff::InputDevices _debugInputDevices;
	ff::InputDevices _debugNoKeyboardInputDevices;
	ff::EventCookie _customDebugCookie;
	double _debugTimeScale;
	bool _debugSteppingFrames;
	bool _debugStepOneFrame;
	bool _showDebugUI;
};
