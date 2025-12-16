#include "stdafx.h"
#include "SVGPolyline.h"
#include <sstream>
#include <string>

using namespace std;
using namespace Gdiplus;
using namespace rapidxml;

void SVGPolyline::Parse(xml_node<>* node) 
{
    SVGElement::Parse(node);
    for (auto* a = node->first_attribute(); a; a = a->next_attribute()) 
    {
        string n = a->name();
        if (n == "points") 
        {
            stringstream ss(a->value());
            float x, y;
            char comma;
            while (ss >> x) 
            {
                if (ss.peek() == ',') ss >> comma;
                if (ss >> y) points.push_back(PointF(x, y));
            }
        }
    }
}

void SVGPolyline::Draw(Graphics& g)
{
    if (points.size() < 2) return;

    auto state = g.Save();
    g.MultiplyTransform(&transform);

    // Compute bounds
    float minX = points[0].X;
    float minY = points[0].Y;
    float maxX = points[0].X;
    float maxY = points[0].Y;

    for (size_t i = 1; i < points.size(); ++i)
    {
        minX = min(minX, points[i].X);
        minY = min(minY, points[i].Y);
        maxX = max(maxX, points[i].X);
        maxY = max(maxY, points[i].Y);
    }

    RectF bounds(minX, minY, maxX - minX, maxY - minY);

    // ===== FILL (theo yêu cầu đồ án: dùng FillPolygon dù không khép kín) =====
    if (Brush* brush = CreateFillBrush(bounds))
    {
        g.FillPolygon(brush, points.data(), (INT)points.size());
        delete brush;
    }

    // ===== STROKE =====
    if (Pen* pen = CreateStrokePen())
    {
        g.DrawLines(pen, points.data(), (INT)points.size());
        delete pen;
    }

    g.Restore(state);
}
