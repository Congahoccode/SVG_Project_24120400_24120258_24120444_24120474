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
    // Parse thuộc tính của group (fill, stroke, transform)
    SVGElement::Parse(node);

    for (auto* child = node->first_node(); child; child = child->next_sibling())
    {
        SVGElement* element = SVGParser::CreateElement(child);
        if (!element) continue;

        // Truyền document
        element->SetDocument(this->document);

        // INHERIT TRƯỚC (fallback)
        element->InheritFrom(*this);

        // Parse thuộc tính riêng (override)
        element->Parse(child);

        children.push_back(element);
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

RectF SVGGroup::GetBoundingBox()
{
    RectF totalRect(0, 0, 0, 0);
    bool first = true;

    for (auto* c : children)
    {
        RectF r = c->GetBoundingBox();

        // Bỏ qua hình rỗng
        if (r.Width <= 0 || r.Height <= 0) continue;

        if (first) {
            totalRect = r;
            first = false;
        }
        else {
            // Hợp nhất hình chữ nhật
            totalRect.Union(totalRect, totalRect, r);
        }
    }

    // Nếu Group có transform (di chuyển/scale), ta phải áp dụng lên khung bao tổng
    if (!transform.IsIdentity())
    {
        GraphicsPath gp;
        gp.AddRectangle(totalRect);
        gp.Transform(&transform);
        gp.GetBounds(&totalRect);
    }

    return totalRect;
}