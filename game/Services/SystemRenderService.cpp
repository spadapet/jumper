#include "pch.h"
#include "Core/CameraView.h"
#include "Core/Globals.h"
#include "Globals/AppGlobals.h"
#include "Graph/Sprite/Sprite.h"
#include "Graph/GraphDevice.h"
#include "Graph/Render/MatrixStack.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RendererActive.h"
#include "Graph/RenderTarget/RenderDepth.h"
#include "Graph/RenderTarget/RenderTarget.h"
#include "Graph/State/GraphStateCache11.h"
#include "Graph/Texture/Texture.h"
#include "Render/RenderWrapper.h"
#include "Services/AppService.h"
#include "Services/CameraService.h"
#include "Services/CollisionService.h"
#include "Services/GameService.h"
#include "Services/LevelService.h"
#include "Services/SystemRenderService.h"
#include "Systems/SystemBase.h"
#include "Types/Timer.h"

SystemRenderServiceStats::SystemRenderServiceStats()
{
	ff::ZeroObject(*this);
}

SystemRenderService::SystemRenderService(ILevelService* levelService)
	: SystemRenderService(levelService, Constants::LEVEL_RENDER_SIZE.ToType<int>(), Constants::LEVEL_BUFFER_SCALE)
{
}

SystemRenderService::SystemRenderService(ILevelService* levelService, ff::PointInt viewSize, int viewScale)
	: _levelService(levelService)
	, _viewSize(viewSize)
	, _bufferSize(viewSize* viewScale)
	, _viewport(viewSize.ToType<float>())
{
	ff::AppGlobals& globals = levelService->GetGameService()->GetAppService()->GetAppGlobals();
	_render = globals.GetGraph()->CreateRenderer();

	InitTargets();
}

void SystemRenderService::InitTargets()
{
	ff::AppGlobals& globals = _levelService->GetGameService()->GetAppService()->GetAppGlobals();
	ff::IGraphDevice* graph = globals.GetGraph();

	_renderTexture = graph->CreateTexture(_viewSize, ff::TextureFormat::BGRA32);
	_renderTarget = graph->CreateRenderTargetTexture(_renderTexture, 0, 1, 0);
	_renderDepth = graph->CreateRenderDepth(_viewSize);

	_bufferTexture = graph->CreateTexture(_bufferSize, ff::TextureFormat::BGRA32);
	_bufferTarget = graph->CreateRenderTargetTexture(_bufferTexture, 0, 1, 0);
	_bufferDepth = graph->CreateRenderDepth(_bufferSize);

	_renderTextureSprite.QueryFrom(_renderTexture);
	_bufferTextureSprite.QueryFrom(_bufferTexture);
}

