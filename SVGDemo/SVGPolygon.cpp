#include "stdafx.h"
#include "SVGPolygon.h"
#include "SVGHelper.h"
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace Gdiplus;

static void GetPoints(const string& s, vector<PointF>& points)
{
    const char* ptr = s.c_str();
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

void SVGPolygon::Parse(rapidxml::xml_node<>* node)
{
    SVGElement::Parse(node); // Parse fill, stroke, transform...

    // Parse danh sách điểm
    if (auto attr = node->first_attribute("points"))
    {
        GetPoints(attr->value(), points);
    }

    fillMode = FillModeAlternate;
    if (auto attr = node->first_attribute("fill-rule")) {
        string rule = attr->value();
        if (rule == "nonzero") fillMode = FillModeWinding;
        else if (rule == "evenodd") fillMode = FillModeAlternate;
    }
}

void SVGPolygon::Draw(Graphics& g)
{
    if (points.empty()) return;

    GraphicsState state = g.Save();
    g.MultiplyTransform(&transform);

    RectF bounds = GetBoundingBox();

    if (auto* brush = CreateFillBrush(bounds))
    {
        g.FillPolygon(brush, points.data(), (INT)points.size(), fillMode);
        delete brush;
    }

    if (auto* pen = CreateStrokePen())
    {
        g.DrawPolygon(pen, points.data(), (INT)points.size());
        delete pen;
    }

    g.Restore(state);
}

RectF SVGPolygon::GetBoundingBox()
{
    if (points.empty()) return RectF(0, 0, 0, 0);

    float minX = points[0].X, maxX = points[0].X;
    float minY = points[0].Y, maxY = points[0].Y;

    for (const auto& p : points) {
        if (p.X < minX) minX = p.X;
        if (p.X > maxX) maxX = p.X;
        if (p.Y < minY) minY = p.Y;
        if (p.Y > maxY) maxY = p.Y;
    }
    float w = maxX - minX; if (w < 0.1f) w = 0.1f;
    float h = maxY - minY; if (h < 0.1f) h = 0.1f;

    return RectF(minX, minY, w, h);
}