#include "pch.h"
#include "Components/InputStateComponent.h"
#include "Components/UpdateInputComponent.h"
#include "Core/Globals.h"
#include "Input/InputMapping.h"
#include "Systems/InputEventsSystem.h"

struct InputEventsSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS()

	UpdateInputComponent* _input;
	InputStateComponent* _state;
};

BEGIN_ENTRY_COMPONENTS(InputEventsSystemEntry)
HAS_COMPONENT(UpdateInputComponent)
HAS_COMPONENT(InputStateComponent)
END_ENTRY_COMPONENTS(InputEventsSystemEntry)

InputEventsSystem::InputEventsSystem(ff::EntityDomain* domain)
	: _bucket(domain->GetBucket<InputEventsSystemEntry>())
{
}

void InputEventsSystem::Advance(ff::EntityDomain* domain)
{
	for (InputEventsSystemEntry& entry : _bucket->GetEntries())
	{
		UpdateInputComponent& input = *entry._input;
		InputStateComponent& state = *entry._state;
		state.Reset();

		const ff::IInputEvents* events = input._provider ? input._provider->GetInputEvents() : nullptr;
		const ff::InputDevices& devices = input._provider ? input._provider->GetInputDevices() : ff::InputDevices::Null();
		if (events)
		{
			state._pressDir = ff::PointFixedInt(
				events->GetAnalogValue(devices, InputEvents::ID_RIGHT) - events->GetAnalogValue(devices, InputEvents::ID_LEFT),
				events->GetAnalogValue(devices, InputEvents::ID_DOWN) - events->GetAnalogValue(devices, InputEvents::ID_UP));

			state._pressingJump = events->GetDigitalValue(devices, InputEvents::ID_JUMP) != 0;
			state._pressedJump = events->HasStartEvent(InputEvents::ID_JUMP);

			state._pressingRun = events->GetDigitalValue(devices, InputEvents::ID_RUN) != 0;
			state._pressedRun = events->HasStartEvent(InputEvents::ID_RUN);
		}
	}
}
