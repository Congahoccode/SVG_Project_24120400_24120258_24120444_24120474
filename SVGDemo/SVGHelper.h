#pragma once
#include <string>
#include <locale.h>
#include <vector>
#include <gdiplus.h>
#include <map>
#include <algorithm>
#include <cmath>

// Tạo locale chuẩn C (dùng dấu chấm)
static _locale_t c_locale = _create_locale(LC_NUMERIC, "C");

// Hàm đọc số thực chuẩn xác
inline float ParseNumber(const char*& ptr)
{
    while (*ptr && (*ptr == ' ' || *ptr == ',' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) ptr++;

    if (*ptr == '\0') return 0.0f;

    char* endPtr;
    double val = _strtod_l(ptr, &endPtr, c_locale);

    if (ptr == endPtr) return 0.0f;
    ptr = endPtr;
    return (float)val;
}

inline void ParseNumberList(const char* s, std::vector<float>& numbers)
{
    const char* ptr = s;
    while (*ptr) {
        while (*ptr && !isdigit(*ptr) && *ptr != '-' && *ptr != '.' && *ptr != '+') ptr++;
        if (!*ptr) break;
        numbers.push_back(ParseNumber(ptr));
    }
}

// Hàm đọc đơn vị
inline float ParseUnit(const std::string& str, float defaultVal = 0.0f)
{
    if (str.empty()) return defaultVal;

    const char* ptr = str.c_str();
    float val = ParseNumber(ptr);

    while (*ptr && (*ptr == ' ' || *ptr == '\t')) ptr++;
    std::string suffix = ptr;

    if (suffix == "px") return val;
    if (suffix == "pt") return val * 1.0f;
    if (suffix == "pc") return val * 15.0f;
    if (suffix == "mm") return val * 3.7795f;
    if (suffix == "cm") return val * 37.795f;
    if (suffix == "in") return val * 96.0f;
    if (suffix == "%") return val / 100.0f;

    return val;
}

// --- XỬ LÝ MÀU SẮC ---

inline int ClampColorValue(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return val;
}

inline bool GetNamedColor(std::string name, Gdiplus::Color& outColor)
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    static const std::map<std::string, DWORD> colorMap = {
        {"none", 0}, {"transparent", 0}, {"black", 0x000000}, {"white", 0xFFFFFF},
        {"red", 0xFF0000}, {"lime", 0x00FF00}, {"blue", 0x0000FF}, {"yellow", 0xFFFF00},
        {"cyan", 0x00FFFF}, {"magenta", 0xFF00FF}, {"gray", 0x808080}, {"grey", 0x808080},
        {"orange", 0xFFA500}, {"gold", 0xFFD700}, {"purple", 0x800080}, {"teal", 0x008080},
        {"navy", 0x000080}, {"maroon", 0x800000}, {"olive", 0x808000}, {"green", 0x008000},
        {"silver", 0xC0C0C0}, {"aqua", 0x00FFFF}, {"fuchsia", 0xFF00FF},
        {"midnightblue", 0x191970}, {"darkmagenta", 0x8B008B}, {"deepskyblue", 0x00BFFF},
        {"darkslategray", 0x2F4F4F}, {"blueviolet", 0x8A2BE2}, {"darkorange", 0xFF8C00},
        {"aliceblue", 0xF0F8FF}, {"antiquewhite", 0xFAEBD7}, {"aquamarine", 0x7FFFD4},
        {"azure", 0xF0FFFF}, {"beige", 0xF5F5DC}, {"bisque", 0xFFE4C4}, {"blanchedalmond", 0xFFEBCD},
        {"brown", 0xA52A2A}, {"burlywood", 0xDEB887}, {"cadetblue", 0x5F9EA0},
        {"chartreuse", 0x7FFF00}, {"chocolate", 0xD2691E}, {"coral", 0xFF7F50}, {"cornflowerblue", 0x6495ED},
        {"cornsilk", 0xFFF8DC}, {"crimson", 0xDC143C}, {"darkblue", 0x00008B}, {"darkcyan", 0x008B8B},
        {"darkgoldenrod", 0xB8860B}, {"darkgray", 0xA9A9A9}, {"darkgreen", 0x006400},
        {"darkkhaki", 0xBDB76B}, {"darkolivegreen", 0x556B2F},
        {"darkorchid", 0x9932CC}, {"darkred", 0x8B0000}, {"darksalmon", 0xE9967A}, {"darkseagreen", 0x8FBC8F},
        {"darkslateblue", 0x483D8B}, {"darkslategrey", 0x2F4F4F}, {"darkturquoise", 0x00CED1},
        {"darkviolet", 0x9400D3}, {"deeppink", 0xFF1493}, {"dimgray", 0x696969},
        {"dodgerblue", 0x1E90FF}, {"firebrick", 0xB22222}, {"floralwhite", 0xFFFAF0},
        {"forestgreen", 0x228B22}, {"gainsboro", 0xDCDCDC}, {"ghostwhite", 0xF8F8FF},
        {"goldenrod", 0xDAA520}, {"greenyellow", 0xADFF2F},
        {"honeydew", 0xF0FFF0}, {"hotpink", 0xFF69B4}, {"indianred", 0xCD5C5C}, {"indigo", 0x4B0082},
        {"ivory", 0xFFFFF0}, {"khaki", 0xF0E68C}, {"lavender", 0xE6E6FA}, {"lavenderblush", 0xFFF0F5},
        {"lawngreen", 0x7CFC00}, {"lemonchiffon", 0xFFFACD}, {"lightblue", 0xADD8E6}, {"lightcoral", 0xF08080},
        {"lightcyan", 0xE0FFFF}, {"lightgoldenrodyellow", 0xFAFAD2}, {"lightgray", 0xD3D3D3}, {"lightgreen", 0x90EE90},
        {"lightgrey", 0xD3D3D3}, {"lightpink", 0xFFB6C1}, {"lightsalmon", 0xFFA07A}, {"lightseagreen", 0x20B2AA},
        {"lightskyblue", 0x87CEFA}, {"lightslategray", 0x778899}, {"lightslategrey", 0x778899}, {"lightsteelblue", 0xB0C4DE},
        {"lightyellow", 0xFFFFE0}, {"limegreen", 0x32CD32}, {"linen", 0xFAF0E6}, {"mediumaquamarine", 0x66CDAA},
        {"mediumblue", 0x0000CD}, {"mediumorchid", 0xBA55D3}, {"mediumpurple", 0x9370DB}, {"mediumseagreen", 0x3CB371},
        {"mediumslateblue", 0x7B68EE}, {"mediumspringgreen", 0x00FA9A}, {"mediumturquoise", 0x48D1CC}, {"mediumvioletred", 0xC71585},
        {"mintcream", 0xF5FFFA}, {"mistyrose", 0xFFE4E1}, {"moccasin", 0xFFE4B5},
        {"navajowhite", 0xFFDEAD}, {"oldlace", 0xFDF5E6}, {"olivedrab", 0x6B8E23},
        {"orangered", 0xFF4500}, {"orchid", 0xDA70D6}, {"palegoldenrod", 0xEEE8AA}, {"palegreen", 0x98FB98},
        {"paleturquoise", 0xAFEEEE}, {"palevioletred", 0xDB7093}, {"papayawhip", 0xFFEFD5}, {"peachpuff", 0xFFDAB9},
        {"peru", 0xCD853F}, {"pink", 0xFFC0CB}, {"plum", 0xDDA0DD}, {"powderblue", 0xB0E0E6},
        {"rosybrown", 0xBC8F8F}, {"royalblue", 0x4169E1}, {"saddlebrown", 0x8B4513}, {"salmon", 0xFA8072},
        {"sandybrown", 0xF4A460}, {"seagreen", 0x2E8B57}, {"seashell", 0xFFF5EE}, {"sienna", 0xA0522D},
        {"skyblue", 0x87CEEB}, {"slateblue", 0x6A5ACD}, {"slategray", 0x708090}, {"slategrey", 0x708090},
        {"snow", 0xFFFAFA}, {"springgreen", 0x00FF7F}, {"steelblue", 0x4682B4}, {"tan", 0xD2B48C},
        {"thistle", 0xD8BFD8}, {"tomato", 0xFF6347}, {"turquoise", 0x40E0D0}, {"violet", 0xEE82EE},
        {"wheat", 0xF5DEB3}, {"whitesmoke", 0xF5F5F5}, {"yellowgreen", 0x9ACD32}
    };
    auto it = colorMap.find(name);
    if (it != colorMap.end()) {
        if (name == "none" || name == "transparent") { outColor = Gdiplus::Color(0, 0, 0, 0); return true; }
        DWORD h = it->second;
        outColor = Gdiplus::Color(255, (BYTE)((h >> 16) & 0xFF), (BYTE)((h >> 8) & 0xFF), (BYTE)(h & 0xFF));
        return true;
    }
    return false;
}

