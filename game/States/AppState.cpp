#include "pch.h"
#include "App.xaml.h"
#include "Core/Globals.h"
#include "Core/Vars.h"
#include "Data/Data.h"
#include "Data/SavedData.h"
#include "Dict/Dict.h"
#include "Globals/AppGlobals.h"
#include "Globals/ProcessGlobals.h"
#include "Module/Module.h"
#include "Resource/Resources.h"
#include "Services/GameService.h"
#include "State/DebugPageState.h"
#include "State/States.h"
#include "States/AppState.h"
#include "States/DebugState.h"
#include "States/TitleState.h"
#include "String/StringUtil.h"
#include "Thread/ThreadDispatch.h"
#include "Thread/ThreadPool.h"
#include "Thread/ThreadUtil.h"
#include "Value/Values.h"
#include "Windows/FileUtil.h"
#include "UI/Components.h"
#include "UI/DebugControl.h"
#include "UI/DebugPage.h"
#include "UI/XamlGlobalState.h"

static ff::StaticString APP_STATE_NAME(L"AppState");

AppState::AppState(ff::ProcessGlobals* processGlobals, ff::AppGlobals* globals)
	: _processGlobals(processGlobals)
	, _globals(globals)
	, _loadLevelResourcesEvent(ff::CreateEvent())
	, _debugState(std::make_shared<DebugState>(this))
	, _debugSteppingFrames(false)
	, _debugStepOneFrame(false)
	, _debugTimeScale(1.0)
	, _customDebugCookie(nullptr)
	, _showDebugUI(false)
{
}

void AppState::AdvanceDebugInput(ff::AppGlobals* globals)
{
	bool debugEnabled = DEBUG || (GetActiveGame() && GetActiveGame()->GetVars().GetDebugKeysEnabled());
	if (!debugEnabled)
	{
		_debugStepOneFrame = false;
		_debugSteppingFrames = false;
		_debugTimeScale = 1.0;
		return;
	}

	noAssertRet(_debugInputResource.HasObject());

	if (_debugInputResource->Advance(GetDebugInputDevices(), GetAppGlobals().GetGlobalTime()._secondsPerAdvance))
	{
		if (_debugInputResource->HasStartEvent(InputEvents::ID_DEBUG_SHOW_UI) && !IsShowingDebugUI())
		{
			ShowDebugUI(true);
		}

		if (_debugInputResource->HasStartEvent(InputEvents::ID_DEBUG_CANCEL_STEP_ONE_FRAME))
		{
			_debugStepOneFrame = false;
			_debugSteppingFrames = false;
		}

		if (_debugInputResource->HasStartEvent(InputEvents::ID_DEBUG_STEP_ONE_FRAME))
		{
			_debugStepOneFrame = _debugSteppingFrames;
			_debugSteppingFrames = true;
		}

		if (_debugInputResource->HasStartEvent(InputEvents::ID_DEBUG_ZOOM_PLAYER_CAMERA) && GetActiveGame())
		{
			GetActiveGame()->GetCheatingVars().ToggleDebugCameraType();
		}
	}

	if (_debugInputResource->GetDigitalValue(GetDebugInputDevices(), InputEvents::ID_DEBUG_SPEED_FAST))
	{
		_debugTimeScale = 4.0;
	}
	else if (_debugInputResource->GetDigitalValue(GetDebugInputDevices(), InputEvents::ID_DEBUG_SPEED_SLOW))
	{
		_debugTimeScale = 0.25;
	}
	else
	{
		_debugTimeScale = 1.0;
	}
}

void AppState::OnFrameRendered(ff::AppGlobals* globals, ff::AdvanceType type, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_debugStepOneFrame = false;
}

void AppState::SaveState(ff::AppGlobals* globals)
{
	ff::Dict dict = globals->GetState(::APP_STATE_NAME);
	dict.Set<ff::DataValue>(Strings::ID_SYSTEM_OPTIONS, &_systemOptions, sizeof(SystemOptions));
	dict.Set<ff::DataValue>(Strings::ID_GAME_OPTIONS, &_gameOptions, sizeof(GameOptions));

	globals->SetState(::APP_STATE_NAME, dict);
}

