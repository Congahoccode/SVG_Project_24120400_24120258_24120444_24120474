#include "stdafx.h"
#include "SVGRadialGradient.h"
#include "SVGDocument.h" 
#include "SVGLinearGradient.h"
#include "SVGHelper.h"
#include <algorithm>
#include <vector>

using namespace std;
using namespace Gdiplus;

void SVGRadialGradient::Parse(rapidxml::xml_node<>* node, SVGDocument* doc)
{
    if (auto attr = node->first_attribute("id")) id = attr->value();
    if (auto attr = node->first_attribute("cx")) cx = ParseUnit(attr->value()); else cx = 0.5f;
    if (auto attr = node->first_attribute("cy")) cy = ParseUnit(attr->value()); else cy = 0.5f;
    if (auto attr = node->first_attribute("r"))  r = ParseUnit(attr->value());  else r = 0.5f;
    if (auto attr = node->first_attribute("fx")) fx = ParseUnit(attr->value()); else fx = cx;
    if (auto attr = node->first_attribute("fy")) fy = ParseUnit(attr->value()); else fy = cy;

    if (auto attr = node->first_attribute("gradientUnits"))
        userSpace = string(attr->value()) == "userSpaceOnUse";

    // Kế thừa
    if (auto attr = node->first_attribute("xlink:href")) {
        string href = attr->value();
        if (!href.empty() && href[0] == '#' && doc) {
            string pid = href.substr(1);
            if (auto* rad = doc->GetRadialGradient(pid)) stops = rad->GetStops();
            else if (auto* lin = doc->GetLinearGradient(pid)) stops = lin->GetStops();
        }
    }

    // Parse Matrix
    if (auto attr = node->first_attribute("gradientTransform")) {
        ParseTransformString(attr->value(), transform);
    }

    // Parse Stops
    if (node->first_node("stop")) stops.clear();
    for (auto* stop = node->first_node("stop"); stop; stop = stop->next_sibling("stop")) {
        SVGGradientStop gs; gs.offset = 0.0f;
        if (auto a = stop->first_attribute("offset")) gs.offset = ParseUnit(a->value());

        string cStr;
        if (auto c = stop->first_attribute("stop-color")) cStr = c->value();

        gs.color = ParseColor(cStr);
        if (auto op = stop->first_attribute("stop-opacity")) {
            const char* p = op->value();
            gs.color = Color((BYTE)(ParseNumber(p) * 255), gs.color.GetR(), gs.color.GetG(), gs.color.GetB());
        }
        stops.push_back(gs);
    }
}

PathGradientBrush* SVGRadialGradient::CreateBrush(const RectF& bounds) const
{
    const float EXPAND = 1.5f;

    GraphicsPath path;
    float realR = r;
    if (realR <= 0.0001f) realR = 0.5f;

    float expandedR = realR * EXPAND;
    path.AddEllipse(cx - expandedR, cy - expandedR, expandedR * 2, expandedR * 2);

    PointF centerPoint(fx, fy);
    Matrix m;

    // 1. Gradient Transform
    if (!transform.IsIdentity()) {
        m.Multiply(const_cast<Matrix*>(&transform), MatrixOrderAppend);
    }
    // 2. Map to Bounds
    if (!userSpace) {
        m.Scale(bounds.Width, bounds.Height, MatrixOrderAppend);
        m.Translate(bounds.X, bounds.Y, MatrixOrderAppend);
    }

    // Biến đổi hình học
    path.Transform(&m);
    m.TransformPoints(&centerPoint);

    auto* brush = new PathGradientBrush(&path);
    brush->SetCenterPoint(centerPoint);

    if (!stops.empty()) {
        vector<SVGGradientStop> safeStops = stops;
        if (safeStops.empty() || safeStops.back().offset < 0.999f) {
            safeStops.push_back({ 1.0f, safeStops.empty() ? Color::Black : safeStops.back().color });
        }

        vector<Color> cols;
        vector<REAL> pos;

        for (auto it = safeStops.rbegin(); it != safeStops.rend(); ++it) {
            cols.push_back(it->color);
            float p = 1.0f - (it->offset / EXPAND);
            if (p < 0) p = 0; if (p > 1) p = 1;
            pos.push_back(p);
        }

        if (pos.empty() || pos.front() > 0.001f) {
            Color lastColor = cols.front();

            pos.insert(pos.begin(), 0.0f);
            cols.insert(cols.begin(), lastColor);
        }
        else {
            pos.front() = 0.0f;
        }

        if (pos.back() < 0.999f) {
            pos.push_back(1.0f);
            cols.push_back(cols.back());
        }
        else {
            pos.back() = 1.0f;
        }

        brush->SetInterpolationColors(cols.data(), pos.data(), (INT)cols.size());
    }
    else {
        brush->SetCenterColor(Color::Black);
        int c = 1; Color sc = Color::Black;
        brush->SetSurroundColors(&sc, &c);
    }
    brush->SetWrapMode(WrapModeClamp);

    return brush;
}