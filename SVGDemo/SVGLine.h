#pragma once
#include "SVGElement.h"

class SVGLine : public SVGElement 
{
private:
    float x1, y1, x2, y2;

public:
    SVGLine() : x1(0), y1(0), x2(0), y2(0) {}
    void Parse(rapidxml::xml_node<>* node) override;
    void Draw(Gdiplus::Graphics& g) override;
    Gdiplus::RectF GetBoundingBox() override;
};
