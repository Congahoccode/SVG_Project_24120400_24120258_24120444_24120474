#include "stdafx.h"
#include "SVGLine.h"
#include "SVGHelper.h"
#include <string>
#include <algorithm>

using namespace std;
using namespace Gdiplus;
using namespace rapidxml;

void SVGLine::Parse(xml_node<>* node)
{
    SVGElement::Parse(node);
    if (auto a = node->first_attribute("x1")) x1 = ParseUnit(a->value());
    if (auto a = node->first_attribute("y1")) y1 = ParseUnit(a->value());
    if (auto a = node->first_attribute("x2")) x2 = ParseUnit(a->value());
    if (auto a = node->first_attribute("y2")) y2 = ParseUnit(a->value());
}

void SVGLine::Draw(Gdiplus::Graphics& g)
{
    auto state = g.Save();
    g.MultiplyTransform(&transform);

    if (Pen* pen = CreateStrokePen())
    {
        g.DrawLine(pen, x1, y1, x2, y2);
        delete pen;
    }

    g.Restore(state);
}

Gdiplus::RectF SVGLine::GetBoundingBox()
{
    float minX = min(x1, x2);
    float minY = min(y1, y2);
    float w = abs(x2 - x1);
    float h = abs(y2 - y1);

    if (h < 0.1f) h = 0.1f;

    return Gdiplus::RectF(minX, minY, w, h);
}