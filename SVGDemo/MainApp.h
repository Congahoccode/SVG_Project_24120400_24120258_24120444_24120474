#pragma once

#include <vector>
#include <string>

#include "SVGParser.h"
#include "SVGRenderer.h"
#include "SVGElement.h"


namespace Gdiplus 
{
    class Graphics;
}

class MainApp
{
private:
    SVGParser parser;
    SVGRenderer renderer;
    std::vector<SVGElement*> elements;

public:
    MainApp() = default;
    ~MainApp();

    // Đọc file SVG, trả về true nếu thành công
    bool LoadSVG(const std::string& filePath);

    // Vẽ toàn bộ phần tử SVG
    void Render(Gdiplus::Graphics& g);

    // Giải phóng bộ nhớ
    void Clear();

	// Lấy bộ kết xuất
	SVGRenderer& GetRenderer() { return renderer; }
    const vector<SVGElement*>& GetElements() const { return parser.GetElements(); }
};
