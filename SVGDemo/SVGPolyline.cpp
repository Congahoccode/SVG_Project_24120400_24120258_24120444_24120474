#include "stdafx.h"
#include "SVGPolyline.h"
#include "SVGHelper.h" 
#include <string>
#include <algorithm>

using namespace std;
using namespace Gdiplus;
using namespace rapidxml;

void SVGPolyline::Parse(xml_node<>* node)
{
    SVGElement::Parse(node);
    if (auto a = node->first_attribute("points"))
    {
        const char* ptr = a->value();
        while (*ptr) {
            while (*ptr && !isdigit(*ptr) && *ptr != '-' && *ptr != '.' && *ptr != '+') ptr++;
            if (!*ptr) break;
            float x = ParseNumber(ptr);
            while (*ptr && !isdigit(*ptr) && *ptr != '-' && *ptr != '.' && *ptr != '+') ptr++;
            if (!*ptr) break;
            float y = ParseNumber(ptr);
            points.push_back(PointF(x, y));
        }
    }
}

void SVGPolyline::Draw(Graphics& g)
{
    if (points.size() < 2) return;

    auto state = g.Save();
    g.MultiplyTransform(&transform);

    RectF bounds = GetBoundingBox();

    if (Brush* brush = CreateFillBrush(bounds))
    {
        g.FillPolygon(brush, points.data(), (INT)points.size());
        delete brush;
    }

    if (Pen* pen = CreateStrokePen())
    {
        g.DrawLines(pen, points.data(), (INT)points.size());
        delete pen;
    }

    g.Restore(state);
}

Gdiplus::RectF SVGPolyline::GetBoundingBox()
{
    if (points.empty()) return RectF(0, 0, 0, 0);

    float minX = points[0].X, maxX = points[0].X;
    float minY = points[0].Y, maxY = points[0].Y;

    for (const auto& p : points)
    {
        if (p.X < minX) minX = p.X;
        if (p.X > maxX) maxX = p.X;
        if (p.Y < minY) minY = p.Y;
        if (p.Y > maxY) maxY = p.Y;
    }
    return RectF(minX, minY, maxX - minX, maxY - minY);
}