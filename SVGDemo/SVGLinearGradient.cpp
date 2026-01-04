#include "stdafx.h"
#include "SVGLinearGradient.h"
#include "SVGDocument.h"
#include "SVGRadialGradient.h"
#include "SVGHelper.h"
#include "rapidxml.hpp"
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace Gdiplus;

// Helper: Hex2Int (Static để tránh lỗi duplicate symbol)
static int Hex2Int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

void SVGLinearGradient::Parse(rapidxml::xml_node<>* node, SVGDocument* doc)
{
    if (auto attr = node->first_attribute("id")) id = attr->value();

    if (auto attr = node->first_attribute("x1")) x1 = ParseUnit(attr->value());
    if (auto attr = node->first_attribute("y1")) y1 = ParseUnit(attr->value());
    if (auto attr = node->first_attribute("x2")) x2 = ParseUnit(attr->value());
    if (auto attr = node->first_attribute("y2")) y2 = ParseUnit(attr->value());

    if (auto attr = node->first_attribute("gradientUnits"))
        userSpace = string(attr->value()) == "userSpaceOnUse";

    // --- XỬ LÝ KẾ THỪA ---
    if (auto attr = node->first_attribute("xlink:href")) {
        string href = attr->value();
        if (!href.empty() && href[0] == '#' && doc) {
            string pid = href.substr(1);
            SVGLinearGradient* lin = doc->GetLinearGradient(pid);
            if (lin) this->stops = lin->GetStops();
            else {
                SVGRadialGradient* rad = doc->GetRadialGradient(pid);
                if (rad) this->stops = rad->GetStops(); // Lỗi LNK thường xảy ra ở đây nếu SVGRadialGradient thiếu GetStops
            }
        }
    }

    // --- XỬ LÝ TRANSFORM (SỬA LỖI ĐA GIÁC CHROME) ---
    if (auto attr = node->first_attribute("gradientTransform")) {
        string t = attr->value();
        transform.Reset();

        size_t pos = 0;
        while (pos < t.length()) {
            size_t start = t.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", pos);
            if (start == string::npos) break;
            size_t openParen = t.find('(', start);
            if (openParen == string::npos) break;
            string command = t.substr(start, openParen - start);
            size_t closeParen = t.find(')', openParen);
            if (closeParen == string::npos) break;
            string args = t.substr(openParen + 1, closeParen - openParen - 1);

            vector<float> vals;
            ParseNumberList(args.c_str(), vals);

            if (command == "matrix" && vals.size() >= 6) {
                Matrix m(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
                transform.Multiply(&m, MatrixOrderPrepend);
            }
            else if (command == "translate" && vals.size() >= 1) {
                transform.Translate(vals[0], (vals.size() > 1) ? vals[1] : 0, MatrixOrderPrepend);
            }
            else if (command == "scale" && vals.size() >= 1) {
                transform.Scale(vals[0], (vals.size() > 1) ? vals[1] : vals[0], MatrixOrderPrepend);
            }
            else if (command == "rotate" && vals.size() >= 1) {
                if (vals.size() >= 3) transform.RotateAt(vals[0], PointF(vals[1], vals[2]), MatrixOrderPrepend);
                else transform.Rotate(vals[0], MatrixOrderPrepend);
            }
            pos = closeParen + 1;
        }
    }

    // Parse Stops
    if (node->first_node("stop")) stops.clear(); // Xóa stop kế thừa nếu có

    for (auto* stop = node->first_node("stop"); stop; stop = stop->next_sibling("stop")) {
        SVGGradientStop gs; gs.offset = 0.0f;
        if (auto a = stop->first_attribute("offset")) gs.offset = ParseUnit(a->value());

        int r = 0, g = 0, b = 0, a = 255;
        if (auto c = stop->first_attribute("stop-color")) {
            string s = c->value();
            if (!s.empty() && s[0] == '#') {
                string hex = s.substr(1);
                if (hex.length() >= 6) {
                    r = stoi(hex.substr(0, 2), 0, 16); g = stoi(hex.substr(2, 2), 0, 16); b = stoi(hex.substr(4, 2), 0, 16);
                }
                else if (hex.length() >= 3) {
                    r = Hex2Int(hex[0]) * 17; g = Hex2Int(hex[1]) * 17; b = Hex2Int(hex[2]) * 17;
                }
            }
        }

        if (auto op = stop->first_attribute("stop-opacity")) {
            const char* ptr = op->value();
            a = (int)(255 * ParseNumber(ptr));
        }

        gs.color = Color(a, r, g, b);
        stops.push_back(gs);
    }
}

LinearGradientBrush* SVGLinearGradient::CreateBrush(const RectF& bounds) const
{
    PointF p1(x1, y1);
    PointF p2(x2, y2);

    // Fix lỗi GDI+ InvalidParameter nếu 2 điểm trùng nhau
    if (abs(p1.X - p2.X) < 0.0001f && abs(p1.Y - p2.Y) < 0.0001f) p2.X += 0.001f;

    auto* brush = new LinearGradientBrush(p1, p2, Color::Black, Color::White);

    Matrix m;

    // 1. Map toạ độ 0..1 vào Bounds (nếu cần)
    if (!userSpace) {
        m.Translate(bounds.X, bounds.Y);
        m.Scale(bounds.Width, bounds.Height);
    }

    // 2. Áp dụng Gradient Transform (Xoay, Nghiêng...)
    if (!transform.IsIdentity()) {
        m.Multiply(const_cast<Matrix*>(&transform), MatrixOrderPrepend);
    }

    brush->MultiplyTransform(&m);

    if (stops.size() >= 2) {
        vector<Color> colors; vector<REAL> positions;
        for (auto& s : stops) { colors.push_back(s.color); positions.push_back(s.offset); }

        if (positions.front() > 0.001f) { positions.insert(positions.begin(), 0.0f); colors.insert(colors.begin(), colors.front()); }
        if (positions.back() < 0.999f) { positions.push_back(1.0f); colors.push_back(colors.back()); }

        brush->SetInterpolationColors(colors.data(), positions.data(), (INT)colors.size());
    }
    return brush;
}