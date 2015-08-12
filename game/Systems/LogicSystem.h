#pragma once

#include "Core/Globals.h"
#include "Systems/SystemBase.h"

class ILevelService;
struct LogicSystemEntry;

class LogicSystem : public SystemBase
{
public:
	LogicSystem(ILevelService* levelService);

	// SystemBase
	virtual void Advance(ff::EntityDomain* domain) override;

	void AdvanceAfterCollisions();

private:
	void BuildUpdateEntries();
	void AdvancePasses(size_t startPass, size_t endPass);

	typedef void (LogicSystem::* UpdateFunc)(ff::EntityDomain* domain, const LogicSystemEntry& entry);

	ff::IEntityBucket<LogicSystemEntry>* _bucket;
	ff::Vector<ff::Entity> _updateEntities;
	std::array<ff::Vector<LogicSystemEntry*>, (size_t)Advancing::Type::PassCount> _updatePasses;
	ILevelService* _levelService;
};

class LogicSystemAfterCollisions : public SystemBase
{
public:
	LogicSystemAfterCollisions(std::shared_ptr<LogicSystem> logicSystem);

	// SystemBase
	virtual void Advance(ff::EntityDomain* domain) override;

private:
	std::shared_ptr<LogicSystem> _logicSystem;
};
