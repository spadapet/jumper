#pragma once

#include "Resource/ResourceValue.h"

enum class EntityType;
struct EntityCollision;
struct EntityTileCollision;

namespace Strings
{
	extern const ff::StaticString ID_GAME_OPTIONS;
	extern const ff::StaticString ID_SYSTEM_OPTIONS;
	extern const ff::StaticString ID_VARS;
}

namespace GameEvents
{
	extern const ff::hash_t ID_DEBUG_RESTART_GAME;
	extern const ff::hash_t ID_DEBUG_RESTART_LEVEL;
}

namespace InputEvents
{
	extern const ff::hash_t ID_UP;
	extern const ff::hash_t ID_DOWN;
	extern const ff::hash_t ID_LEFT;
	extern const ff::hash_t ID_RIGHT;
	extern const ff::hash_t ID_ACTION;
	extern const ff::hash_t ID_JUMP;
	extern const ff::hash_t ID_RUN;
	extern const ff::hash_t ID_CANCEL;
	extern const ff::hash_t ID_HOME;
	extern const ff::hash_t ID_PAUSE;
	extern const ff::hash_t ID_START;

	// Debug events
	extern const ff::hash_t ID_DEBUG_STEP_ONE_FRAME;
	extern const ff::hash_t ID_DEBUG_CANCEL_STEP_ONE_FRAME;
	extern const ff::hash_t ID_DEBUG_SPEED_SLOW;
	extern const ff::hash_t ID_DEBUG_SPEED_FAST;
	extern const ff::hash_t ID_DEBUG_SHOW_UI;
	extern const ff::hash_t ID_DEBUG_ZOOM_PLAYER_CAMERA;
}

namespace Constants
{
	const ff::FixedInt LEVEL_RENDER_WIDTH = 480_f; // 1920 / 4
	const ff::FixedInt LEVEL_RENDER_HEIGHT = 270_f; // 1080 / 4
	const ff::PointFixedInt LEVEL_RENDER_SIZE(LEVEL_RENDER_WIDTH, LEVEL_RENDER_HEIGHT);

	const ff::FixedInt LEVEL_BUFFER_SCALE = 4_f; // scale to a 1080p buffer, which then gets scaled to the screen

	const ff::FixedInt LEVEL_TILE_WIDTH = 20_f; // Screen is 24 tiles wide
	const ff::FixedInt LEVEL_TILE_HEIGHT = 20_f; // Screen in 13.5 tiles high
	const ff::PointFixedInt LEVEL_TILE_SIZE(LEVEL_TILE_WIDTH, LEVEL_TILE_HEIGHT);

	const ff::FixedInt COLLISION_TILE_WIDTH = 120_f; // 4 columns per screen
	const ff::FixedInt COLLISION_TILE_HEIGHT = 135_f; // 2 rows per screen
	const ff::PointFixedInt COLLISION_TILE_SIZE(COLLISION_TILE_WIDTH, COLLISION_TILE_HEIGHT);

	const ff::FixedInt MAX_TILE_PUSH_SIZE = 10_f;
	const ff::FixedInt MAX_STAIR_SIZE = 5_f;

	const size_t REASONABLE_ENTITY_BUFFER_SIZE = 512;
	const size_t REASONABLE_ENTITY_BUCKET_SIZE = 512;
	const size_t BIG_ENTITY_BUCKET_SIZE = 512;
}

namespace Rendering
{
	enum class Layer : unsigned char
	{
		Unknown,
		DebugGrid,
		LevelBackground,
		LevelTiles,
		Player,

		FirstOverlay, // placeholder

		FirstHighRes, // placeholder

		FirstHighResOverlay, // placeholder

		FirstScreenRes, // placeholder
		ScreenResDebugBoxes,
		ScreenResDebugCollisions,

		FirstScreenResOverlay, // placeholder

		Count
	};

	enum class Shaders : unsigned char
	{
		Normal,

		Count
	};

	enum class Type
	{
		None,
		Layer,
		Player,

		Count
	};
}

namespace Advancing
{
	enum class Type // in priority order
	{
		None,

		StartPass0_Level = 0x00000001,
		Layer,

		StartPass1_Objects = 0x01000001,
		Player,

		// Cameras always update after collisions and must be the last pass
		StartPass2_Camera = 0x02000001,
		Camera,
		DebugCamera,

		PassCount = 3
	};

	enum class State
	{
		None = 0x0,
		Deleting = 0x1,

		FacingRight = 0x10,
		FacingLeft = 0x20,
		FacingUp = 0x40,
		FacingDown = 0x80,

		Standing = 0x100,
		StandingOnRamp = Standing | 0x200,
		InAir = 0x400,
		Jumping = InAir | 0x800,

		Group0 = 0xF,
		Group1 = 0xF0,
		Group2 = 0xF00,
	};
}

namespace EntityEvents
{
	extern const ff::hash_t ID_BEFORE_ENTITY_COLLISIONS;
	extern const ff::hash_t ID_BEFORE_ENTITY_TILE_COLLISIONS;
	extern const ff::hash_t ID_BOUNDING_BOX_CHANGED;
	extern const ff::hash_t ID_DELETE_ENTITY;
	extern const ff::hash_t ID_HIT_BOX_CHANGED;
	extern const ff::hash_t ID_POSITION_CHANGED;
	extern const ff::hash_t ID_POSITION_TO_BOUNDING_BOX_CHANGED;
	extern const ff::hash_t ID_POSITION_TO_HIT_BOX_CHANGED;
	extern const ff::hash_t ID_STATE_CHANGED;
	extern const ff::hash_t ID_TRANSFORM_CHANGED;
	extern const ff::hash_t ID_TYPE_CHANGED;

	struct EntityCollisionsEventArgs : ff::EntityEventArgs
	{
		const EntityCollision* _collisions;
		size_t _count;
	};

	struct EntityTileCollisionsEventArgs : ff::EntityEventArgs
	{
		const EntityTileCollision* _collisions;
		size_t _count;
	};

	struct StateChangedEventArgs : ff::EntityEventArgs
	{
		Advancing::State _oldState;
		Advancing::State _newState;
	};

	struct TypeChangedEventArgs : ff::EntityEventArgs
	{
		EntityType _oldType;
		EntityType _newType;
	};
}
