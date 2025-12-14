#pragma once
#include <map>
#include <string>

class SVGLinearGradient;

class SVGDocument
{
private:
    std::map<std::string, SVGLinearGradient*> linearGradients;

public:
    ~SVGDocument();

    void AddLinearGradient(SVGLinearGradient* g);
    SVGLinearGradient* GetLinearGradient(const std::string& id);
};
