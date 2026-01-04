#include "stdafx.h"
#include "SVGCircle.h"
#include "SVGHelper.h"

void SVGCircle::Parse(rapidxml::xml_node<>* node)
{
    SVGElement::Parse(node);
    if (auto attr = node->first_attribute("cx")) cx = ParseUnit(attr->value());
    if (auto attr = node->first_attribute("cy")) cy = ParseUnit(attr->value());
    if (auto attr = node->first_attribute("r"))  r = ParseUnit(attr->value());
}

void SVGCircle::Draw(Gdiplus::Graphics& g)
{
    Gdiplus::GraphicsState state = g.Save();
    g.MultiplyTransform(&transform);

    Gdiplus::RectF bounds(cx - r, cy - r, 2 * r, 2 * r);

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

Gdiplus::RectF SVGCircle::GetBoundingBox()
{
    return Gdiplus::RectF(cx - r, cy - r, 2 * r, 2 * r);
}