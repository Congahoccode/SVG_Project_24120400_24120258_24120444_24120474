#pragma once
#include "SVGElement.h"
#include <vector>
#include <sstream>

class SVGPath : public SVGElement
{
private:
    Gdiplus::GraphicsPath path; // Đối tượng lưu trữ đường dẫn phức tạp

public:
    SVGPath() {}
    void Parse(rapidxml::xml_node<>* node) override;
    void Draw(Gdiplus::Graphics& g) override;
};
