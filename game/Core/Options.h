#pragma once

enum class GameType
{
	Adventure,
};

enum class GamePlayers
{
	Single,
};

enum class GameDifficulty
{
	Normal,
};

struct GameOptions
{
	GameOptions();
	GameOptions(const GameOptions& rhs);
	GameOptions& operator=(const GameOptions& rhs);

	static const int VERSION = 1;

	int _version;
	GameType _type;
	GamePlayers _players;
	GameDifficulty _difficulty;
};

struct SystemOptions
{
	SystemOptions();
	SystemOptions(const SystemOptions& rhs);
	SystemOptions& operator=(const SystemOptions& rhs);

	static const int VERSION = 1;

	int _version;
	bool _fullScreen;
	bool _soundOn;
	bool _musicOn;
	ff::FixedInt _soundVolume;
	ff::FixedInt _musicVolume;
};
