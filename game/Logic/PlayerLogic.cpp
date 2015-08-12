#include "pch.h"
#include "Components/HitBoxComponent.h"
#include "Components/InputForcesComponent.h"
#include "Components/InputStateComponent.h"
#include "Components/LogicStateComponent.h"
#include "Components/PositionComponent.h"
#include "Components/VelocityComponent.h"
#include "Core/Globals.h"
#include "Core/Helpers.h"
#include "Core/TileSet.h"
#include "Core/Vars.h"
#include "Logic/LogicHelpers.h"
#include "Logic/PlayerLogic.h"
#include "Services/CollisionService.h"
#include "Services/GameService.h"
#include "Services/LevelService.h"

struct PlayerLogicEntry : ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS2(PlayerLogicEntry)

	void SetState(Advancing::State state);
	ff::RectFixedInt HitLeftEdge() const;
	ff::RectFixedInt HitTopEdge() const;
	ff::RectFixedInt HitRightEdge() const;
	ff::RectFixedInt HitBottomEdge() const;

	LogicStateComponent* _state;
	InputStateComponent* _input;
	InputForcesComponent* _forces;
	VelocityComponent* _vel;
	PositionComponent* _pos;
	HitBoxComponent* _hit;

	bool _boostJump;
};

BEGIN_ENTRY_COMPONENTS(PlayerLogicEntry)
HAS_COMPONENT(LogicStateComponent)
HAS_COMPONENT(InputStateComponent)
HAS_COMPONENT(InputForcesComponent)
HAS_COMPONENT(VelocityComponent)
HAS_COMPONENT(PositionComponent)
HAS_COMPONENT(HitBoxComponent)
END_ENTRY_COMPONENTS(PlayerLogicEntry)

PlayerLogicEntry::PlayerLogicEntry()
	: _boostJump(false)
{
}

void PlayerLogicEntry::SetState(Advancing::State state)
{
	_state->SetState(_entity, state);
	_boostJump = _state->HasAll(Advancing::State::Jumping);

	if (_state->HasAll(Advancing::State::Standing))
	{
		_vel->_posVelocity.y = std::min(0_f, _vel->_posVelocity.y);
	}
}

ff::RectFixedInt PlayerLogicEntry::HitLeftEdge() const
{
	return _hit->_box.LeftEdge().Deflate(0, Constants::MAX_STAIR_SIZE, 0, Constants::MAX_STAIR_SIZE);
}

ff::RectFixedInt PlayerLogicEntry::HitTopEdge() const
{
	return _hit->_box.TopEdge().Deflate(Constants::MAX_STAIR_SIZE, 0, Constants::MAX_STAIR_SIZE, 0);
}

ff::RectFixedInt PlayerLogicEntry::HitRightEdge() const
{
	return _hit->_box.RightEdge().Deflate(0, Constants::MAX_STAIR_SIZE, 0, Constants::MAX_STAIR_SIZE);
}

ff::RectFixedInt PlayerLogicEntry::HitBottomEdge() const
{
	return _hit->_box.BottomEdge().Deflate(Constants::MAX_STAIR_SIZE, 0, Constants::MAX_STAIR_SIZE, 0);
}

PlayerLogic::PlayerLogic(ILevelService* levelService)
	: _levelService(levelService)
	, _bucket(levelService->GetDomain().GetBucket<PlayerLogicEntry>())
{
}

void PlayerLogic::UpdateState(ff::Entity entity, LogicStateComponent& state)
{
	PlayerLogicEntry* entry = _bucket->GetEntry(entity);

	if (state._state == Advancing::State::None)
	{
		entry->SetState(Advancing::State::Standing);
	}
	else if (state.HasAll(Advancing::State::Standing))
	{
		if (entry->_input->_pressedJump)
		{
			entry->SetState(Advancing::State::Jumping);
		}
	}
}

