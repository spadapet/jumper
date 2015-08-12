#pragma once

#include "State/State.h"

namespace ff
{
	class XamlViewState;
}

class DebugPage;
class IAppService;
class IGameService;

class DebugState : public ff::State
{
public:
	DebugState(IAppService* appService);

	DebugPage* GetDebugPage() const;
	void OnActiveGameChanged(IGameService* game);

	virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
	virtual size_t GetChildStateCount() override;
	virtual State* GetChildState(size_t index) override;

private:
	IAppService* _appService;
	Noesis::Ptr<DebugPage> _debugPage;
	std::shared_ptr<ff::XamlViewState> _viewState;
	bool _showing;
};
