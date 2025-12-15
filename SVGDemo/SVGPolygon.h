#pragma once
#include "SVGElement.h"
#include <vector>

class SVGPolygon : public SVGElement
{
private:
    std::vector<Gdiplus::PointF> points;

public:
    SVGPolygon() {}
    void Parse(rapidxml::xml_node<>* node) override;
    void Draw(Gdiplus::Graphics& g) override;
    RectF GetBounds()
    {
        if (points.empty()) return RectF(0, 0, 0, 0);
        float minX = points[0].X, maxX = points[0].X;
        float minY = points[0].Y, maxY = points[0].Y;
        for (auto& p : points) {
            if (p.X < minX) minX = p.X; if (p.X > maxX) maxX = p.X;
            if (p.Y < minY) minY = p.Y; if (p.Y > maxY) maxY = p.Y;
        }
        return RectF(minX, minY, maxX - minX, maxY - minY);
    }
};
