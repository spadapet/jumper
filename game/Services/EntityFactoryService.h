#pragma once

namespace ff
{
	class IInputEventsProvider;
}

class ILevelService;
class Player;
enum class DebugCameraType;
struct LevelData;

class IEntityFactoryService
{
public:
	virtual ff::Entity CreateCamera(ff::Entity followEntity) = 0;
	virtual ff::Entity CreateCamera(ff::Entity followEntity, DebugCameraType type) = 0;
	virtual ff::Entity CreateLayer(const LevelData& levelData, size_t layer) = 0;
	virtual ff::Entity CreateLevel(const LevelData& levelData) = 0;
	virtual ff::Entity CreatePlayer(Player* player, ff::PointFixedInt pos) = 0;
};

class EntityFactoryService : public IEntityFactoryService
{
public:
	EntityFactoryService(ILevelService* levelService);
	~EntityFactoryService();

	virtual ff::Entity CreateCamera(ff::Entity followEntity) override;
	virtual ff::Entity CreateCamera(ff::Entity followEntity, DebugCameraType type) override;
	virtual ff::Entity CreateLayer(const LevelData& levelData, size_t layer) override;
	virtual ff::Entity CreateLevel(const LevelData& levelData) override;
	virtual ff::Entity CreatePlayer(Player* player, ff::PointFixedInt pos) override;

private:
	void SetDefaultPlayerInputForces(ff::Entity entity);

	ILevelService* _levelService;
	ff::Entity _blankCollisionEntity;
	ff::Entity _blankEntity;
};