inline Gdiplus::Color ParseColor(std::string val) {
    val.erase(std::remove(val.begin(), val.end(), ' '), val.end());
    if (val == "none" || val == "transparent") return Gdiplus::Color(0, 0, 0, 0);
    if (!val.empty() && val[0] == '#') {
        std::string hex = val.substr(1);
        int r = 0, g = 0, b = 0;
        if (hex.length() == 3) {
            int hr = std::stoi(hex.substr(0, 1), 0, 16); r = hr * 17;
            int hg = std::stoi(hex.substr(1, 1), 0, 16); g = hg * 17;
            int hb = std::stoi(hex.substr(2, 1), 0, 16); b = hb * 17;
        }
        else if (hex.length() >= 6) {
            r = std::stoi(hex.substr(0, 2), 0, 16);
            g = std::stoi(hex.substr(2, 2), 0, 16);
            b = std::stoi(hex.substr(4, 2), 0, 16);
        }
        return Gdiplus::Color(255, r, g, b);
    }
    if (val.find("rgb(") == 0) {
        size_t s = 4, e = val.find(")");
        if (e != std::string::npos) {
            std::string content = val.substr(s, e - s);
            std::replace(content.begin(), content.end(), ',', ' ');
            const char* ptr = content.c_str();
            float fr = ParseNumber(ptr); float fg = ParseNumber(ptr); float fb = ParseNumber(ptr);
            int r = ClampColorValue((int)fr); int g = ClampColorValue((int)fg); int b = ClampColorValue((int)fb);
            return Gdiplus::Color(255, r, g, b);
        }
    }
    Gdiplus::Color named;
    if (GetNamedColor(val, named)) return named;
    return Gdiplus::Color(255, 0, 0, 0);
}

