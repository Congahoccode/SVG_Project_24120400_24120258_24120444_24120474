#include "stdafx.h"
#include "SVGPath.h"
#include <iostream>
#include <cstdlib> 

using namespace std;
using namespace Gdiplus;

// Hàm lấy phần tử vẽ tiếp theo
static float GetNextNumber(const char*& ptr)
{
    // Bỏ qua khoảng trắng và dấu phẩy
    while (*ptr && (*ptr == ' ' || *ptr == ',' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) {
        ptr++;
    }

    if (*ptr == '\0') return 0.0f;

    char* endPtr;
    double val = strtod(ptr, &endPtr);

    if (ptr == endPtr) {
        return 0.0f;
    }

    ptr = endPtr;
    return (float)val;
}

void SVGPath::Parse(xml_node<>* node)
{
    SVGElement::Parse(node);

    // 1. Fill Rule
    if (auto attrRule = node->first_attribute("fill-rule")) {
        string r = attrRule->value();
        if (r == "evenodd") path.SetFillMode(FillModeAlternate);
        else if (r == "nonzero") path.SetFillMode(FillModeWinding);
    }

    // 2. Parse Path Data
    xml_attribute<>* attr = node->first_attribute("d");
    if (!attr) return;

    // Lấy con trỏ C-style
    const char* ptr = attr->value();

    PointF current(0, 0);
    PointF startFig(0, 0);
    PointF lastControl(0, 0);
    char command = ' ';

    while (*ptr)
    {
        // Bỏ qua rác
        while (*ptr && (*ptr == ' ' || *ptr == ',' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) ptr++;
        if (*ptr == '\0') break;

        // Nếu là ký tự lệnh (M, L, C...)
        if (isalpha(*ptr)) {
            command = *ptr;
            ptr++;
        }
        // Nếu là số, giữ nguyên lệnh cũ (implicit repetition)

        switch (command)
        {
        case 'M':
        {
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);
            path.StartFigure();
            current = PointF(x, y);
            startFig = current;
            lastControl = current;

            command = 'L';
            break;
        }
        case 'L':
        {
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);
            path.AddLine(current, PointF(x, y));
            current = PointF(x, y);
            lastControl = current;
            break;
        }
        case 'H':
        {
            float x = GetNextNumber(ptr);
            path.AddLine(current, PointF(x, current.Y));
            current.X = x;
            lastControl = current;
            break;
        }
        case 'V':
        {
            float y = GetNextNumber(ptr);
            path.AddLine(current, PointF(current.X, y));
            current.Y = y;
            lastControl = current;
            break;
        }
        case 'C':
        {
            float x1 = GetNextNumber(ptr);
            float y1 = GetNextNumber(ptr);
            float x2 = GetNextNumber(ptr);
            float y2 = GetNextNumber(ptr);
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);
            path.AddBezier(current, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            lastControl = PointF(x2, y2);
            current = PointF(x, y);
            break;
        }
        case 'S':
        {
            float x1 = 2 * current.X - lastControl.X;
            float y1 = 2 * current.Y - lastControl.Y;
            float x2 = GetNextNumber(ptr);
            float y2 = GetNextNumber(ptr);
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);
            path.AddBezier(current, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            lastControl = PointF(x2, y2);
            current = PointF(x, y);
            break;
        }
        case 'Q':
        {
            float x1 = GetNextNumber(ptr);
            float y1 = GetNextNumber(ptr);
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);
            float c1x = current.X + (2.0f / 3.0f) * (x1 - current.X);
            float c1y = current.Y + (2.0f / 3.0f) * (y1 - current.Y);
            float c2x = x + (2.0f / 3.0f) * (x1 - x);
            float c2y = y + (2.0f / 3.0f) * (y1 - y);
            path.AddBezier(current, PointF(c1x, c1y), PointF(c2x, c2y), PointF(x, y));
            lastControl = PointF(x1, y1);
            current = PointF(x, y);
            break;
        }
        case 'Z':
        case 'z':
        {
            path.CloseFigure();
            current = startFig;
            lastControl = current;
            break;
        }

        // TƯƠNG ĐỐI (Relative)
        case 'm':
        {
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);
            if (path.GetPointCount() == 0) {
                current = PointF(dx, dy);
                startFig = current;
            }
            else {
                path.StartFigure();
                current = PointF(current.X + dx, current.Y + dy);
                startFig = current;
            }
            lastControl = current;

            command = 'l';
            break;
        }
        case 'l':
        {
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);
            PointF p(current.X + dx, current.Y + dy);
            path.AddLine(current, p);
            current = p;
            lastControl = current;
            break;
        }
        case 'h':
        {
            float dx = GetNextNumber(ptr);
            PointF p(current.X + dx, current.Y);
            path.AddLine(current, p);
            current = p;
            lastControl = current;
            break;
        }
        case 'v':
        {
            float dy = GetNextNumber(ptr);
            PointF p(current.X, current.Y + dy);
            path.AddLine(current, p);
            current = p;
            lastControl = current;
            break;
        }
        case 'c':
        {
            float dx1 = GetNextNumber(ptr);
            float dy1 = GetNextNumber(ptr);
            float dx2 = GetNextNumber(ptr);
            float dy2 = GetNextNumber(ptr);
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);
            PointF p1(current.X + dx1, current.Y + dy1);
            PointF p2(current.X + dx2, current.Y + dy2);
            PointF p3(current.X + dx, current.Y + dy);
            path.AddBezier(current, p1, p2, p3);
            lastControl = p2;
            current = p3;
            break;
        }
        case 's':
        {
            float ctrl1X = 2 * current.X - lastControl.X;
            float ctrl1Y = 2 * current.Y - lastControl.Y;
            float dx2 = GetNextNumber(ptr);
            float dy2 = GetNextNumber(ptr);
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);
            PointF p2(current.X + dx2, current.Y + dy2);
            PointF p3(current.X + dx, current.Y + dy);
            path.AddBezier(current, PointF(ctrl1X, ctrl1Y), p2, p3);
            lastControl = p2;
            current = p3;
            break;
        }
        case 'q': // Quadratic Relative
        {
            float dx1 = GetNextNumber(ptr);
            float dy1 = GetNextNumber(ptr);
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);
            PointF p1(current.X + dx1, current.Y + dy1);
            PointF pEnd(current.X + dx, current.Y + dy);
            float c1x = current.X + (2.0f / 3.0f) * (p1.X - current.X);
            float c1y = current.Y + (2.0f / 3.0f) * (p1.Y - current.Y);
            float c2x = pEnd.X + (2.0f / 3.0f) * (p1.X - pEnd.X);
            float c2y = pEnd.Y + (2.0f / 3.0f) * (p1.Y - pEnd.Y);
            path.AddBezier(current, PointF(c1x, c1y), PointF(c2x, c2y), pEnd);
            lastControl = p1;
            current = pEnd;
            break;
        }
        case 'A': // Arc (Bỏ qua)
        case 'a':
        {
            GetNextNumber(ptr); GetNextNumber(ptr); GetNextNumber(ptr);
            GetNextNumber(ptr); GetNextNumber(ptr);
            float x, y;
            if (command == 'A') { x = GetNextNumber(ptr); y = GetNextNumber(ptr); }
            else { x = current.X + GetNextNumber(ptr); y = current.Y + GetNextNumber(ptr); }
            path.AddLine(current, PointF(x, y));
            current = PointF(x, y);
            lastControl = current;
            break;
        }

        default:
            ptr++;
            break;
        }
    }
}

void SVGPath::Draw(Graphics& g)
{
    auto state = g.Save();
    g.MultiplyTransform(&transform);

    RectF bounds;
    path.GetBounds(&bounds);

    if (auto* brush = CreateFillBrush(bounds))
    {
        g.FillPath(brush, &path);
        delete brush;
    }

    if (auto* pen = CreateStrokePen())
    {
        g.DrawPath(pen, &path);
        delete pen;
    }

    g.Restore(state);
}

RectF SVGPath::GetBoundingBox()
{
    RectF bounds;
    // Lấy kích thước path
    path.GetBounds(&bounds, &transform, NULL);
    return bounds;
}