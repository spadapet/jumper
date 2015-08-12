#include "pch.h"
#include "Globals/AppGlobals.h"
#include "Graph/RenderTarget/RenderTargetWindow.h"
#include "Services/AppService.h"
#include "Services/GameService.h"
#include "States/DebugState.h"
#include "UI/DebugControl.h"
#include "UI/DebugPage.h"
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"
#include "UI/XamlViewState.h"

DebugState::DebugState(IAppService* appService)
	: _appService(appService)
	, _showing(false)
{
}

DebugPage* DebugState::GetDebugPage() const
{
	return _debugPage;
}

void DebugState::OnActiveGameChanged(IGameService* game)
{
	if (_debugPage)
	{
		Vars* vars = game ? &game->GetCheatingVars() : nullptr;
		_debugPage->GetVarsControl()->Reset(vars);
	}
}

std::shared_ptr<ff::State> DebugState::Advance(ff::AppGlobals* globals)
{
	if (_appService->IsShowingDebugUI() != _showing)
	{
		_showing = !_showing;

		if (_showing)
		{
			if (!_debugPage)
			{
				_debugPage = *new DebugPage(_appService);
				OnActiveGameChanged(_appService->GetActiveGame());
			}

			if (!_viewState)
			{
				ff::XamlGlobalState& uiGlobals = _appService->GetXamlGlobals();
				_viewState = uiGlobals.CreateViewState(uiGlobals.CreateView(_debugPage), _appService->GetAppGlobals().GetTarget());
			}

			_viewState->GetView()->SetFocus(true);
		}
	}

	return ff::State::Advance(globals);
}

size_t DebugState::GetChildStateCount()
{
	return (_showing && _viewState) ? 1 : 0;
}

ff::State* DebugState::GetChildState(size_t index)
{
	return _viewState.get();
}
