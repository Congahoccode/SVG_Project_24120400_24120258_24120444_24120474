#include "stdafx.h"
#include "SVGPolygon.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace Gdiplus;

// Hàm tách số (Copy từ SVGElement để dùng nội bộ)
static void GetPoints(const string& s, vector<PointF>& points)
{
    string temp = "";
    vector<float> coords;

    for (char c : s)
    {
        // Chấp nhận số, dấu chấm, dấu trừ, e (khoa học)
        if (isdigit(c) || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E')
        {
            temp += c;
        }
        else
        {
            if (!temp.empty()) {
                try { coords.push_back(stof(temp)); }
                catch (...) {}
                temp = "";
            }
        }
    }
    if (!temp.empty()) { try { coords.push_back(stof(temp)); } catch (...) {} }

    // Gom cứ 2 số thành 1 điểm (x, y)
    for (size_t i = 0; i + 1 < coords.size(); i += 2)
    {
        points.push_back(PointF(coords[i], coords[i + 1]));
    }
}

void SVGPolygon::Parse(rapidxml::xml_node<>* node)
{
    // 1. Parse các thuộc tính chung (Màu, Stroke, Transform...)
    SVGElement::Parse(node);

    // 2. Parse danh sách điểm
    if (auto attr = node->first_attribute("points"))
    {
        string s = attr->value();
        GetPoints(s, points);
    }
}

void SVGPolygon::Draw(Graphics& g)
{
    if (points.empty()) return;

    GraphicsState state = g.Save();

    // QUAN TRỌNG: Áp dụng transform của chính Polygon (nếu có)
    g.MultiplyTransform(&transform);

    // 1. Tô màu (Fill)
    if (auto* brush = CreateFillBrush(GetBounds())) // Hàm này có trong SVGElement.cpp
    {
        g.FillPolygon(brush, points.data(), (INT)points.size());
        delete brush;
    }

    // 2. Vẽ viền (Stroke)
    if (auto* pen = CreateStrokePen()) // Hàm này có trong SVGElement.cpp
    {
        g.DrawPolygon(pen, points.data(), (INT)points.size());
        delete pen;
    }

    g.Restore(state);
}