void SystemRenderService::Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, SystemIter beginIter, SystemIter endIter)
{
	UpdateRenderQueue(beginIter, endIter);

	ff::RendererActive render;
	ff::RectFixedInt viewRect(_viewSize.ToType<ff::FixedInt>());
	ff::RectFixedInt bufferRect(_bufferSize.ToType<ff::FixedInt>());
	ff::RectFixedInt targetRect = GetTargetViewport(target);
	ff::RectFixedInt targetSizeRect(targetRect.Size());
	const CameraView& cameraView = _levelService->GetCameraService()->GetFinalCameraView();

	// Draw low res (level...)
	_renderTarget->Clear();
	_stats._lowResTicks = ff::Timer::GetCurrentRawTime();
	if (_renderQueue.HasLowRes() && (render = RenderWrapper::BeginRender(_render.get(), _renderTarget, _renderDepth, viewRect, cameraView._view)) != nullptr)
	{
		render->GetWorldMatrixStack().TransformMatrix(cameraView._worldToViewTransform);
		_renderQueue.RenderLowRes(RenderWrapper(render));
	}

	// Draw low res overlays (status bars...)
	_stats._lowResOverlayTicks = ff::Timer::GetCurrentRawTime();
	if (_renderQueue.HasLowResOverlays() && (render = RenderWrapper::BeginRender(_render.get(), _renderTarget, _renderDepth, viewRect, viewRect)) != nullptr)
	{
		_renderQueue.RenderLowResOverlays(RenderWrapper(render));
	}

	// Stretch low res rendering to the high res buffer
	_stats._lowResStretchTicks = ff::Timer::GetCurrentRawTime();
	if ((render = RenderWrapper::BeginRender(_render.get(), _bufferTarget, nullptr, bufferRect, viewRect)) != nullptr)
	{
		RenderWrapper wrapper(render);
		wrapper.DrawSprite(_renderTextureSprite, ff::PointFixedInt::Zeros(), ff::PointFixedInt::Ones(), 0_f, ff::GetColorWhite());
	}

	// Render high res items (hints...)
	_stats._highResTicks = ff::Timer::GetCurrentRawTime();
	if (_renderQueue.HasHighRes() && (render = RenderWrapper::BeginRender(_render.get(), _bufferTarget, _bufferDepth, bufferRect, cameraView._view)) != nullptr)
	{
		render->GetWorldMatrixStack().TransformMatrix(cameraView._worldToViewTransform);
		_renderQueue.RenderHighRes(RenderWrapper(render));
	}

	// Render high res overlays (text...)
	_stats._highResOverlayTicks = ff::Timer::GetCurrentRawTime();
	if (_renderQueue.HasHighResOverlays() && (render = RenderWrapper::BeginRender(_render.get(), _bufferTarget, _bufferDepth, bufferRect, bufferRect)) != nullptr)
	{
		_renderQueue.RenderHighResOverlays(RenderWrapper(render));
	}

	// Stretch high res buffer to target
	_stats._highResStretchTicks = ff::Timer::GetCurrentRawTime();
	if (render = RenderWrapper::BeginRender(_render.get(), target, nullptr, targetRect, bufferRect))
	{
		RenderWrapper wrapper(render);
		if (wrapper.GetRenderer11())
		{
			wrapper.GetRenderer11()->PushTextureSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR);
		}

		wrapper.DrawSprite(_bufferTextureSprite, ff::PointFixedInt::Zeros(), ff::PointFixedInt::Ones(), 0_f, ff::GetColorWhite());
	}

	// Render screen res items (debug boxes...)
	_stats._screenResTicks = ff::Timer::GetCurrentRawTime();
	if (_renderQueue.HasScreenRes() && (render = RenderWrapper::BeginRender(_render.get(), target, depth, targetRect, cameraView._view)) != nullptr)
	{
		render->GetWorldMatrixStack().TransformMatrix(cameraView._worldToViewTransform);
		_renderQueue.RenderScreenRes(RenderWrapper(render));
	}

	// Render screen res overlays (mouse cursor...)
	_stats._screenResOverlayTicks = ff::Timer::GetCurrentRawTime();
	if (_renderQueue.HasScreenResOverlays() && (render = RenderWrapper::BeginRender(_render.get(), target, depth, targetRect, targetSizeRect)) != nullptr)
	{
		_renderQueue.RenderScreenResOverlays(RenderWrapper(render));
	}

	_stats._lowResTicks = _stats._lowResOverlayTicks - _stats._lowResTicks;
	_stats._lowResOverlayTicks = _stats._lowResStretchTicks - _stats._lowResOverlayTicks;
	_stats._lowResStretchTicks = _stats._highResTicks - _stats._lowResStretchTicks;
	_stats._highResTicks = _stats._highResOverlayTicks - _stats._highResTicks;
	_stats._highResOverlayTicks = _stats._highResStretchTicks - _stats._highResOverlayTicks;
	_stats._highResStretchTicks = _stats._screenResTicks - _stats._highResStretchTicks;
	_stats._screenResTicks = _stats._screenResOverlayTicks - _stats._screenResTicks;
	_stats._screenResOverlayTicks = ff::Timer::GetCurrentRawTime() - _stats._screenResOverlayTicks;
}

const SystemRenderServiceStats& SystemRenderService::GetStats() const
{
	return _stats;
}

void SystemRenderService::UpdateRenderQueue(SystemIter beginIter, SystemIter endIter)
{
	_renderQueue.Reset();

	INT64 startTime = ff::Timer::GetCurrentRawTime();

	for (SystemIter i = beginIter; i != endIter; i++)
	{
		SystemBase* system = i->get();
		system->Render(&_levelService->GetDomain(), &_renderQueue);

		INT64 endTime = ff::Timer::GetCurrentRawTime();
		system->SetRenderQueueTime(endTime - startTime);
		startTime = endTime;
	}

	_renderQueue.DoneDrawing();
}

ff::RectFixedInt SystemRenderService::GetTargetViewport(ff::IRenderTarget* target)
{
	return _viewport.GetView(target).ToType<ff::FixedInt>();
}

bool SystemRenderService::EntityHitTest(const ff::RectFixedInt& screenRect, EntityType typeFilter, ff::ItemCollector<ff::Entity>& results)
{
	const CameraView& cameraView = _levelService->GetCameraService()->GetFinalCameraView();
	ff::RectFixedInt targetRect = GetTargetViewport();
	noAssertRetVal(targetRect.Area(), false);

	ff::RectFixedInt viewRect = (screenRect - targetRect.TopLeft()) / targetRect.Size();
	viewRect = viewRect * cameraView._view.Size() + cameraView._view.TopLeft();
	ff::RectFixedInt worldRect(
		cameraView.TransformViewToWorld(viewRect.TopLeft()),
		cameraView.TransformViewToWorld(viewRect.BottomRight()));

	return _levelService->GetCollisionService()->EntityHitTest(worldRect, typeFilter, results, true);
}