void PlayerLogic::UpdateVelocity(ff::Entity entity, LogicStateComponent& state, VelocityComponent& velocity, const PositionComponent& position)
{
	PlayerLogicEntry* entry = _bucket->GetEntry(entity);
	const InputStateComponent& input = *entry->_input;
	const InputForcesComponent& forces = *entry->_forces;

	bool inAir = state.HasAll(Advancing::State::InAir);
	ff::PointFixedInt pressDir = forces.GetPressDir(input._pressDir);
	ff::PointFixedInt gravity = forces.GetGravity(_levelService->GetGameService()->GetVars().GetLevelGravity());
	ff::PointFixedInt maxVelocity = forces.GetMaxVelocity(input._pressingRun, false, false);

	// Slow down
	if (pressDir.IsNull())
	{
		velocity._posVelocity = LogicHelpers::AdjustTowardsLimit(
			velocity._posVelocity,
			ff::PointFixedInt::Zeros(),
			forces.GetFriction(!inAir, false, false));
	}

	// Moving
	velocity._posVelocity += pressDir * forces.GetMoveForce(!inAir);

	// Falling/jumping
	entry->_boostJump = entry->_boostJump && input._pressingJump;
	velocity._posVelocity -= forces.GetJumpVelocity(input._pressingRun, state._stateCounter, gravity.y) * (int)inAir * (int)entry->_boostJump;
	velocity._posVelocity += gravity * (int)inAir;
	velocity._posVelocity = LogicHelpers::Clamp(velocity._posVelocity, ff::RectFixedInt(-maxVelocity, maxVelocity));
}

// Figures out the Y coordinate of a ramp hit, given the top of bottom edge of a hit box
template<TileHitRectType UpType, TileHitRectType DownType>
ff::FixedInt AdjustHitVertical(const TileCollision& tc, ff::FixedInt x, ff::FixedInt y)
{
	const TileHitRect* tileHitRect = tc._tileHitRect;
	const TileHitRectType rampTypes = Helpers::CombineFlags(UpType, DownType);

	if (Helpers::HasAllFlags(tileHitRect->_type, TileHitRectType::RampSibling))
	{
		for (const TileHitRect& checkHitRect : tc._tile->_hitRects)
		{
			if (Helpers::HasAnyFlags(checkHitRect._type, rampTypes))
			{
				tileHitRect = &checkHitRect;
				break;
			}
		}
	}

	if (Helpers::HasAnyFlags(tileHitRect->_type, rampTypes))
	{
		ff::PointFixedInt p1, p2;
		if (Helpers::HasAnyFlags(tileHitRect->_type, UpType))
		{
			p1 = tileHitRect->_rect.BottomLeft();
			p2 = tileHitRect->_rect.TopRight();
		}
		else
		{
			p1 = tileHitRect->_rect.TopLeft();
			p2 = tileHitRect->_rect.BottomRight();
		}

		ff::PointFixedInt offset = tc._worldRect.TopLeft() - tc._tileRect.TopLeft();
		p1 += offset;
		p2 += offset;

		if (x <= p1.x)
		{
			y = p1.y;
		}
		else if (x >= p2.x)
		{
			y = p2.y;
		}
		else
		{
			y = (x - p1.x) * (p2.y - p1.y) / (p2.x - p1.x) + p1.y;
		}
	}

	return y;
}

ff::FixedInt AdjustHitBottom(const TileCollision& tc, const ff::RectFixedInt& bottomEdge)
{
	return ::AdjustHitVertical<TileHitRectType::RampUp, TileHitRectType::RampDown>(tc, bottomEdge.CenterX(), tc._worldRect.top);
}

ff::FixedInt AdjustHitTop(const TileCollision& tc, const ff::RectFixedInt& topEdge)
{
	return ::AdjustHitVertical<TileHitRectType::RampUp, TileHitRectType::RampDown>(tc, topEdge.CenterX(), tc._worldRect.bottom);
}

