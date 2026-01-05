#include "stdafx.h"
#include "SVGText.h"
#include "SVGHelper.h"
#include <sstream>

using namespace Gdiplus;
using namespace std;

// Chuẩn hóa khoảng trắng trong chuỗi
static string NormalizeWhitespace(const string& s)
{
    string out;
    bool inSpace = false;

    for (char c : s)
    {
        if (isspace((unsigned char)c))
        {
            if (!inSpace)
            {
                out += ' ';
                inSpace = true;
            }
        }
        else
        {
            out += c;
            inSpace = false;
        }
    }

    if (!out.empty() && out.front() == ' ') out.erase(0, 1);
    if (!out.empty() && out.back() == ' ') out.pop_back();

    return out;
}


void SVGText::Parse(rapidxml::xml_node<>* node)
{
    // Parse fill, stroke, transform
    SVGElement::Parse(node);

    for (auto* a = node->first_attribute(); a; a = a->next_attribute())
    {
        string name = a->name();
        string val = a->value();

        if (name == "x") x = ParseUnit(val);
        else if (name == "y") y = ParseUnit(val);
        else if (name == "font-family") {
            fontFamily = wstring(val.begin(), val.end());
        }
        else if (name == "font-size") {
            fontSize = ParseUnit(val);
        }
        else if (name == "font-weight") {
            if (val == "bold") fontStyle = (FontStyle)(fontStyle | FontStyleBold);
        }
        else if (name == "font-style") {
            if (val == "italic") fontStyle = (FontStyle)(fontStyle | FontStyleItalic);
        }
        else if (name == "text-anchor") {
            if (val == "middle") anchor = TextAnchor::Middle;
            else if (val == "end") anchor = TextAnchor::End;
        }
    }

    // Lấy nội dung text
    string rawText = node->value();
    string norm = NormalizeWhitespace(rawText);
    content = wstring(norm.begin(), norm.end());
}

void SVGText::Draw(Gdiplus::Graphics& g)
{
    if (content.empty()) return;

    GraphicsState state = g.Save();
    g.MultiplyTransform(&transform);

    FontFamily primary(fontFamily.c_str());
    FontFamily fallback(L"Arial");

    FontFamily* family =
        (primary.GetLastStatus() == Ok) ? &primary : &fallback;

    Font font(family, fontSize, fontStyle, UnitPixel);

    // Split multiline
    vector<wstring> lines;
    wstringstream ss(content);
    wstring line;
    while (getline(ss, line))
        lines.push_back(line);

    float currentY = y;

    for (const auto& ln : lines)
    {
        // Measure text
        RectF layout;
        g.MeasureString(ln.c_str(), -1, &font, PointF(0, 0), StringFormat::GenericTypographic(), &layout);

        float drawX = x;
        if (anchor == TextAnchor::Middle)
            drawX -= layout.Width / 2;
        else if (anchor == TextAnchor::End)
            drawX -= layout.Width;

        GraphicsPath path;
        path.AddString(
            ln.c_str(),
            -1,
            family,
            fontStyle,
            fontSize,
            PointF(drawX, currentY - fontSize),
            nullptr
        );

        // Fill text
        if (Brush* fill = CreateFillBrush(layout))
        {
            g.FillPath(fill, &path);
            delete fill;
        }

        // Stroke text
        if (Pen* stroke = CreateStrokePen())
        {
            g.DrawPath(stroke, &path);
            delete stroke;
        }
    }

    g.Restore(state);
}

Gdiplus::RectF SVGText::GetBoundingBox()
{
    if (content.empty()) return RectF(0, 0, 0, 0);
    HDC hdc = GetDC(NULL);
    Graphics g(hdc);

    FontFamily primary(fontFamily.c_str());
    FontFamily fallback(L"Arial");
    FontFamily* family = (primary.GetLastStatus() == Ok) ? &primary : &fallback;
    Font font(family, fontSize, fontStyle, UnitPixel);

    GraphicsPath path;

    vector<wstring> lines;
    wstringstream ss(content);
    wstring line;
    while (getline(ss, line)) lines.push_back(line);

    float currentY = y;

    for (const auto& ln : lines)
    {
        RectF layout;
        g.MeasureString(ln.c_str(), -1, &font, PointF(0, 0), StringFormat::GenericTypographic(), &layout);

        path.AddString(
            ln.c_str(), -1, family, fontStyle, fontSize,
            layout,
            StringFormat::GenericTypographic()
        );
    }

    ReleaseDC(NULL, hdc);
    RectF bounds;
    path.GetBounds(&bounds);
    return bounds;
}