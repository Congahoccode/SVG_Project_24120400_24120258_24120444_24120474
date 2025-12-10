#pragma once
#include <string>
#include <windows.h>
#include <gdiplus.h>
#pragma warning(push)
#pragma warning(disable : 26495) 
#include "rapidxml.hpp"
#pragma warning(pop)

using namespace std;
using namespace rapidxml;
using namespace Gdiplus;

class SVGElement 
{
protected:
    Color fillColor;
    float fillOpacity;
    Color strokeColor;
    float strokeOpacity;
    float strokeWidth;
    Matrix transform;

public:
    SVGElement()
        : fillColor(Color(0, 0, 0, 0)),
        fillOpacity(1.0f),
        strokeColor(Color(0, 0, 0, 0)),
        strokeOpacity(1.0f),
        strokeWidth(1.0f) {
		transform.Reset();
    }

    virtual ~SVGElement() {}

    virtual void InheritFrom(const SVGElement& parent); // Kế thừa node cha trong element svg group
    virtual void Parse(xml_node<>* node);
    virtual void Draw(Graphics& graphics) = 0;
};