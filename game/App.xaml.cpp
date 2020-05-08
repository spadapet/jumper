﻿#include "pch.h"
#include "App.xaml.h"
#include "States/AppState.h"
#include "Thread/ThreadDispatch.h"
#include "UI/FailurePage.xaml.h"

Jumper::App::App()
{
	InitializeComponent();
}

Jumper::App::~App()
{
	_globals.Shutdown();
	_processGlobals.Shutdown();
}

void Jumper::App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args)
{
	if (!_processGlobals.IsValid())
	{
		_processGlobals.Startup();

		Platform::WeakReference weakThis(this);
		ff::GetMainThreadDispatch()->Post([weakThis]()
			{
				weakThis.Resolve<App>()->InitializeGlobals();
			});
	}
}

void Jumper::App::InitializeGlobals()
{
	auto panel = ref new Windows::UI::Xaml::Controls::SwapChainPanel();
	auto page = ref new Windows::UI::Xaml::Controls::Page();
	page->Content = panel;

	auto window = Windows::UI::Xaml::Window::Current;
	window->Content = page;

	_appState = std::make_shared<AppState>(&_processGlobals, &_globals);

	if (!_globals.Startup(ff::AppGlobalsFlags::All, panel, _appState.get()))
	{
		window->Content = ref new FailurePage(&_globals);
	}

	window->Activate();
}