void PlayerLogic::UpdatePosition(ff::Entity entity, LogicStateComponent& state, const VelocityComponent& velocity, PositionComponent& position)
{
	ff::PointFixedInt offset = velocity._posVelocity;

	if (state.HasAll(Advancing::State::Standing))
	{
		PlayerLogicEntry* entry = _bucket->GetEntry(entity);
		ff::RectFixedInt hitBox = entry->_hit->_box.Offset(offset).Deflate(Constants::MAX_STAIR_SIZE, 0_f);
		ff::RectFixedInt bottomEdge = hitBox.BottomEdge();
		hitBox.bottom += Constants::MAX_TILE_PUSH_SIZE;

		TileType tileTypes = TileType::None;
		TileHitRectType hitRectTypes = TileHitRectType::None;
		ff::Vector<TileCollision, 32> collisions;

		if (_levelService->GetCollisionService()->TileHitTest(hitBox, tileTypes, hitRectTypes, collisions.GetCollector()) &&
			Helpers::HasAllFlags(hitRectTypes, TileHitRectType::Solid))
		{
			ff::FixedInt topHit = 0;
			const TileHitRect* didHitRamp = nullptr;
			bool didHit = false;

			for (const TileCollision& tc : collisions)
			{
				if (tc._tileHitRect->IsSolid())
				{
					ff::FixedInt top = ::AdjustHitBottom(tc, bottomEdge);
					ff::FixedInt distance = std::abs(top - bottomEdge.top);

					if (distance <= Constants::MAX_STAIR_SIZE || (distance <= Constants::MAX_TILE_PUSH_SIZE && tc._tileHitRect->IsFootRamp()))
					{
						if (tc._tileHitRect->IsFootRamp() && tc._worldRect.left <= position._pos.x && tc._worldRect.right >= position._pos.x)
						{
							// Ramps at the X-center always win
							topHit = top;
							didHit = true;
							didHitRamp = tc._tileHitRect;
							break;
						}
						else
						{
							topHit = didHit ? std::min(topHit, top) : top;
							didHit = true;
						}
					}
				}
			}

			if (didHit)
			{
				offset.y += topHit - bottomEdge.top;
				entry->SetState(didHitRamp ? Advancing::State::StandingOnRamp : Advancing::State::Standing);

				if ((didHitRamp && didHitRamp->HasAll(TileHitRectType::RampUp) && offset.x > 0_f) ||
					(didHitRamp && didHitRamp->HasAll(TileHitRectType::RampDown) && offset.x < 0_f))
				{
					// Going uphill, slow down
					ff::FixedInt offsetLength = std::sqrt(offset.LengthSquared());
					ff::FixedInt velocityLength = std::sqrt(velocity._posVelocity.LengthSquared());
					ff::FixedInt angleAdjust = std::cos(std::atan2((float)didHitRamp->_rect.Height(), (float)didHitRamp->_rect.Width()));

					if (offsetLength != 0_f)
					{
						offset = offset * velocityLength * angleAdjust / offsetLength;
					}
				}
				else
				{
					// Going downhill will automatically seem faster since offset.y is changed while offset.x stays the same
				}
			}
			else
			{
				entry->SetState(Advancing::State::InAir);
			}
		}
		else
		{
			entry->SetState(Advancing::State::InAir);
		}
	}

	position.Offset(entity, offset);
}

