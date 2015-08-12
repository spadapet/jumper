#include "pch.h"
#include "Core/Options.h"

GameOptions::GameOptions()
	: _version(GameOptions::VERSION)
	, _type(GameType::Adventure)
	, _players(GamePlayers::Single)
	, _difficulty(GameDifficulty::Normal)
{
}

GameOptions::GameOptions(const GameOptions& rhs)
	: GameOptions()
{
	*this = rhs;
}

GameOptions& GameOptions::operator=(const GameOptions& rhs)
{
	std::memcpy(this, &rhs, sizeof(rhs));
	return *this;
}

SystemOptions::SystemOptions()
	: _version(SystemOptions::VERSION)
	, _fullScreen(false)
	, _soundOn(true)
	, _musicOn(true)
	, _soundVolume(1)
	, _musicVolume(1)
{
}

SystemOptions::SystemOptions(const SystemOptions& rhs)
	: SystemOptions()
{
	*this = rhs;
}

SystemOptions& SystemOptions::operator=(const SystemOptions& rhs)
{
	std::memcpy(this, &rhs, sizeof(rhs));
	return *this;
}
