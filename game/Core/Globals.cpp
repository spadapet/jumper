#include "pch.h"
#include "Core/Globals.h"

const ff::StaticString Strings::ID_GAME_OPTIONS(L"GameOptions");
const ff::StaticString Strings::ID_SYSTEM_OPTIONS(L"SystemOptions");
const ff::StaticString Strings::ID_VARS(L"Vars");

const ff::hash_t EntityEvents::ID_BEFORE_ENTITY_COLLISIONS = ff::HashFunc(L"BeforeEntityCollisions");
const ff::hash_t EntityEvents::ID_BEFORE_ENTITY_TILE_COLLISIONS = ff::HashFunc(L"BeforeEntityTileCollisions");
const ff::hash_t EntityEvents::ID_BOUNDING_BOX_CHANGED = ff::HashFunc(L"BoundingBoxChanged");
const ff::hash_t EntityEvents::ID_DELETE_ENTITY = ff::HashFunc(L"DeleteEntity");
const ff::hash_t EntityEvents::ID_HIT_BOX_CHANGED = ff::HashFunc(L"HitBoxChanged");
const ff::hash_t EntityEvents::ID_POSITION_CHANGED = ff::HashFunc(L"PositionChanged");
const ff::hash_t EntityEvents::ID_POSITION_TO_BOUNDING_BOX_CHANGED = ff::HashFunc(L"PositionToBoundingBoxChanged");
const ff::hash_t EntityEvents::ID_POSITION_TO_HIT_BOX_CHANGED = ff::HashFunc(L"PositionToHitBoxChanged");
const ff::hash_t EntityEvents::ID_STATE_CHANGED = ff::HashFunc(L"StateChanged");
const ff::hash_t EntityEvents::ID_TRANSFORM_CHANGED = ff::HashFunc(L"TransformChanged");
const ff::hash_t EntityEvents::ID_TYPE_CHANGED = ff::HashFunc(L"TypeChanged");

const ff::hash_t GameEvents::ID_DEBUG_RESTART_GAME = ff::HashFunc(L"DebugEvent.RestartGame");
const ff::hash_t GameEvents::ID_DEBUG_RESTART_LEVEL = ff::HashFunc(L"DebugEvent.RestartLevel");

const ff::hash_t InputEvents::ID_UP = ff::HashFunc(L"up");
const ff::hash_t InputEvents::ID_DOWN = ff::HashFunc(L"down");
const ff::hash_t InputEvents::ID_LEFT = ff::HashFunc(L"left");
const ff::hash_t InputEvents::ID_RIGHT = ff::HashFunc(L"right");
const ff::hash_t InputEvents::ID_ACTION = ff::HashFunc(L"action");
const ff::hash_t InputEvents::ID_JUMP = ff::HashFunc(L"jump");
const ff::hash_t InputEvents::ID_RUN = ff::HashFunc(L"run");
const ff::hash_t InputEvents::ID_CANCEL = ff::HashFunc(L"cancel");
const ff::hash_t InputEvents::ID_HOME = ff::HashFunc(L"home");
const ff::hash_t InputEvents::ID_PAUSE = ff::HashFunc(L"pause");
const ff::hash_t InputEvents::ID_START = ff::HashFunc(L"start");

const ff::hash_t InputEvents::ID_DEBUG_STEP_ONE_FRAME = ff::HashFunc(L"stepOneFrame");
const ff::hash_t InputEvents::ID_DEBUG_CANCEL_STEP_ONE_FRAME = ff::HashFunc(L"cancelStepOneFrame");
const ff::hash_t InputEvents::ID_DEBUG_SPEED_SLOW = ff::HashFunc(L"speedSlow");
const ff::hash_t InputEvents::ID_DEBUG_SPEED_FAST = ff::HashFunc(L"speedFast");
const ff::hash_t InputEvents::ID_DEBUG_SHOW_UI = ff::HashFunc(L"showDebugUI");
const ff::hash_t InputEvents::ID_DEBUG_ZOOM_PLAYER_CAMERA = ff::HashFunc(L"zoomPlayerCamera");
