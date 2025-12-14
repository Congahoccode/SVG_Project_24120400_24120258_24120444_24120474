#pragma once
#pragma once
#include <string>
#include <vector>
#include <gdiplus.h>

using namespace Gdiplus;
using namespace std;

struct SVGGradientStop
{
    float offset;     // [0..1]
    Color color;      // ARGB
};

class SVGLinearGradient
{
private:
    string id;

    float x1 = 0, y1 = 0;
    float x2 = 1, y2 = 0;

    bool userSpace = false;

    vector<SVGGradientStop> stops;

public:
    SVGLinearGradient() {}

    void SetId(const string& _id) { id = _id; }
    const string& GetId() const { return id; }

    void Parse(rapidxml::xml_node<>* node);

    LinearGradientBrush* CreateBrush(const RectF& bounds) const;
};
