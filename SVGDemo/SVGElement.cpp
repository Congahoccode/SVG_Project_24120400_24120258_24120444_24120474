#include "stdafx.h"
#include "SVGElement.h"
#include "SVGDocument.h"
#include "SVGLinearGradient.h"
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <iostream>

using namespace std;
using namespace Gdiplus;

// --- Helper: Tách số từ chuỗi Transform ---
static void GetNumbers(const string& s, vector<float>& numbers)
{
    string temp = "";
    for (char c : s) {
        if (isdigit(c) || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E') temp += c;
        else if (!temp.empty()) {
            try { numbers.push_back(stof(temp)); }
            catch (...) {}
            temp = "";
        }
    }
    if (!temp.empty()) { try { numbers.push_back(stof(temp)); } catch (...) {} }
}

// --- HÀM TRA CỨU MÀU CHUẨN SVG/CSS (Full List) ---
static bool GetNamedColor(string name, Color& outColor)
{
    // 1. Chuyển tên về chữ thường để không phân biệt hoa thường (Red == red)
    transform(name.begin(), name.end(), name.begin(), ::tolower);

    // 2. Bảng màu chuẩn W3C (Static để chỉ khởi tạo 1 lần duy nhất)
    static const map<string, DWORD> colorMap = {
        {"aliceblue", 0xF0F8FF}, {"antiquewhite", 0xFAEBD7}, {"aqua", 0x00FFFF},
        {"aquamarine", 0x7FFFD4}, {"azure", 0xF0FFFF}, {"beige", 0xF5F5DC},
        {"bisque", 0xFFE4C4}, {"black", 0x000000}, {"blanchedalmond", 0xFFEBCD},
        {"blue", 0x0000FF}, {"blueviolet", 0x8A2BE2}, {"brown", 0xA52A2A},
        {"burlywood", 0xDEB887}, {"cadetblue", 0x5F9EA0}, {"chartreuse", 0x7FFF00},
        {"chocolate", 0xD2691E}, {"coral", 0xFF7F50}, {"cornflowerblue", 0x6495ED},
        {"cornsilk", 0xFFF8DC}, {"crimson", 0xDC143C}, {"cyan", 0x00FFFF},
        {"darkblue", 0x00008B}, {"darkcyan", 0x008B8B}, {"darkgoldenrod", 0xB8860B},
        {"darkgray", 0xA9A9A9}, {"darkgreen", 0x006400}, {"darkgrey", 0xA9A9A9},
        {"darkkhaki", 0xBDB76B}, {"darkmagenta", 0x8B008B}, {"darkolivegreen", 0x556B2F},
        {"darkorange", 0xFF8C00}, {"darkorchid", 0x9932CC}, {"darkred", 0x8B0000},
        {"darksalmon", 0xE9967A}, {"darkseagreen", 0x8FBC8F}, {"darkslateblue", 0x483D8B},
        {"darkslategray", 0x2F4F4F}, {"darkslategrey", 0x2F4F4F}, {"darkturquoise", 0x00CED1},
        {"darkviolet", 0x9400D3}, {"deeppink", 0xFF1493}, {"deepskyblue", 0x00BFFF},
        {"dimgray", 0x696969}, {"dimgrey", 0x696969}, {"dodgerblue", 0x1E90FF},
        {"firebrick", 0xB22222}, {"floralwhite", 0xFFFAF0}, {"forestgreen", 0x228B22},
        {"fuchsia", 0xFF00FF}, {"gainsboro", 0xDCDCDC}, {"ghostwhite", 0xF8F8FF},
        {"gold", 0xFFD700}, {"goldenrod", 0xDAA520}, {"gray", 0x808080},
        {"green", 0x008000}, {"greenyellow", 0xADFF2F}, {"grey", 0x808080},
        {"honeydew", 0xF0FFF0}, {"hotpink", 0xFF69B4}, {"indianred", 0xCD5C5C},
        {"indigo", 0x4B0082}, {"ivory", 0xFFFFF0}, {"khaki", 0xF0E68C},
        {"lavender", 0xE6E6FA}, {"lavenderblush", 0xFFF0F5}, {"lawngreen", 0x7CFC00},
        {"lemonchiffon", 0xFFFACD}, {"lightblue", 0xADD8E6}, {"lightcoral", 0xF08080},
        {"lightcyan", 0xE0FFFF}, {"lightgoldenrodyellow", 0xFAFAD2}, {"lightgray", 0xD3D3D3},
        {"lightgreen", 0x90EE90}, {"lightgrey", 0xD3D3D3}, {"lightpink", 0xFFB6C1},
        {"lightsalmon", 0xFFA07A}, {"lightseagreen", 0x20B2AA}, {"lightskyblue", 0x87CEFA},
        {"lightslategray", 0x778899}, {"lightslategrey", 0x778899}, {"lightsteelblue", 0xB0C4DE},
        {"lightyellow", 0xFFFFE0}, {"lime", 0x00FF00}, {"limegreen", 0x32CD32},
        {"linen", 0xFAF0E6}, {"magenta", 0xFF00FF}, {"maroon", 0x800000},
        {"mediumaquamarine", 0x66CDAA}, {"mediumblue", 0x0000CD}, {"mediumorchid", 0xBA55D3},
        {"mediumpurple", 0x9370DB}, {"mediumseagreen", 0x3CB371}, {"mediumslateblue", 0x7B68EE},
        {"mediumspringgreen", 0x00FA9A}, {"mediumturquoise", 0x48D1CC}, {"mediumvioletred", 0xC71585},
        {"midnightblue", 0x191970}, {"mintcream", 0xF5FFFA}, {"mistyrose", 0xFFE4E1},
        {"moccasin", 0xFFE4B5}, {"navajowhite", 0xFFDEAD}, {"navy", 0x000080},
        {"oldlace", 0xFDF5E6}, {"olive", 0x808000}, {"olivedrab", 0x6B8E23},
        {"orange", 0xFFA500}, {"orangered", 0xFF4500}, {"orchid", 0xDA70D6},
        {"palegoldenrod", 0xEEE8AA}, {"palegreen", 0x98FB98}, {"paleturquoise", 0xAFEEEE},
        {"palevioletred", 0xDB7093}, {"papayawhip", 0xFFEFD5}, {"peachpuff", 0xFFDAB9},
        {"peru", 0xCD853F}, {"pink", 0xFFC0CB}, {"plum", 0xDDA0DD},
        {"powderblue", 0xB0E0E6}, {"purple", 0x800080}, {"rebeccapurple", 0x663399},
        {"red", 0xFF0000}, {"rosybrown", 0xBC8F8F}, {"royalblue", 0x4169E1},
        {"saddlebrown", 0x8B4513}, {"salmon", 0xFA8072}, {"sandybrown", 0xF4A460},
        {"seagreen", 0x2E8B57}, {"seashell", 0xFFF5EE}, {"sienna", 0xA0522D},
        {"silver", 0xC0C0C0}, {"skyblue", 0x87CEEB}, {"slateblue", 0x6A5ACD},
        {"slategray", 0x708090}, {"slategrey", 0x708090}, {"snow", 0xFFFAFA},
        {"springgreen", 0x00FF7F}, {"steelblue", 0x4682B4}, {"tan", 0xD2B48C},
        {"teal", 0x008080}, {"thistle", 0xD8BFD8}, {"tomato", 0xFF6347},
        {"turquoise", 0x40E0D0}, {"violet", 0xEE82EE}, {"wheat", 0xF5DEB3},
        {"white", 0xFFFFFF}, {"whitesmoke", 0xF5F5F5}, {"yellow", 0xFFFF00},
        {"yellowgreen", 0x9ACD32}
    };

    // Tìm kiếm trong map (Tốc độ cực nhanh)
    auto it = colorMap.find(name);
    if (it != colorMap.end()) {
        // Tách Hex 0xRRGGBB thành R, G, B
        DWORD hex = it->second;
        BYTE r = (hex >> 16) & 0xFF;
        BYTE g = (hex >> 8) & 0xFF;
        BYTE b = hex & 0xFF;

        // Trả về color với Alpha mặc định là 255 (sẽ được override bởi opacity sau)
        outColor = Color(255, r, g, b);
        return true;
    }

    return false;
}

// --- Helper: Chuyển Hex sang Int thủ công ---
static int Hex2Int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

void SVGElement::Parse(xml_node<>* node)
{
    // 1. Parse Opacity
    if (auto attr = node->first_attribute("fill-opacity"))
        fillOpacity = (float)atof(attr->value());
    if (auto attr = node->first_attribute("stroke-opacity"))
        strokeOpacity = (float)atof(attr->value());

    // 2. Parse Fill Color
    // ===== PARSE FILL =====
    if (auto attr = node->first_attribute("fill"))
    {
        string s = attr->value();

        // --- Trim whitespace ---
        while (!s.empty() && isspace((unsigned char)s.front())) s.erase(0, 1);
        while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();

        // ===== 1. fill="none" =====
        if (s == "none") {
            fillType = FillType::None;
            fillColor = Color(255, 0, 0, 0);
        }

        // ===== 2. fill="url(#id)" → Linear Gradient =====
        else if (s.rfind("url(", 0) == 0)
        {
            fillType = FillType::LinearGradient;

            size_t start = s.find('#');
            size_t end = s.find(')', start);

            if (start != string::npos && end != string::npos && document)
            {
                string id = s.substr(start + 1, end - start - 1);
                fillGradient = document->GetLinearGradient(id);
            }
        }

        // ===== 3. fill="#RRGGBB" hoặc "#RGB" =====
        else if (!s.empty() && s[0] == '#') {
            fillType = FillType::Solid;

            string hex = s.substr(1);
            int r = 0, g = 0, b = 0;

            if (hex.length() >= 6) {
                r = Hex2Int(hex[0]) * 16 + Hex2Int(hex[1]);
                g = Hex2Int(hex[2]) * 16 + Hex2Int(hex[3]);
                b = Hex2Int(hex[4]) * 16 + Hex2Int(hex[5]);
            }
            else if (hex.length() >= 3) {
                r = Hex2Int(hex[0]) * 17;
                g = Hex2Int(hex[1]) * 17;
                b = Hex2Int(hex[2]) * 17;
            }

            fillColor = Color(
                (BYTE)(fillOpacity * 255),
                r, g, b
            );
        }

        // ===== 4. fill="rgb(r,g,b)" =====
        else if (s.rfind("rgb", 0) == 0) {
            fillType = FillType::Solid;

            vector<float> vals;
            GetNumbers(s, vals); // lấy r, g, b

            if (vals.size() >= 3) {
                fillColor = Color(
                    (BYTE)(fillOpacity * 255),
                    (BYTE)vals[0],
                    (BYTE)vals[1],
                    (BYTE)vals[2]
                );
            }
        }
        // 5. NameColor

        else
        {
            Color namedC;
            // Gọi hàm tra cứu bảng màu (đã thêm ở trên)
            if (GetNamedColor(s, namedC))
            {
                fillType = FillType::Solid;
                // Kết hợp màu tìm được với độ trong suốt hiện tại
                fillColor = Color(
                    (BYTE)(fillOpacity * 255),
                    namedC.GetR(),
                    namedC.GetG(),
                    namedC.GetB()
                );
            }
            else
            {
                // Nếu không tìm thấy màu nào -> Fallback về None (Không tô)
                fillType = FillType::Unset;
                fillColor = Color(0, 0, 0, 0);
            }
        }
    }


    // 3. Parse Stroke Color
    if (auto attr = node->first_attribute("stroke"))
    {
        string s = attr->value();

        // Trim whitespace
        while (!s.empty() && isspace((unsigned char)s.front())) s.erase(0, 1);
        while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();

        // ===== 1. stroke="none" =====
        if (s == "none")
        {
            strokeColor = Color(0, 0, 0, 0);
        }

        // ===== 2. stroke="#RRGGBB" / "#RGB" =====
        else if (!s.empty() && s[0] == '#')
        {
            string hex = s.substr(1);
            int r = 0, g = 0, b = 0;

            if (hex.length() >= 6) {
                r = Hex2Int(hex[0]) * 16 + Hex2Int(hex[1]);
                g = Hex2Int(hex[2]) * 16 + Hex2Int(hex[3]);
                b = Hex2Int(hex[4]) * 16 + Hex2Int(hex[5]);
            }
            else if (hex.length() >= 3) {
                r = Hex2Int(hex[0]) * 17;
                g = Hex2Int(hex[1]) * 17;
                b = Hex2Int(hex[2]) * 17;
            }

            strokeColor = Color(
                (BYTE)(strokeOpacity * 255),
                r, g, b
            );
        }

        // ===== 3. stroke="rgb(r,g,b)" =====
        else if (s.rfind("rgb", 0) == 0)
        {
            vector<float> vals;
            GetNumbers(s, vals);

            if (vals.size() >= 3)
            {
                strokeColor = Color(
                    (BYTE)(strokeOpacity * 255),
                    (BYTE)vals[0],
                    (BYTE)vals[1],
                    (BYTE)vals[2]
                );
            }
        }

        // ===== 4. stroke="black", "red", ... =====
        else
        {
            Color namedC;
            if (GetNamedColor(s, namedC))
            {
                strokeColor = Color(
                    (BYTE)(strokeOpacity * 255),
                    namedC.GetR(),
                    namedC.GetG(),
                    namedC.GetB()
                );
            }
            else
            {
                // Fallback: không vẽ stroke
                strokeColor = Color(0, 0, 0, 0);
            }
        }
    }


    if (auto attr = node->first_attribute("stroke-width"))
        strokeWidth = (float)atof(attr->value());

    if (auto attr = node->first_attribute("stroke-miterlimit"))
        strokeMiterLimit = atof(attr->value());


    // 4. Transform (Không đổi)
    if (auto attr = node->first_attribute("transform"))
    {
        string t = attr->value();
        size_t pos = 0;
        while (pos < t.length()) {
            size_t start = t.find_first_of("abcdefghijklmnopqrstuvwxyz", pos);
            if (start == string::npos) break;
            size_t openParen = t.find('(', start);
            if (openParen == string::npos) break;
            string command = t.substr(start, openParen - start);
            size_t closeParen = t.find(')', openParen);
            if (closeParen == string::npos) break;
            string args = t.substr(openParen + 1, closeParen - openParen - 1);
            vector<float> vals;
            GetNumbers(args, vals);

            if (command == "translate" && vals.size() >= 1) {
                float dx = vals[0];
                float dy = (vals.size() > 1) ? vals[1] : 0;
                transform.Translate(dx, dy, MatrixOrderPrepend);
            }
            else if (command == "rotate" && vals.size() >= 1) {
                transform.Rotate(vals[0], MatrixOrderPrepend);
            }
            else if (command == "scale" && vals.size() >= 1) {
                float sx = vals[0];
                float sy = (vals.size() > 1) ? vals[1] : sx;
                transform.Scale(sx, sy, MatrixOrderPrepend);
            }
            pos = closeParen + 1;
        }
    }
}

void SVGElement::InheritFrom(const SVGElement& parent)
{
    if (!this->document)
        this->document = parent.document;

    // ===== FILL =====
    if (this->fillType == FillType::Unset && parent.fillType != FillType::Unset)
    {
        this->fillType = parent.fillType;
        this->fillColor = parent.fillColor;
        this->fillOpacity = parent.fillOpacity;
        this->fillGradient = parent.fillGradient;
    }

    // ===== STROKE =====
    if (this->strokeColor.GetA() == 0 && parent.strokeColor.GetA() > 0)
    {
        this->strokeColor = parent.strokeColor;
        this->strokeWidth = parent.strokeWidth;
        this->strokeOpacity = parent.strokeOpacity;
        this->strokeMiterLimit = parent.strokeMiterLimit;
    }
}

Brush* SVGElement::CreateFillBrush(const RectF& bounds)
{
    if (fillType == FillType::None) return nullptr;

    // ===== SOLID COLOR =====
    if (fillType == FillType::Solid)
    {
        return new SolidBrush(fillColor);
    }

    // ===== LINEAR GRADIENT =====
    if (fillType == FillType::LinearGradient && fillGradient)
    {
        return fillGradient->CreateBrush(bounds);
    }

    return nullptr;
}

Pen* SVGElement::CreateStrokePen()
{
    if (strokeWidth <= 0 || strokeColor.GetA() == 0)
        return nullptr;

    Pen* pen = new Pen(strokeColor, strokeWidth);
    pen->SetMiterLimit(strokeMiterLimit);
    return pen;
}
