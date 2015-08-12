#include "pch.h"
#include "App.xaml.h"
#include "MainUtilInclude.h"
#include "Module/Module.h"
#include "String/StringUtil.h"

// {F09FDF18-3449-458E-8686-9A4BA367E3C4}
static const GUID MODULE_ID = { 0xf09fdf18, 0x3449, 0x458e, { 0x86, 0x86, 0x9a, 0x4b, 0xa3, 0x67, 0xe3, 0xc4 } };
static ff::StaticString MODULE_NAME(L"Jumper");
static ff::ModuleFactory RegisterThisModule(MODULE_NAME, MODULE_ID, ff::GetMainInstance, ff::GetModuleStartup);

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ args)
{
	ff::SetMainModule(MODULE_NAME, MODULE_ID, ff::GetMainInstance());

	auto callbackFunc = [](Windows::UI::Xaml::ApplicationInitializationCallbackParams^ args)
	{
		auto app = ref new ::Jumper::App();
	};

	auto callback = ref new Windows::UI::Xaml::ApplicationInitializationCallback(callbackFunc);
	Windows::UI::Xaml::Application::Start(callback);

	return 0;
}
