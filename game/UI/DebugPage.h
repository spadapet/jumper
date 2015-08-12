#pragma once

class DebugControl;
class IAppService;

class DebugPage : public Noesis::Grid
{
public:
	DebugPage(IAppService* appService);
	IAppService* GetAppService();
	DebugControl* GetVarsControl() const;

	virtual bool ConnectEvent(Noesis::BaseComponent* source, const char* event, const char* handler) override;

private:
	IAppService* _appService;
	DebugControl* _debugControl;

	NS_DECLARE_REFLECTION(DebugPage, Noesis::Grid);
};