inline void ParseTransformString(const std::string& t, Gdiplus::Matrix& matrix) {
    matrix.Reset();
    size_t pos = 0;
    std::string s = t;
    Gdiplus::MatrixOrder order = Gdiplus::MatrixOrderPrepend;
    while (pos < s.length()) {
        size_t start = s.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", pos);
        if (start == std::string::npos) break;

        size_t openParen = s.find('(', start);
        if (openParen == std::string::npos) break;

        std::string command = s.substr(start, openParen - start);
        size_t closeParen = s.find(')', openParen);
        if (closeParen == std::string::npos) break;

        std::string args = s.substr(openParen + 1, closeParen - openParen - 1);
        std::vector<float> v;
        ParseNumberList(args.c_str(), v);

        if (command == "matrix" && v.size() >= 6) {
            Gdiplus::Matrix m(v[0], v[1], v[2], v[3], v[4], v[5]);
            matrix.Multiply(&m, order);
        }
        else if (command == "translate" && v.size() >= 1) {
            matrix.Translate(v[0], (v.size() > 1) ? v[1] : 0, order);
        }
        else if (command == "scale" && v.size() >= 1) {
            matrix.Scale(v[0], (v.size() > 1) ? v[1] : v[0], order);
        }
        else if (command == "rotate" && v.size() >= 1) {
            if (v.size() >= 3) {
                matrix.Translate(v[1], v[2], order);
                matrix.Rotate(v[0], order);
                matrix.Translate(-v[1], -v[2], order);
            }
            else {
                matrix.Rotate(v[0], order);
            }
        }
        else if (command == "skewX" && v.size() >= 1) {
            float rad = v[0] * 3.14159265f / 180.0f;
            matrix.Shear(tan(rad), 0, order);
        }
        else if (command == "skewY" && v.size() >= 1) {
            float rad = v[0] * 3.14159265f / 180.0f;
            matrix.Shear(0, tan(rad), order);
        }

        pos = closeParen + 1;
    }
}