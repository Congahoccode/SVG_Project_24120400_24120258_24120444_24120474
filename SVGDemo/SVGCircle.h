#pragma once
#include "stdafx.h"
#include "SVGElement.h"

class SVGCircle : public SVGElement 
{
private:
    float cx, cy, r;

public:
    SVGCircle() : cx(0), cy(0), r(0) {}
    void Parse(rapidxml::xml_node<>* node) override;
    void Draw(Gdiplus::Graphics& g) override;
    Gdiplus::RectF GetBoundingBox() override;
};
