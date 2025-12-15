#include "stdafx.h"
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include "SVGRenderer.h"

using namespace Gdiplus;
using namespace std;

void SVGRenderer::Render(Graphics& g, const vector<SVGElement*>& elements)
{
    // Clear previous drawing, fill background
    g.Clear(backgroundColor);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    /*
    // Save current graphics state
    GraphicsState state = g.Save();

    // Apply renderer-wide transforms (scale, offset)
    Matrix transform;
    transform.Scale(scale, scale);
    transform.Translate(offset.X, offset.Y, MatrixOrderAppend);
    g.MultiplyTransform(&transform);
    */

    g.TranslateTransform(offset.X, offset.Y);
    g.ScaleTransform(zoomFactor, zoomFactor);
	g.RotateTransform(rotationAngle);


    // --- Draw each SVG element ---
    for (auto e : elements)
    {
        if (e)
        {
            // Each element applies its own transform in Draw
            e->Draw(g);
        }
    }

    // Restore original graphics state
    //g.Restore(state);
}

void SVGRenderer::Zoom(float factor)
{
    zoomFactor *= factor;
    // Giới hạn zoom để tránh mất hình
    if (zoomFactor < 0.1f) zoomFactor = 0.1f;
    if (zoomFactor > 10.0f) zoomFactor = 10.0f;
}

void SVGRenderer::Pan(float dx, float dy)
{
    offset.X += dx;
    offset.Y += dy;
}
void SVGRenderer::Rotate(float degrees)
{
    rotationAngle += degrees;

    // Giữ góc trong khoảng 0–360 cho gọn
    if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;
    if (rotationAngle < 0.0f) rotationAngle += 360.0f;
}