void AppState::PostEvent(ff::hash_t eventId)
{
	auto weakOwner = weak_from_this();
	ff::GetGameThreadDispatch()->Post([weakOwner, eventId]()
		{
			auto owner = std::dynamic_pointer_cast<AppState>(weakOwner.lock());
			if (owner)
			{
				owner->_appEvents.Notify(eventId);
			}
		});
}

ff::Event<ff::hash_t>& AppState::GetEventHandler()
{
	return _appEvents;
}

ff::ProcessGlobals& AppState::GetProcessGlobals()
{
	return *_processGlobals;
}

ff::AppGlobals& AppState::GetAppGlobals()
{
	return *_globals;
}

ff::XamlGlobalState& AppState::GetXamlGlobals()
{
	return *_xamlGlobals.get();
}

ff::IResources* AppState::GetLevelResources()
{
	if (!_levelResources)
	{
		ff::WaitForHandle(_loadLevelResourcesEvent);
	}

	return _levelResources;
}

const SystemOptions& AppState::GetSystemOptions() const
{
	return _systemOptions;
}

const GameOptions& AppState::GetGameOptions() const
{
	return _gameOptions;
}

void AppState::SetSystemOptions(const SystemOptions& options)
{
	_systemOptions = options;
}

void AppState::SetGameOptions(const GameOptions& options)
{
	_gameOptions = options;
}

ff::Dict AppState::GetDefaultVars() const
{
	ff::ValuePtrT<ff::DictValue> dictValue = ff::GetThisModule().GetValue(Strings::ID_VARS);
	return dictValue.GetValue();
}

void AppState::RegisterGame(IGameService* game)
{
	if (!_games.Contains(game))
	{
		_games.Push(game);

		if (_games.Size() < 2)
		{
			OnActiveGameChanged();
		}
	}
}

void AppState::UnregisterGame(IGameService* game)
{
	_games.DeleteItem(game);

	if (_games.Size() < 2)
	{
		OnActiveGameChanged();
	}
}

const ff::Vector<IGameService*>& AppState::GetGames() const
{
	return _games;
}

IGameService* AppState::GetActiveGame() const
{
	// uhh, just pick the first
	return _games.Size() ? _games[0] : nullptr;
}

void AppState::ShowDebugUI(bool show)
{
	_showDebugUI = show;
}

bool AppState::IsShowingDebugUI() const
{
	return _showDebugUI;
}

bool AppState::OnInitialized(ff::AppGlobals* globals)
{
	InitOptions();
	InitLevelResources();
	InitInputDevices();

	return true;
}

void AppState::OnGameThreadInitialized(ff::AppGlobals* globals)
{
	std::weak_ptr<ff::State> weakThis = weak_from_this();
	_customDebugCookie = globals->GetDebugPageState()->CustomDebugEvent().Add([weakThis]()
		{
			auto owner = std::static_pointer_cast<AppState>(weakThis.lock());
			if (owner)
			{
				owner->ShowDebugUI(!owner->IsShowingDebugUI());
			}
		});

	_xamlGlobals = std::make_shared<ff::XamlGlobalState>(_globals);
	_xamlGlobals->Startup(this, this, ff::String::from_static(L"ApplicationResources.xaml"));
	::RegisterNoesisComponents();

	_globals->SetFullScreen(_systemOptions._fullScreen);
}

void AppState::OnGameThreadShutdown(ff::AppGlobals* globals)
{
	if (_xamlGlobals)
	{
		_xamlGlobals->Shutdown();
		_xamlGlobals = nullptr;
	}

	globals->GetDebugPageState()->CustomDebugEvent().Remove(_customDebugCookie);
}

std::shared_ptr<ff::State> AppState::CreateInitialState(ff::AppGlobals* globals)
{
	std::shared_ptr<ff::States> states = std::make_shared<ff::States>();
	states->AddTop(shared_from_this());
	states->AddTop(std::make_shared<TitleState>(this));
	states->AddTop(_debugState);
	states->AddTop(_xamlGlobals);

	return states;
}

double AppState::GetTimeScale(ff::AppGlobals* globals)
{
	return _debugTimeScale;
}

ff::AdvanceType AppState::GetAdvanceType(ff::AppGlobals* globals)
{
	if (_debugStepOneFrame)
	{
		return ff::AdvanceType::SingleStep;
	}

	if (_debugSteppingFrames)
	{
		return ff::AdvanceType::Stopped;
	}

	return ff::AdvanceType::Running;
}

