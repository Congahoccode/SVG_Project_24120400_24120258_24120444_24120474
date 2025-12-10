#include "stdafx.h"
#include "SVGParser.h"
#include "SVGGroup.h"


SVGGroup::~SVGGroup()
{
    for (auto child : children)
        delete child;
    children.clear();
}

void SVGGroup::Parse(xml_node<>* node)
{
    // tự gọi Parse() của SVGElement -> lấy fill/stroke/transform của nhóm
    SVGElement::Parse(node);

    // Parse các element con
    for (auto* child = node->first_node(); child; child = child->next_sibling())
    {
        SVGElement* element = SVGParser::CreateElement(child);
        if (element) {
            element->InheritFrom(*this);  // kế thừa fill/stroke
            element->Parse(child);        // parse thuộc tính riêng
            children.push_back(element);
        }
    }
}

void SVGGroup::Draw(Graphics& g)
{
    auto state = g.Save();

    // áp dụng transform của group
    g.MultiplyTransform(&transform);

    // vẽ các children
    for (auto* c : children) {
        c->Draw(g);
    }

    g.Restore(state);
}
