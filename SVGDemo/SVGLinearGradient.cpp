#include "stdafx.h"
#include "SVGLinearGradient.h"
#include "rapidxml.hpp"
#include <sstream>

static float ParseFloat(const char* s)
{
    return (float)atof(s);
}

void SVGLinearGradient::Parse(rapidxml::xml_node<>* node)
{
    if (auto attr = node->first_attribute("id"))
        id = attr->value();

    if (auto attr = node->first_attribute("x1")) x1 = ParseFloat(attr->value());
    if (auto attr = node->first_attribute("y1")) y1 = ParseFloat(attr->value());
    if (auto attr = node->first_attribute("x2")) x2 = ParseFloat(attr->value());
    if (auto attr = node->first_attribute("y2")) y2 = ParseFloat(attr->value());

    if (auto attr = node->first_attribute("gradientUnits"))
        userSpace = string(attr->value()) == "userSpaceOnUse";

    // Parse <stop>
    for (auto* stop = node->first_node("stop"); stop; stop = stop->next_sibling("stop"))
    {
        SVGGradientStop gs;
        gs.offset = 0.0f;

        if (auto a = stop->first_attribute("offset"))
        {
            string v = a->value();
            if (v.back() == '%')
                gs.offset = atof(v.c_str()) / 100.0f;
            else
                gs.offset = atof(v.c_str());
        }

        int r = 0, g = 0, b = 0, a = 255;

        if (auto c = stop->first_attribute("stop-color"))
        {
            string s = c->value();
            if (!s.empty() && s[0] == '#')
            {
                r = stoi(s.substr(1, 2), nullptr, 16);
                g = stoi(s.substr(3, 2), nullptr, 16);
                b = stoi(s.substr(5, 2), nullptr, 16);
            }
        }

        if (auto op = stop->first_attribute("stop-opacity"))
            a = (int)(255 * atof(op->value()));

        gs.color = Color(a, r, g, b);
        stops.push_back(gs);
    }
}

LinearGradientBrush* SVGLinearGradient::CreateBrush(const RectF& bounds) const
{
    PointF p1, p2;

    if (userSpace)
    {
        p1 = PointF(x1, y1);
        p2 = PointF(x2, y2);
    }
    else
    {
        p1 = PointF(bounds.X + bounds.Width * x1,
            bounds.Y + bounds.Height * y1);
        p2 = PointF(bounds.X + bounds.Width * x2,
            bounds.Y + bounds.Height * y2);
    }

    auto* brush = new LinearGradientBrush(p1, p2, Color::Black, Color::Black);

    if (stops.size() >= 2)
    {
        vector<Color> colors;
        vector<REAL> positions;

        for (auto& s : stops)
        {
            colors.push_back(s.color);
            positions.push_back(s.offset);
        }

        brush->SetInterpolationColors(colors.data(), positions.data(), (INT)colors.size());
    }

    return brush;
}