ff::AutoResourceValue AppState::GetResource(ff::StringRef name)
{
	return ff::AutoResourceValue(ff::GetThisModule().GetResources(), name);
}

ff::ValuePtr AppState::GetValue(ff::StringRef name) const
{
	return ff::GetThisModule().GetValue(name);
}

ff::String AppState::GetString(ff::StringRef name) const
{
	return ff::GetThisModule().GetString(name);
}

void AppState::InitOptions()
{
	ff::Dict dict = GetAppGlobals().GetState(::APP_STATE_NAME);

	ff::ComPtr<ff::IData> systemData = dict.Get<ff::DataValue>(Strings::ID_SYSTEM_OPTIONS);
	if (systemData && systemData->GetSize() == sizeof(SystemOptions))
	{
		_systemOptions = *(SystemOptions*)systemData->GetMem();

		if (_systemOptions._version != SystemOptions::VERSION)
		{
			_systemOptions = SystemOptions();
		}
	}

	ff::ComPtr<ff::IData> gameData = dict.Get<ff::DataValue>(Strings::ID_GAME_OPTIONS);
	if (gameData && gameData->GetSize() == sizeof(GameOptions))
	{
		_gameOptions = *(GameOptions*)gameData->GetMem();

		if (_systemOptions._version != GameOptions::VERSION)
		{
			_gameOptions = GameOptions();
		}
	}
}

void AppState::InitLevelResources()
{
	GetProcessGlobals().GetThreadPool()->AddTask([this]
		{
			_levelResources = LoadLevelResources();
			::SetEvent(_loadLevelResourcesEvent);
		});
}

void AppState::InitInputDevices()
{
	_debugInputResource.Init(L"gameDebugControls");

	_debugInputDevices._keys.Push(_globals->GetKeysDebug());
	_debugInputDevices._mice.Push(_globals->GetPointerDebug());
	_debugNoKeyboardInputDevices._mice = _debugInputDevices._mice;
}

const ff::InputDevices& AppState::GetDebugInputDevices()
{
	return IsShowingDebugUI()
		? _debugNoKeyboardInputDevices
		: _debugInputDevices;
}

ff::ComPtr<ff::IResources> AppState::LoadLevelResources()
{
	ff::Dict dict;
	bool dictLoaded = false;

	ff::String shippedFile = ff::GetExecutableDirectory(ff::String(L"Levels.res.pack"));
	ff::ComPtr<ff::IData> shippedData;

	if (ff::FileExists(shippedFile) && ff::ReadWholeFileMemMapped(shippedFile, &shippedData))
	{
		ff::ComPtr<ff::ISavedData> shippedSavedData;
		if (ff::CreateSavedDataFromMemory(shippedData, shippedData->GetSize(), false, &shippedSavedData))
		{
			ff::ValuePtr savedDictValue = ff::Value::New<ff::SavedDictValue>(shippedSavedData);
			ff::ValuePtr dictValue = savedDictValue->Convert<ff::DictValue>();
			if (dictValue)
			{
				dict = dictValue->GetValue<ff::DictValue>();
				dictLoaded = true;
			}
		}
	}

	if (!dictLoaded)
	{
		ff::String sourceFile = ff::GetExecutableDirectory(ff::String(L"Assets\\Levels\\Levels.res.json"));
		if (ff::FileExists(sourceFile))
		{
			ff::Vector<ff::String> errors;
#ifdef _DEBUG
			dict = ff::LoadResourcesFromFile(sourceFile, ff::GetThisModule().IsDebugBuild(), errors);
#else
			dict = ff::LoadResourcesFromFileCached(sourceFile, ff::GetThisModule().IsDebugBuild(), errors);
#endif
			dictLoaded = true;
		}
	}

	assertRetVal(dictLoaded && dict.Size(), nullptr);

	ff::ComPtr<ff::IResources> resources;
	assertRetVal(ff::CreateResources(&GetAppGlobals(), dict, &resources), nullptr);

	return resources;
}

void AppState::OnActiveGameChanged()
{
	IGameService* gameService = GetActiveGame();
	_debugState->OnActiveGameChanged(gameService);
}
