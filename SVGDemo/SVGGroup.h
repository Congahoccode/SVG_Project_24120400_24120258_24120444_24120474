#pragma once
#include "stdafx.h"
#include "SVGElement.h"
#include <vector>
#include <string>
#include <gdiplus.h>
using namespace Gdiplus;
using namespace std;

class SVGGroup : public SVGElement
{
private:
    vector<SVGElement*> children;
public:
    SVGGroup() {};
    ~SVGGroup() override;

    void Parse(xml_node<>* node) override;

    void Draw(Graphics& g) override;
};