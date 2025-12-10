#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "rapidxml.hpp"
#include "SVGElement.h"
#include "SVGCircle.h"
#include "SVGEclipse.h"
#include "SVGRect.h"
#include "SVGLine.h"
#include "SVGPolygon.h"
#include "SVGPolyline.h"
#include "SVGText.h"
#include "SVGGroup.h"
#include "SVGPath.h"

class SVGParser 
{
private:
    std::vector<char> buffer;
    rapidxml::xml_document<> doc;
    std::vector<SVGElement*> elements;

public:
    bool ParseFile(const std::string& filePath);
    std::vector<SVGElement*>& GetElements() { return elements; }
	static SVGElement* CreateElement(rapidxml::xml_node<>* node); // Xóa private và đổi thành static để SVG group có thể gọi mà không cần tạo instance
};
