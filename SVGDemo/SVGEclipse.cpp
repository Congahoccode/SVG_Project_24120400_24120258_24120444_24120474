#include "stdafx.h"
#include "SVGEclipse.h" 
#include "SVGHelper.h"

void SVGEllipse::Parse(rapidxml::xml_node<>* node)
{
    SVGElement::Parse(node);
    if (auto attr = node->first_attribute("cx")) cx = ParseUnit(attr->value());
    if (auto attr = node->first_attribute("cy")) cy = ParseUnit(attr->value());
    if (auto attr = node->first_attribute("rx")) rx = ParseUnit(attr->value());
    if (auto attr = node->first_attribute("ry")) ry = ParseUnit(attr->value());
}

void SVGEllipse::Draw(Gdiplus::Graphics& g)
{
    Gdiplus::GraphicsState state = g.Save();
    g.MultiplyTransform(&transform);

    Gdiplus::RectF bounds(cx - rx, cy - ry, 2 * rx, 2 * ry);

    if (auto* brush = CreateFillBrush(bounds)) {
        g.FillEllipse(brush, bounds);
        delete brush;
    }
    if (auto* pen = CreateStrokePen()) {
        g.DrawEllipse(pen, bounds);
        delete pen;
    }
    g.Restore(state);
}

Gdiplus::RectF SVGEllipse::GetBoundingBox()
{
    return Gdiplus::RectF(cx - rx, cy - ry, 2 * rx, 2 * ry);
}