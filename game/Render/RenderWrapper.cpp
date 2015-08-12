#include "pch.h"
#include "Core/Helpers.h"
#include "Graph/Anim/AnimKeys.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RenderAnimation.h"
#include "Graph/Render/RendererActive.h"
#include "Render/RenderWrapper.h"

RenderWrapper::RenderWrapper(ff::RendererActive& render)
	: _render(render)
{
	assert(render);
}

ff::IRendererActive* RenderWrapper::BeginRender(ff::IRenderer* render, ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt viewRect, ff::RectFixedInt worldRect)
{
	return render->BeginRender(target, depth, viewRect.ToType<float>(), worldRect.ToType<float>());
}

ff::IRendererActive* RenderWrapper::GetRenderer() const
{
	return _render;
}

ff::IRendererActive11* RenderWrapper::GetRenderer11() const
{
	return _render->AsRendererActive11();
}

void RenderWrapper::DrawSprite(ff::ISprite* sprite, ff::PointFixedInt pos, ff::PointFixedInt scale, ff::FixedInt rotate, XMCOLORREF color)
{
	_render->DrawSprite(sprite, pos.ToType<float>(), scale.ToType<float>(), Helpers::DegreesToRadians(rotate), color);
}

void RenderWrapper::DrawFont(ff::ISprite* sprite, ff::PointFixedInt pos, ff::PointFixedInt scale, XMCOLORREF color)
{
	_render->DrawFont(sprite, pos.ToType<float>(), scale.ToType<float>(), color);
}

void RenderWrapper::DrawLineStrip(const ff::PointFixedInt* points, size_t count, XMCOLORREF color, ff::FixedInt thickness, bool pixelThickness)
{
	ff::Vector<ff::PointFloat, 64> pointFloats;
	pointFloats.Resize(count);

	for (size_t i = 0; i < count; i++)
	{
		pointFloats[i] = points[i].ToType<float>();
	}

	_render->DrawLineStrip(pointFloats.ConstData(), pointFloats.Size(), color, thickness, pixelThickness);
}

void RenderWrapper::DrawLine(ff::PointFixedInt start, ff::PointFixedInt end, XMCOLORREF color, ff::FixedInt thickness, bool pixelThickness)
{
	_render->DrawLine(start.ToType<float>(), end.ToType<float>(), color, thickness, pixelThickness);
}

void RenderWrapper::DrawFilledRectangle(ff::RectFixedInt rect, XMCOLORREF color)
{
	_render->DrawFilledRectangle(rect.ToType<float>(), color);
}

void RenderWrapper::DrawFilledCircle(ff::PointFixedInt center, ff::FixedInt radius, XMCOLORREF color)
{
	_render->DrawFilledCircle(center.ToType<float>(), radius, color);
}

void RenderWrapper::DrawOutlineRectangle(ff::RectFixedInt rect, XMCOLORREF color, ff::FixedInt thickness, bool pixelThickness)
{
	_render->DrawOutlineRectangle(rect.ToType<float>(), color, thickness, pixelThickness);
}

void RenderWrapper::DrawOutlineCircle(ff::PointFixedInt center, ff::FixedInt radius, XMCOLORREF color, ff::FixedInt thickness, bool pixelThickness)
{
	_render->DrawOutlineCircle(center.ToType<float>(), radius, color, thickness, pixelThickness);
}

void RenderWrapper::DrawAnimationFirstFrame(ff::IRenderAnimation* anim, ff::PointFixedInt pos, ff::PointFixedInt scale, ff::FixedInt rotate, XMCOLORREF color)
{
	anim->Render(_render, ff::AnimTweenType::POSE_TWEEN_LINEAR_CLAMP, 0, pos.ToType<float>(), scale.ToType<float>(), Helpers::DegreesToRadians(rotate), color);
}
