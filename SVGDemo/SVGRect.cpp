#include "stdafx.h"
#include "SVGRect.h"
#include "SVGHelper.h"
#include <string>

void SVGRect::Parse(rapidxml::xml_node<>* node)
{
    SVGElement::Parse(node);
    if (auto a = node->first_attribute("x")) x = ParseUnit(a->value());
    if (auto a = node->first_attribute("y")) y = ParseUnit(a->value());
    if (auto a = node->first_attribute("width")) width = ParseUnit(a->value());
    if (auto a = node->first_attribute("height")) height = ParseUnit(a->value());
}

void SVGRect::Draw(Gdiplus::Graphics& g)
{
    auto state = g.Save();
    g.MultiplyTransform(&transform);

    RectF rect(x, y, width, height);

    if (Brush* brush = CreateFillBrush(rect))
    {
        g.FillRectangle(brush, rect);
        delete brush;
    }

    if (Pen* pen = CreateStrokePen())
    {
        g.DrawRectangle(pen, rect);
        delete pen;
    }

    g.Restore(state);
}

Gdiplus::RectF SVGRect::GetBoundingBox()
{
    return Gdiplus::RectF(x, y, width, height);
}