#include "stdafx.h"
#include "SVGElement.h"
#include "SVGDocument.h"
#include <vector>
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
            fillColor = Color(0, 0, 0, 0);
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

        // ===== 5. fallback =====
        else {
            fillType = FillType::None;
            fillColor = Color(0, 0, 0, 0);
        }
    }


    // 3. Parse Stroke Color
    if (auto attr = node->first_attribute("stroke"))
    {
        string s = attr->value();
        // Dọn khoảng trắng
        while (!s.empty() && isspace(s.front())) s.erase(0, 1);

        if (s == "none") strokeColor = Color(0, 0, 0, 0);
        else if (!s.empty() && s[0] == '#') {
            string hex = s.substr(1);
            int r = 0, g = 0, b = 0;
            if (hex.length() >= 6) {
                r = Hex2Int(hex[0]) * 16 + Hex2Int(hex[1]);
                g = Hex2Int(hex[2]) * 16 + Hex2Int(hex[3]);
                b = Hex2Int(hex[4]) * 16 + Hex2Int(hex[5]);
            }
            strokeColor = Color((BYTE)(strokeOpacity * 255), r, g, b);
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
                transform.Translate(dx, dy, MatrixOrderAppend);
            }
            else if (command == "rotate" && vals.size() >= 1) {
                transform.Rotate(vals[0], MatrixOrderAppend);
            }
            else if (command == "scale" && vals.size() >= 1) {
                float sx = vals[0];
                float sy = (vals.size() > 1) ? vals[1] : sx;
                transform.Scale(sx, sy, MatrixOrderAppend);
            }
            pos = closeParen + 1;
        }
    }
}

void SVGElement::InheritFrom(const SVGElement& parent)
{
    this->fillColor = parent.fillColor;
    this->fillOpacity = parent.fillOpacity;
    this->strokeColor = parent.strokeColor;
    this->strokeWidth = parent.strokeWidth;
    this->strokeOpacity = parent.strokeOpacity;
    this->strokeMiterLimit = parent.strokeMiterLimit;
    this->fillType = parent.fillType;
    this->fillGradient = parent.fillGradient;
    this->document = parent.document;
    // Use Clone to copy the transform matrix since assignment is inaccessible
    Matrix* clonedMatrix = parent.transform.Clone();
    if (clonedMatrix != nullptr)
    {
        this->transform.Reset();
        this->transform.Multiply(clonedMatrix, MatrixOrderPrepend);
        delete clonedMatrix;
    }
}