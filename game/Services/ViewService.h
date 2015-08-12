#pragma once

enum class EntityType;

class IViewService
{
public:
	virtual bool EntityHitTest(const ff::RectFixedInt& screenRect, EntityType typeFilter, ff::ItemCollector<ff::Entity>& results) = 0;
};
