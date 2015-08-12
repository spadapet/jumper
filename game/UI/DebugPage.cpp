#include "pch.h"
#include "UI/DebugControl.h"
#include "UI/DebugPage.h"

DebugPage::DebugPage(IAppService* appService)
	: _appService(appService)
	, _debugControl(nullptr)
{
	Noesis::GUI::LoadComponent(this, "DebugPage.xaml");
	_debugControl = FindName<DebugControl>("debugControl");
}

IAppService* DebugPage::GetAppService()
{
	return _appService;
}

DebugControl* DebugPage::GetVarsControl() const
{
	return _debugControl;
}

bool DebugPage::ConnectEvent(Noesis::BaseComponent* source, const char* event, const char* handler)
{
	return false;
}

NS_IMPLEMENT_REFLECTION(DebugPage)
{
	NsMeta<Noesis::TypeId>("Jumper.DebugPage");
}
