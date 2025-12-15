#include "stdafx.h"
#include "SVGText.h"
#include <sstream>

using namespace Gdiplus;
using namespace std;

// Helper: Chuẩn hóa khoảng trắng trong chuỗi
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


void SVGText::Parse(xml_node<>* node)
{
    // Parse fill, stroke, transform
    SVGElement::Parse(node);

    for (auto* a = node->first_attribute(); a; a = a->next_attribute())
    {
        string name = a->name();
        string val = a->value();

        if (name == "x") x = stof(val);
        else if (name == "y") y = stof(val);
        else if (name == "font-size") fontSize = stof(val);

        else if (name == "font-family")
        {
            // SVG: "Times New Roman, sans-serif"
            size_t comma = val.find(',');
            if (comma != string::npos)
                val = val.substr(0, comma);

            if (!val.empty() && val.front() == '\'') val.erase(0, 1);
            if (!val.empty() && val.back() == '\'')  val.pop_back();

            fontFamily = wstring(val.begin(), val.end());
        }

        else if (name == "font-style")
        {
            if (val == "italic" || val == "oblique")
                fontStyle = FontStyleItalic;
            else
                fontStyle = FontStyleRegular;
        }

        else if (name == "text-anchor")
        {
            if (val == "middle") anchor = TextAnchor::Middle;
            else if (val == "end") anchor = TextAnchor::End;
            else anchor = TextAnchor::Start;
        }
    }

    // === Parse text content (support multiline) ===
    content.clear();

    if (node->value() && strlen(node->value()) > 0)
    {
        string v = NormalizeWhitespace(node->value());
        content.assign(v.begin(), v.end());
    }
    else if (node->first_node() && node->first_node()->value())
    {
        string v = NormalizeWhitespace(node->first_node()->value());
        content.assign(v.begin(), v.end());
    }
}

void SVGText::Draw(Graphics& g)
{
    GraphicsState state = g.Save();
    g.MultiplyTransform(&transform);

    FontFamily primary(fontFamily.c_str());
    FontFamily fallback(L"Arial");

    FontFamily* family =
        (primary.GetLastStatus() == Ok) ? &primary : &fallback;

    Font font(family, fontSize, fontStyle, UnitPixel);

    // === Split multiline ===
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
        g.MeasureString(ln.c_str(), -1, &font, PointF(0, 0), &layout);

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
        if (Pen* pen = CreateStrokePen())
        {
            g.DrawPath(pen, &path);
            delete pen;
        }

        currentY += fontSize * 1.2f;
    }

    g.Restore(state);
}
