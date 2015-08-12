#pragma once

class Player;

struct PlayerComponent : public ff::Component
{
	PlayerComponent();

	Player* _player;
};
