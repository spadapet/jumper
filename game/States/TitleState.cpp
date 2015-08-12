#include "pch.h"
#include "Core/Globals.h"
#include "Globals/AppGlobals.h"
#include "Graph/RenderTarget/RenderTargetWindow.h"
#include "Services/AppService.h"
#include "States/PlayGameState.h"
#include "States/TitleState.h"
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"
#include "UI/XamlViewState.h"

TitleState::TitleState(IAppService* appService)
	: _appService(appService)
{
	auto view = _appService->GetXamlGlobals().CreateView(ff::String::from_static(L"TitlePage.xaml"));
	_viewState = _appService->GetXamlGlobals().CreateViewState(view, _appService->GetAppGlobals().GetTarget());

	Noesis::FrameworkElement* root = _viewState->GetView()->GetContent();
	Noesis::Button* playGameButton = root->FindName<Noesis::Button>("playGameButton");

	playGameButton->Click() += Noesis::MakeDelegate(this, &TitleState::OnClickPlayGame);
}

std::shared_ptr<ff::State> TitleState::Advance(ff::AppGlobals* globals)
{
	ff::State::Advance(globals);
	return _pendingState;
}

size_t TitleState::GetChildStateCount()
{
	return 1;
}

ff::State* TitleState::GetChildState(size_t index)
{
	return _viewState.get();
}

void TitleState::OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args)
{
	_pendingState = std::make_shared<PlayGameState>(_appService, _appService->GetGameOptions());
}
