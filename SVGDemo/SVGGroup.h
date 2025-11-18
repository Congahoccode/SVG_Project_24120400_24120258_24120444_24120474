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
    Matrix localTransform;
public:
    SVGGroup() {};
    ~SVGGroup() override;

    void AddChild(SVGElement* element);
    const vector<SVGElement*>& GetChildren() const;

    void SetTransform(const Matrix& transform);
    const Matrix& GetTransform() const;

    void Draw(Graphics& g) override;
};