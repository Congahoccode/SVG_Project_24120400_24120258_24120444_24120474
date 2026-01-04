#pragma once
#include "SVGElement.h"

class SVGEllipse : public SVGElement 
{
private:
    float cx, cy, rx, ry;

public:
    SVGEllipse() : cx(0), cy(0), rx(0), ry(0) {}
    void Parse(rapidxml::xml_node<>* node) override;
    void Draw(Gdiplus::Graphics& g) override;
    Gdiplus::RectF GetBoundingBox() override;
};
