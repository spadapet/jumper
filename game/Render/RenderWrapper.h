#pragma once

namespace ff
{
	class IRenderAnimation;
	class IRenderDepth;
	class IRenderTarget;
	class IRenderer;
	class IRendererActive;
	class IRendererActive11;
	class ISprite;
	class RendererActive;
}

class RenderWrapper
{
public:
	RenderWrapper(ff::RendererActive& render);

	static ff::IRendererActive* BeginRender(ff::IRenderer* render, ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt viewRect, ff::RectFixedInt worldRect);
	ff::IRendererActive* GetRenderer() const;
	ff::IRendererActive11* GetRenderer11() const;

	void DrawSprite(ff::ISprite* sprite, ff::PointFixedInt pos, ff::PointFixedInt scale, ff::FixedInt rotate, XMCOLORREF color);
	void DrawFont(ff::ISprite* sprite, ff::PointFixedInt pos, ff::PointFixedInt scale, XMCOLORREF color);
	void DrawLineStrip(const ff::PointFixedInt* points, size_t count, XMCOLORREF color, ff::FixedInt thickness, bool pixelThickness = false);
	void DrawLine(ff::PointFixedInt start, ff::PointFixedInt end, XMCOLORREF color, ff::FixedInt thickness, bool pixelThickness = false);
	void DrawFilledRectangle(ff::RectFixedInt rect, XMCOLORREF color);
	void DrawFilledCircle(ff::PointFixedInt center, ff::FixedInt radius, XMCOLORREF color);
	void DrawOutlineRectangle(ff::RectFixedInt rect, XMCOLORREF color, ff::FixedInt thickness, bool pixelThickness = false);
	void DrawOutlineCircle(ff::PointFixedInt center, ff::FixedInt radius, XMCOLORREF color, ff::FixedInt thickness, bool pixelThickness = false);
	void DrawAnimationFirstFrame(ff::IRenderAnimation* anim, ff::PointFixedInt pos, ff::PointFixedInt scale, ff::FixedInt rotate, XMCOLORREF color);

private:
	ff::IRendererActive* _render;
};
