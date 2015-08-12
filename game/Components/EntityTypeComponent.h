#pragma once

enum class EntityType
{
	None = 0x0000,
	Camera = 0x0001,
	Enemy = 0x0002,
	Layer = 0x0004,
	Player = 0x0008,

	Any = 0x7FFFFFFF,
	CollidesWithEachOther = Enemy | Player,
	CollidesWithTiles = Enemy | Player,
};

struct EntityTypeComponent : public ff::Component
{
	EntityTypeComponent();

	void Set(ff::Entity entity, EntityType type);
	bool HasAnyType(EntityType type) const;
	bool HasAllTypes(EntityType type) const;

	EntityType _type;
};
