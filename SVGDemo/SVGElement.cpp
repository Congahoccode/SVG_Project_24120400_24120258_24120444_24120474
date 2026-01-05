#include "stdafx.h"
#include "SVGElement.h"
#include "SVGDocument.h"
#include "SVGLinearGradient.h"
#include "SVGRadialGradient.h"
#include "SVGHelper.h"
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <sstream> 
#include <iostream>

using namespace std;
using namespace Gdiplus;

// Constructor
SVGElement::SVGElement()
    : fillColor(Color(255, 0, 0, 0)),
    fillOpacity(1.0f),
    strokeColor(Color(0, 0, 0, 0)),
    strokeOpacity(1.0f),
    strokeWidth(1.0f),
    strokeMiterLimit(4.0f),
    strokeLineCap(LineCapFlat),
    strokeLineJoin(LineJoinMiter),
    strokeColorSet(false),
    strokeWidthSet(false)
{
    document = nullptr;
    fillGradient = nullptr;
    fillRadialGradient = nullptr;
    fillType = FillType::Unset;
    transform.Reset();
}

void SVGElement::Parse(xml_node<>* node)
{
    // 1. Xử lý ID và Đăng ký Element
    if (auto a = node->first_attribute("id")) {
        id = a->value();
        if (document) document->RegisterElement(id, this);
    }

    // 2. Xử lý HREF (cho text path hoặc use)
    if (auto a = node->first_attribute("xlink:href")) {
        string val = a->value();
        if (!val.empty() && val[0] == '#') href = val.substr(1);
        else href = val;
    }
    else if (auto a = node->first_attribute("href")) {
        string val = a->value();
        if (!val.empty() && val[0] == '#') href = val.substr(1);
        else href = val;
    }

    // 3. TỔNG HỢP THUỘC TÍNH (Attribute + Style)
    map<string, string> attrs;

    // a. Đọc Attribute trước
    for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
        attrs[attr->name()] = attr->value();
    }

    // b. Đọc Style đè lên (nếu có)
    if (auto attr = node->first_attribute("style")) {
        string style = attr->value();
        stringstream ss(style);
        string segment;
        while (getline(ss, segment, ';')) {
            size_t colon = segment.find(':');
            if (colon != string::npos) {
                string key = segment.substr(0, colon);
                string val = segment.substr(colon + 1);

                // Trim khoảng trắng
                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                val.erase(0, val.find_first_not_of(" \t\r\n"));
                val.erase(val.find_last_not_of(" \t\r\n") + 1);

                attrs[key] = val;
            }
        }
    }

    // 4. XỬ LÝ GIÁ TRỊ TỪ MAP
    if (attrs.count("clip-path")) {
        string val = attrs["clip-path"];
        size_t start = val.find("url(#");
        if (start != string::npos) {
            size_t end = val.find(")", start);
            if (end != string::npos) clipPathId = val.substr(start + 5, end - (start + 5));
        }
    }

    // --- Stroke Width & Style ---
    if (attrs.count("stroke-width")) {
        strokeWidth = ParseUnit(attrs["stroke-width"]);
        strokeWidthSet = true;
    }
    if (attrs.count("stroke-miterlimit")) {
        const char* ptr = attrs["stroke-miterlimit"].c_str();
        strokeMiterLimit = ParseNumber(ptr);
    }
    if (attrs.count("stroke-linecap")) {
        string v = attrs["stroke-linecap"];
        if (v == "round") strokeLineCap = LineCapRound;
        else if (v == "square") strokeLineCap = LineCapSquare;
        else strokeLineCap = LineCapFlat;
    }
    if (attrs.count("stroke-linejoin")) {
        string v = attrs["stroke-linejoin"];
        if (v == "round") strokeLineJoin = LineJoinRound;
        else if (v == "bevel") strokeLineJoin = LineJoinBevel;
        else strokeLineJoin = LineJoinMiter;
    }
    if (attrs.count("fill-rule")) {
        string v = attrs["fill-rule"];
        if (v == "evenodd") fillRule = FillModeAlternate;
        else if (v == "nonzero") fillRule = FillModeWinding;
    }

    // --- Fill & Stroke (Màu sắc) ---
    if (attrs.count("fill")) ParseFillValue(attrs["fill"]);
    if (attrs.count("stroke")) ParseStrokeValue(attrs["stroke"]);

    // --- Opacity (Xử lý cẩn thận để tránh lỗi chồng màu) ---
    float baseOpacity = 1.0f;
    if (attrs.count("opacity")) {
        const char* ptr = attrs["opacity"].c_str();
        baseOpacity = ParseNumber(ptr);
    }

    if (attrs.count("fill-opacity")) {
        const char* ptr = attrs["fill-opacity"].c_str();
        fillOpacity = ParseNumber(ptr);
    }

    // Stroke Opacity
    if (attrs.count("stroke-opacity")) {
        const char* ptr = attrs["stroke-opacity"].c_str();
        strokeOpacity = ParseNumber(ptr);
    }


    // --- Transform ---
    if (attrs.count("transform")) {
        ParseTransform(attrs["transform"]);
    }

}

