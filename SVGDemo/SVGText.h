#pragma once
#include "SVGElement.h"
#include <string>
#include <vector>

class SVGText : public SVGElement
{
public:
    enum class TextAnchor {
        Start,
        Middle,
        End
    };

private:
    float x = 0.0f;
    float y = 0.0f;

    std::wstring content;
    std::wstring fontFamily = L"Times New Roman";
    float fontSize = 16.0f;

    Gdiplus::FontStyle fontStyle = Gdiplus::FontStyleRegular;
    TextAnchor anchor = TextAnchor::Start;

public:
    SVGText() = default;

    void Parse(rapidxml::xml_node<>* node) override;
    void Draw(Gdiplus::Graphics& g) override;
};
