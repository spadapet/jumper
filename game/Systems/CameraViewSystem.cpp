#include "pch.h"
#include "Components/BoundingBoxComponent.h"
#include "Components/CameraComponent.h"
#include "Components/PositionToBoundingBoxComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Globals.h"
#include "Systems/CameraViewSystem.h"

struct CameraViewSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS()

	CameraComponent* _camera;
	BoundingBoxComponent* _box;
	PositionToBoundingBoxComponent* _posToBox;
	TransformComponent* _transform;
};

BEGIN_ENTRY_COMPONENTS(CameraViewSystemEntry)
HAS_COMPONENT(CameraComponent)
HAS_COMPONENT(BoundingBoxComponent)
HAS_COMPONENT(PositionToBoundingBoxComponent)
HAS_COMPONENT(TransformComponent)
END_ENTRY_COMPONENTS(CameraViewSystemEntry)

CameraViewSystem::CameraViewSystem(ff::EntityDomain* domain)
	: _domain(domain)
	, _bucket(domain->GetBucket<CameraViewSystemEntry>())
{
	_domain->AddEventHandler(EntityEvents::ID_BOUNDING_BOX_CHANGED, this);
	_domain->AddEventHandler(EntityEvents::ID_POSITION_TO_BOUNDING_BOX_CHANGED, this);
	_domain->AddEventHandler(EntityEvents::ID_TRANSFORM_CHANGED, this);
	_bucket->AddListener(this);
}

CameraViewSystem::~CameraViewSystem()
{
	_domain->RemoveEventHandler(EntityEvents::ID_BOUNDING_BOX_CHANGED, this);
	_domain->RemoveEventHandler(EntityEvents::ID_POSITION_TO_BOUNDING_BOX_CHANGED, this);
	_domain->RemoveEventHandler(EntityEvents::ID_TRANSFORM_CHANGED, this);
	_bucket->RemoveListener(this);
}

void CameraViewSystem::OnEntityDeleted(ff::Entity entity)
{
}

void CameraViewSystem::OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs)
{
	UpdateEntity(entity);
}

void CameraViewSystem::OnEntryAdded(CameraViewSystemEntry& entry)
{
	UpdateEntity(entry);
}

void CameraViewSystem::OnEntryRemoving(CameraViewSystemEntry& entry)
{
}

void CameraViewSystem::UpdateEntity(ff::Entity entity)
{
	CameraViewSystemEntry* entry = _bucket->GetEntry(entity);
	noAssertRet(entry);
	UpdateEntity(*entry);
}

void CameraViewSystem::UpdateEntity(CameraViewSystemEntry& entry)
{
	const ff::RectFixedInt& worldBounds = entry._box->_box;

	CameraView& view = entry._camera->_view;
	view._viewToWorldTransform = entry._transform->GetTransformRaw();
	view._worldToViewTransform = entry._transform->GetInverseTransformRaw();
	view._worldBounds = worldBounds;
	view._worldRenderBounds = worldBounds.Inflate(worldBounds.Width() / 4, worldBounds.Height() / 4);
	view._worldUpdateBounds = worldBounds.Inflate(worldBounds.Width(), worldBounds.Height());
	view._view.SetRect(entry._posToBox->_points[0], entry._posToBox->_points[2]);
}
