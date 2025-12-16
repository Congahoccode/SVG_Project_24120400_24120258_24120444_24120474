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

void SVGRenderer::AutoFit(int screenWidth, int screenHeight, const vector<SVGElement*>& elements)
{
    if (elements.empty()) return;

    // 1. Tính tổng khung bao của toàn bộ bản vẽ
    RectF totalRect(0, 0, 0, 0);
    bool first = true;

    for (auto* e : elements)
    {
        RectF r = e->GetBoundingBox();
        if (r.Width <= 0 || r.Height <= 0) continue;

        if (first) { totalRect = r; first = false; }
        else { totalRect.Union(totalRect, totalRect, r); }
    }

    if (totalRect.Width <= 0 || totalRect.Height <= 0) return;

    // 2. Tính tỉ lệ Zoom cần thiết
    float padding = 40.0f;
    float availableW = (float)screenWidth - padding;
    float availableH = (float)screenHeight - padding;

    float scaleX = availableW / totalRect.Width;
    float scaleY = availableH / totalRect.Height;

    // Chọn tỉ lệ nhỏ hơn để hình nằm lọt lòng (Fit)
    zoomFactor = min(scaleX, scaleY);

    // 3. Tính Offset để đưa hình về giữa màn hình
    // Công thức: (Màn hình / 2) - (Tâm hình * Zoom)
    float midX = totalRect.X + totalRect.Width / 2.0f;
    float midY = totalRect.Y + totalRect.Height / 2.0f;

    offset.X = (screenWidth / 2.0f) - (midX * zoomFactor);
    offset.Y = (screenHeight / 2.0f) - (midY * zoomFactor);
}