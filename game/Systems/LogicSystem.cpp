#include "pch.h"
#include "Components/EntityTypeComponent.h"
#include "Components/InputStateComponent.h"
#include "Components/LogicStateComponent.h"
#include "Components/PositionComponent.h"
#include "Components/VelocityComponent.h"
#include "Core/CameraView.h"
#include "Services/CameraService.h"
#include "Services/CollisionService.h"
#include "Services/EntityLogicService.h"
#include "Services/LevelService.h"
#include "Systems/LogicSystem.h"

static VelocityComponent NULL_VELOCITY;

struct LogicSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS2(LogicSystemEntry)

	LogicStateComponent* _state;
	PositionComponent* _pos;
	VelocityComponent* _vel;

	Advancing::Type _type;
	IEntityMoveLogic* _logic;
};

BEGIN_ENTRY_COMPONENTS(LogicSystemEntry)
HAS_COMPONENT(LogicStateComponent)
HAS_OPTIONAL_COMPONENT(PositionComponent)
HAS_OPTIONAL_COMPONENT(VelocityComponent)
END_ENTRY_COMPONENTS(LogicSystemEntry)

LogicSystemEntry::LogicSystemEntry()
	: _type(Advancing::Type::None)
	, _logic(nullptr)
{
}

LogicSystem::LogicSystem(ILevelService* levelService)
	: _bucket(levelService->GetDomain().GetBucket<LogicSystemEntry>())
	, _levelService(levelService)
{
	_updateEntities.Reserve(Constants::REASONABLE_ENTITY_BUFFER_SIZE);

	for (ff::Vector<LogicSystemEntry*>& entries : _updatePasses)
	{
		entries.Reserve(Constants::REASONABLE_ENTITY_BUFFER_SIZE);
	}
}

void LogicSystem::Advance(ff::EntityDomain* domain)
{
	BuildUpdateEntries();
	AdvancePasses(0, _updatePasses.size() - 1);
}

void LogicSystem::AdvanceAfterCollisions()
{
	AdvancePasses(_updatePasses.size() - 1, _updatePasses.size());
}

static bool CompareLogicSystemEntry(const LogicSystemEntry* lhs, const LogicSystemEntry* rhs)
{
	return lhs->_state->_type < rhs->_state->_type;
}

void LogicSystem::BuildUpdateEntries()
{
	_updateEntities.Clear();

	for (ff::Vector<LogicSystemEntry*>& entries : _updatePasses)
	{
		entries.Clear();
	}

	// Collect entities that need to be updated based on the camera's view
	{
		auto entityCollector = _updateEntities.GetCollector();
		const CameraView& cameraView = _levelService->GetCameraService()->GetAdvanceCameraView();
		_levelService->GetCollisionService()->EntityHitTest(cameraView._worldUpdateBounds, EntityType::Any, entityCollector, false);
		_levelService->GetUpdateEntities(entityCollector);
	}

	for (ff::Entity entity : _updateEntities)
	{
		LogicSystemEntry* entry = _bucket->GetEntry(entity);
		if (entry && !entry->_state->IsDeleting())
		{
			unsigned int pass = ((unsigned int)entry->_state->_type & 0xFF000000) >> 24;
			_updatePasses[pass].Push(entry);
		}
	}

	for (ff::Vector<LogicSystemEntry*>& entries : _updatePasses)
	{
		std::sort(entries.begin(), entries.end(), ::CompareLogicSystemEntry);
	}
}

void LogicSystem::AdvancePasses(size_t startPass, size_t endPass)
{
	for (size_t i = startPass; i < endPass; i++)
	{
		const ff::Vector<LogicSystemEntry*>& passEntries = _updatePasses[i];

		for (LogicSystemEntry* entry : passEntries)
		{
			if (entry->_type != entry->_state->_type || entry->_logic == nullptr)
			{
				entry->_type = entry->_state->_type;
				entry->_logic = _levelService->GetEntityLogicService()->GetMoveLogic(entry->_type);
				assert(entry->_logic != nullptr);
			}

			if (!entry->_state->IsDeleting())
			{
				entry->_state->_stateCounter++;
				entry->_logic->UpdateState(entry->_entity, *entry->_state);
			}
		}

		// Everything decides where to move before anything actually moves
		for (LogicSystemEntry* entry : passEntries)
		{
			if (entry->_vel && entry->_pos && !entry->_state->IsDeleting())
			{
				entry->_logic->UpdateVelocity(entry->_entity, *entry->_state, *entry->_vel, *entry->_pos);
			}
		}

		// Everything moves all at once, before collisions are processed
		for (LogicSystemEntry* entry : passEntries)
		{
			if (entry->_pos && !entry->_state->IsDeleting())
			{
				entry->_logic->UpdatePosition(entry->_entity, *entry->_state, entry->_vel ? *entry->_vel : ::NULL_VELOCITY, *entry->_pos);
			}
		}
	}
}

LogicSystemAfterCollisions::LogicSystemAfterCollisions(std::shared_ptr<LogicSystem> logicSystem)
	: _logicSystem(logicSystem)
{
}

void LogicSystemAfterCollisions::Advance(ff::EntityDomain* domain)
{
	_logicSystem->AdvanceAfterCollisions();
}
