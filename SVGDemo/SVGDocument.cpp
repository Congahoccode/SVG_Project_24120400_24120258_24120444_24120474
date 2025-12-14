#include "stdafx.h"
#include "SVGDocument.h"
#include "SVGLinearGradient.h"

SVGDocument::~SVGDocument()
{
    for (auto& p : linearGradients)
        delete p.second;
}

void SVGDocument::AddLinearGradient(SVGLinearGradient* g)
{
    if (!g || g->GetId().empty()) return;
    linearGradients[g->GetId()] = g;
}

SVGLinearGradient* SVGDocument::GetLinearGradient(const std::string& id)
{
    auto it = linearGradients.find(id);
    return (it != linearGradients.end()) ? it->second : nullptr;
}