void PlayerLogic::HandleTileCollisions(ff::Entity entity, LogicStateComponent& state, const EntityTileCollision* collisions, size_t collisionCount, TileType collisionTypes, TileHitRectType hitRectTypes)
{
	PlayerLogicEntry* entry = _bucket->GetEntry(entity);
	ff::RectType<ff::RectFixedInt> edges(entry->HitLeftEdge(), entry->HitTopEdge(), entry->HitRightEdge(), entry->HitBottomEdge());
	ff::RectType<TileHitRectType> edgeHitRectTypes = ff::RectType<TileHitRectType>::Zeros();
	ff::RectFixedInt edgeFurthestHit = ff::RectFixedInt::Zeros();
	bool footRamp = false;

	if (Helpers::HasAllFlags(hitRectTypes, TileHitRectType::Solid))
	{
		for (size_t i = 0; i < collisionCount; i++)
		{
			const TileCollision& tc = collisions[i];
			if (!tc._tileHitRect->IsSolid())
			{
				continue;
			}

			if (edges.left.DoesTouchHorizIntersectVert(tc._worldRect) && !tc._tileHitRect->HasAny(TileHitRectType::RampDown, TileHitRectType::HeadUp) && tc._worldRect.right - edges.left.right <= Constants::MAX_TILE_PUSH_SIZE)
			{
				if (!state.HasAll(Advancing::State::StandingOnRamp) || tc._worldRect.top <= entry->_pos->_pos.y - Constants::LEVEL_TILE_HEIGHT + Constants::MAX_STAIR_SIZE)
				{
					edgeFurthestHit.left = (edgeHitRectTypes.left == TileHitRectType::None) ? tc._worldRect.right : std::max(edgeFurthestHit.left, tc._worldRect.right);
					edgeHitRectTypes.left = Helpers::SetFlags(edgeHitRectTypes.left, tc._tileHitRect->_type);
				}
			}

			if (edges.right.DoesTouchHorizIntersectVert(tc._worldRect) && !tc._tileHitRect->HasAny(TileHitRectType::RampUp, TileHitRectType::HeadDown) && edges.right.left - tc._worldRect.left <= Constants::MAX_TILE_PUSH_SIZE)
			{
				if (!state.HasAll(Advancing::State::StandingOnRamp) || tc._worldRect.top <= entry->_pos->_pos.y - Constants::LEVEL_TILE_HEIGHT + Constants::MAX_STAIR_SIZE)
				{
					edgeFurthestHit.right = (edgeHitRectTypes.right == TileHitRectType::None) ? tc._worldRect.left : std::min(edgeFurthestHit.right, tc._worldRect.left);
					edgeHitRectTypes.right = Helpers::SetFlags(edgeHitRectTypes.right, tc._tileHitRect->_type);
				}
			}

			if (edges.top.DoesTouchVertIntersectHoriz(tc._worldRect))
			{
				ff::FixedInt bottom = ::AdjustHitTop(tc, edges.top);
				if (bottom >= edges.top.bottom && bottom - edges.top.bottom <= Constants::MAX_TILE_PUSH_SIZE)
				{
					edgeFurthestHit.top = (edgeHitRectTypes.top == TileHitRectType::None) ? bottom : std::max(edgeFurthestHit.top, bottom);
					edgeHitRectTypes.top = Helpers::SetFlags(edgeHitRectTypes.top, tc._tileHitRect->_type);
				}
			}

			if (edges.bottom.DoesTouchVertIntersectHoriz(tc._worldRect))
			{
				ff::FixedInt top = ::AdjustHitBottom(tc, edges.bottom);
				if ((top - (state.HasAll(Advancing::State::StandingOnRamp) ? 1_f : 0_f)) <= edges.bottom.top && edges.bottom.top - top <= Constants::MAX_TILE_PUSH_SIZE)
				{
					// If there's any ramp at the X-center, then use it over all other non-ramps
					if (tc._tileHitRect->IsFootRamp() && tc._worldRect.left <= entry->_pos->_pos.x && tc._worldRect.right >= entry->_pos->_pos.x)
					{
						edgeFurthestHit.bottom = footRamp ? std::min(edgeFurthestHit.bottom, top) : top;
						edgeHitRectTypes.bottom = Helpers::SetFlags(edgeHitRectTypes.bottom, tc._tileHitRect->_type);
						footRamp = true;
					}
					else if (!footRamp)
					{
						edgeFurthestHit.bottom = (edgeHitRectTypes.bottom == TileHitRectType::None) ? top : std::min(edgeFurthestHit.bottom, top);
						edgeHitRectTypes.bottom = Helpers::SetFlags(edgeHitRectTypes.bottom, tc._tileHitRect->_type);
					}
				}
			}
		}
	}

	if (Helpers::HasAllFlags(edgeHitRectTypes.bottom, TileHitRectType::Solid))
	{
		entry->_pos->Offset(entity, 0_f, edgeFurthestHit.bottom - edges.bottom.bottom);
		entry->_vel->_posVelocity.y = std::min(0_f, entry->_vel->_posVelocity.y);
		entry->SetState(Helpers::CombineFlags(Advancing::State::Standing, Helpers::HasAnyFlags(edgeHitRectTypes.bottom, Helpers::CombineFlags(TileHitRectType::RampUp, TileHitRectType::RampDown)) ? Advancing::State::StandingOnRamp : Advancing::State::None));
	}
	else if (state.HasAll(Advancing::State::Jumping))
	{
		if (Helpers::HasAllFlags(edgeHitRectTypes.top, TileHitRectType::Solid))
		{
			entry->_pos->Offset(entity, 0_f, edgeFurthestHit.top - edges.top.top);
			entry->_vel->_posVelocity.y = std::max(0_f, entry->_vel->_posVelocity.y);
			entry->SetState(Advancing::State::None);
		}
	}
	else if (state.HasAll(Advancing::State::Standing))
	{
		entry->SetState(Advancing::State::InAir);
	}

	if (Helpers::HasAllFlags(edgeHitRectTypes.left, TileHitRectType::Solid))
	{
		entry->_pos->Offset(entity, edgeFurthestHit.left - edges.left.left, 0_f);
		entry->_vel->_posVelocity.x = std::max(0_f, entry->_vel->_posVelocity.x);
	}
	else if (Helpers::HasAllFlags(edgeHitRectTypes.right, TileHitRectType::Solid))
	{
		entry->_pos->Offset(entity, edgeFurthestHit.right - edges.right.right, 0_f);
		entry->_vel->_posVelocity.x = std::min(0_f, entry->_vel->_posVelocity.x);
	}
}
