#include "stdafx.h"
#include "SVGParser.h"
#include "SVGLinearGradient.h"
#include <iostream>

bool SVGParser::ParseFile(const std::string& filePath) 
{
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    buffer = std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');

    doc.parse<0>(&buffer[0]);
    rapidxml::xml_node<>* root = doc.first_node("svg");
    if (!root) return false;

	// Parse defs first
    if (auto* defs = root->first_node("defs"))
    {
        for (auto* node = defs->first_node(); node; node = node->next_sibling())
        {
            std::string name = node->name();

            if (name == "linearGradient")
            {
                SVGLinearGradient* grad = new SVGLinearGradient();
                grad->Parse(node);
                document.AddLinearGradient(grad);
            }
        }
    }

	// Parse các phần tử con của <svg>
    for (auto* node = root->first_node(); node; node = node->next_sibling()) 
    {
        SVGElement* element = CreateElement(node);
        if (element) 
        {
			element->SetDocument(&document);
            element->Parse(node);
            elements.push_back(element);
        }
    }
    return true;
}

SVGElement* SVGParser::CreateElement(rapidxml::xml_node<>* node) 
{
    std::string name = node->name();

    if (name == "rect") return new SVGRect();
    if (name == "circle") return new SVGCircle();
    if (name == "ellipse") return new SVGEllipse();
    if (name == "line") return new SVGLine();
    if (name == "polygon") return new SVGPolygon();
    if (name == "polyline") return new SVGPolyline();
    if (name == "text") return new SVGText();
    if (name == "g") return new SVGGroup();
	if (name == "path") return new SVGPath();
    return nullptr;
}
