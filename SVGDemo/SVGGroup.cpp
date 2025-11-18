#include "stdafx.h"
#include "SVGGroup.h"


SVGGroup::~SVGGroup()
{
    for (auto child : children)
        delete child;
    children.clear();
}

void SVGGroup::AddChild(SVGElement* element)
{
    if (element)
        children.push_back(element);
}

const vector<SVGElement*>& SVGGroup::GetChildren() const
{
    return children;
}

void SVGGroup::SetTransform(const Matrix& transform)
{
    REAL m[6];
    transform.GetElements(m);
    localTransform.SetElements(m[0], m[1], m[2], m[3], m[4], m[5]);
}

const Matrix& SVGGroup::GetTransform() const
{
    return localTransform;
}

void SVGGroup::Draw(Graphics& g)
{
    // Lưu transform hiện tại
    Matrix prevTransform;
    g.GetTransform(&prevTransform);

    // Áp transform cục bộ của nhóm
    g.MultiplyTransform(&localTransform);

    // Duyệt và vẽ tất cả phần tử con
    for (auto child : children)
    {
        if (child)
            child->Draw(g);
    }

    // Khôi phục transform ban đầu
    g.SetTransform(&prevTransform);
}
