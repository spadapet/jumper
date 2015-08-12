#pragma once

#include "State/State.h"

namespace ff
{
	class EntityDomain;
	class IInputMapping;
	class XamlViewState;
}

class IAppService;
class IGameService;

class TitleState : public ff::State
{
public:
	TitleState(IAppService* appService);

	virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
	virtual size_t GetChildStateCount() override;
	virtual State* GetChildState(size_t index) override;

private:
	void OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args);

	IAppService* _appService;
	std::shared_ptr<ff::XamlViewState> _viewState;
	std::shared_ptr<ff::State> _pendingState;
};
