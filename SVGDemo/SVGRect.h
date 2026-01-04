#pragma once
#include "SVGElement.h"

class SVGRect : public SVGElement 
{
private:
    float x, y, width, height;

public:
    SVGRect() : x(0), y(0), width(0), height(0) {}
    void Parse(rapidxml::xml_node<>* node) override;
    void Draw(Gdiplus::Graphics& g) override;
    Gdiplus::RectF GetBoundingBox() override;
};