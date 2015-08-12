#pragma once

namespace ff
{
	class IInputEventsProvider;
}

struct UpdateInputComponent : public ff::Component
{
	UpdateInputComponent();

	ff::IInputEventsProvider* _provider;
};