void SVGElement::ParseFillValue(const string& value)
{
    if (value == "none") {
        fillType = FillType::None;
    }
    else if (value.find("url(#") == 0 && document) {
        size_t start = value.find('#');
        size_t end = value.find(')', start);
        if (start != string::npos) {
            string id = value.substr(start + 1, end - start - 1);
            fillGradient = document->GetLinearGradient(id);
            fillRadialGradient = document->GetRadialGradient(id);
            if (fillGradient) fillType = FillType::LinearGradient;
            else if (fillRadialGradient) fillType = FillType::RadialGradient;
        }
    }
    else {
        fillType = FillType::Solid;
        fillColor = ParseColor(value);
    }
}

void SVGElement::ParseStrokeValue(const string& value)
{
    strokeColorSet = true;
    if (value == "none") {
        strokeColor = Color(0, 0, 0, 0);
        strokeWidth = 0.0f;
    }
    else {
        strokeColor = ParseColor(value);
        if (strokeWidth == 0.0f) strokeWidth = 1.0f;
    }
}

void SVGElement::ParseTransform(const string& value)
{
    ParseTransformString(value, transform);
}

void SVGElement::InheritFrom(const SVGElement& parent)
{
    if (!this->document) this->document = parent.document;

    this->fillOpacity = parent.fillOpacity;
    this->strokeOpacity = parent.strokeOpacity;

    if (this->fillType == FillType::Unset && parent.fillType != FillType::Unset) {
        this->fillType = parent.fillType;
        this->fillColor = parent.fillColor;
        this->fillGradient = parent.fillGradient;
        this->fillRadialGradient = parent.fillRadialGradient;
    }

    if (!this->strokeColorSet && parent.strokeColorSet) {
        this->strokeColor = parent.strokeColor;
        this->strokeLineCap = parent.strokeLineCap;
        this->strokeLineJoin = parent.strokeLineJoin;
        this->strokeColorSet = true;
    }

    if (!this->strokeWidthSet && parent.strokeWidthSet) {
        this->strokeWidth = parent.strokeWidth;
        this->strokeMiterLimit = parent.strokeMiterLimit;
		this->strokeWidthSet = true;
    }
}

Brush* SVGElement::CreateFillBrush(const RectF& bounds)
{
    if (fillType == FillType::None) return nullptr;

    if (fillType == FillType::Unset)
        return new SolidBrush(Color((BYTE)(fillOpacity * 255), 0, 0, 0));

    if (fillType == FillType::Solid) {
        // Áp dụng Alpha duy nhất tại đây: Alpha gốc của màu * fillOpacity
        BYTE a = (BYTE)(fillColor.GetA() * fillOpacity);
        return new SolidBrush(Color(a, fillColor.GetR(), fillColor.GetG(), fillColor.GetB()));
    }

    if (fillType == FillType::LinearGradient && fillGradient) return fillGradient->CreateBrush(bounds);
    if (fillType == FillType::RadialGradient && fillRadialGradient) return fillRadialGradient->CreateBrush(bounds);
    return nullptr;
}

Pen* SVGElement::CreateStrokePen()
{
    if (strokeWidth <= 0 || strokeColor.GetA() == 0) return nullptr;

    BYTE a = (BYTE)(strokeColor.GetA() * strokeOpacity);
    if (a == 0) return nullptr;

    Color finalColor(a, strokeColor.GetR(), strokeColor.GetG(), strokeColor.GetB());
    Pen* pen = new Pen(finalColor, strokeWidth);
    pen->SetMiterLimit(strokeMiterLimit);
    pen->SetLineCap(strokeLineCap, strokeLineCap, DashCapFlat);
    pen->SetLineJoin(strokeLineJoin);
    return pen;
}

void SVGElement::ApplyClip(Graphics& g)
{
    if (clipPathId.empty() || !document) return;
    SVGElement* clipEl = document->GetElementById(clipPathId);
    if (!clipEl) return;
    GraphicsPath* path = clipEl->GetGraphicsPath();
    if (path) {
        g.SetClip(path, CombineModeIntersect);
        delete path;
    }
}