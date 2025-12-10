// File: SVGText.cpp
#include "stdafx.h"
#include "SVGText.h"
#include <iostream>

void SVGText::Parse(xml_node<>* node)
{
    SVGElement::Parse(node); // Lấy fill color và transform

    for (auto* a = node->first_attribute(); a; a = a->next_attribute())
    {
        string n = a->name();
        string v = a->value();
        if (n == "x") x = stof(v);
        else if (n == "y") y = stof(v);
        else if (n == "font-size") fontSize = stof(v);
        else if (n == "font-family")
        {
            // QUAN TRỌNG: Cắt chuỗi font tại dấu phẩy đầu tiên
            // SVG: "Times New Roman, sans-serif" -> GDI+ chỉ hiểu "Times New Roman"
            size_t commaPos = v.find(',');
            if (commaPos != string::npos) {
                v = v.substr(0, commaPos);
            }
            // Loại bỏ dấu ' nếu có (ví dụ 'Arial')
            if (!v.empty() && v.front() == '\'') v.erase(0, 1);
            if (!v.empty() && v.back() == '\'') v.pop_back();

            fontFamily = wstring(v.begin(), v.end());
        }
    }

    // Lấy nội dung text (Handle cả trường hợp text nằm trong node con)
    if (node->value() && strlen(node->value()) > 0) {
        string val = node->value();
        content = wstring(val.begin(), val.end());
    }
    else if (node->first_node() && node->first_node()->value()) {
        string val = node->first_node()->value();
        content = wstring(val.begin(), val.end());
    }
}

void SVGText::Draw(Graphics& g)
{
    GraphicsState state = g.Save();
    g.MultiplyTransform(&transform);

    SolidBrush brush(fillColor);

    // Nếu không tìm thấy font, fallback về Arial
    FontFamily family(fontFamily.c_str());
    if (family.GetLastStatus() != Ok) {
        FontFamily fallback(L"Arial");
        Font font(&fallback, fontSize, FontStyleRegular, UnitPixel);
        g.DrawString(content.c_str(), -1, &font, PointF(x, y - fontSize), &brush);
    }
    else {
        Font font(&family, fontSize, FontStyleRegular, UnitPixel);
        // SVG vẽ text từ baseline, GDI+ vẽ từ top-left
        // Ước lượng đơn giản: y - fontSize (để đẩy chữ lên trên baseline)
        g.DrawString(content.c_str(), -1, &font, PointF(x, y - fontSize), &brush);
    }

    g.Restore(state);
}