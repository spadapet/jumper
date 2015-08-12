#pragma once

#include "Graph/RenderTarget/Viewport.h"
#include "Services/RenderQueueService.h"
#include "Services/ViewService.h"
#include "Systems/SystemBase.h"

namespace ff
{
	class EntityDomain;
	class IRenderDepth;
	class IRenderer;
	class IRenderTarget;
	class ISprite;
	class ITexture;
}

class ILevelService;

struct SystemRenderServiceStats
{
	SystemRenderServiceStats();

	UINT64 _lowResTicks;
	UINT64 _lowResOverlayTicks;
	UINT64 _lowResStretchTicks;
	UINT64 _highResTicks;
	UINT64 _highResOverlayTicks;
	UINT64 _highResStretchTicks;
	UINT64 _screenResTicks;
	UINT64 _screenResOverlayTicks;
};

class SystemRenderService : public IViewService
{
public:
	SystemRenderService(ILevelService* levelService);
	SystemRenderService(ILevelService* levelService, ff::PointInt viewSize, int viewScale);

	void Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, SystemIter beginIter, SystemIter endIter);
	const SystemRenderServiceStats& GetStats() const;

	// IViewService
	virtual bool EntityHitTest(const ff::RectFixedInt& screenRect, EntityType typeFilter, ff::ItemCollector<ff::Entity>& results) override;

private:
	void InitTargets();
	void UpdateRenderQueue(SystemIter beginIter, SystemIter endIter);
	ff::RectFixedInt GetTargetViewport(ff::IRenderTarget* target = nullptr);

	ff::PointInt _viewSize;
	ff::PointInt _bufferSize;
	ff::Viewport _viewport;
	SystemRenderServiceStats _stats;
	ILevelService* _levelService;
	RenderQueueService _renderQueue;

	// Low res game buffer
	ff::ComPtr<ff::ITexture> _renderTexture;
	ff::ComPtr<ff::ISprite> _renderTextureSprite;
	ff::ComPtr<ff::IRenderTarget> _renderTarget;
	ff::ComPtr<ff::IRenderDepth> _renderDepth;

	// 1080p screen buffer
	ff::ComPtr<ff::ITexture> _bufferTexture;
	ff::ComPtr<ff::ISprite> _bufferTextureSprite;
	ff::ComPtr<ff::IRenderTarget> _bufferTarget;
	ff::ComPtr<ff::IRenderDepth> _bufferDepth;

	std::unique_ptr<ff::IRenderer> _render;
};
