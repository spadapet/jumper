#include "pch.h"
#include "Systems/SystemBase.h"
#include "Types/Timer.h"

SystemBase::SystemBase()
	: _advanceTime(0)
	, _renderQueueTime(0)
{
}

SystemBase::~SystemBase()
{
}

void SystemBase::AdvanceHelper(ff::EntityDomain* domain, const ff::Vector<std::shared_ptr<SystemBase>>& systems)
{
	INT64 startTime = ff::Timer::GetCurrentRawTime();

	for (const std::shared_ptr<SystemBase>& i : systems)
	{
		i->Advance(domain);

		INT64 endTime = ff::Timer::GetCurrentRawTime();
		i->SetAdvanceTime(endTime - startTime);
		startTime = endTime;
	}
}

void SystemBase::Advance(ff::EntityDomain* domain)
{
}

void SystemBase::Render(ff::EntityDomain* domain, IRenderQueueService* render)
{
}

void SystemBase::SetName(ff::StringRef value)
{
	_name = value;
}

ff::StringRef SystemBase::GetName() const
{
	return _name;
}

void SystemBase::SetAdvanceTime(int64_t value)
{
	_advanceTime = value;
}

int64_t SystemBase::GetAdvanceTime() const
{
	return _advanceTime;
}

void SystemBase::SetRenderQueueTime(int64_t value)
{
	_renderQueueTime = value;
}

int64_t SystemBase::GetRenderQueueTime() const
{
	return _renderQueueTime;
}
