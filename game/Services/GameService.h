#pragma once

class IAppService;
class Player;
class Vars;
struct GameOptions;

class IGameService
{
public:
	virtual IAppService* GetAppService() const = 0;
	virtual size_t GetPlayerCount() const = 0;
	virtual Player* GetPlayer(size_t index) = 0;
	virtual const GameOptions& GetGameOptions() const = 0;
	virtual const Vars& GetVars() const = 0;
	virtual Vars& GetCheatingVars() = 0;
};
