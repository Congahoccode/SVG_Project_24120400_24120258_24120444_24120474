#include "stdafx.h"
#define _USE_MATH_DEFINES 
#include "SVGPath.h"
#include "SVGHelper.h" 
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace Gdiplus;
using namespace std;

// Wrapper đơn giản gọi về Helper
static float GetNextNumber(const char*& ptr)
{
    return ParseNumber(ptr);
}

static bool GetNextFlag(const char*& ptr)
{
    while (*ptr && (*ptr == ' ' || *ptr == ',' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) ptr++;
    if (*ptr == '0') { ptr++; return false; }
    if (*ptr == '1') { ptr++; return true; }
    return false;
}

// ... (Giữ nguyên hàm AddArcToBezier để tiết kiệm chỗ, không thay đổi logic vẽ cong) ...
static void AddArcToBezier(GraphicsPath& path, float x0, float y0, float rx, float ry,
    float angle, bool largeArc, bool sweep, float x, float y)
{
    if (rx == 0 || ry == 0) { path.AddLine(PointF(x0, y0), PointF(x, y)); return; }
    if (abs(x0 - x) < 0.001f && abs(y0 - y) < 0.001f) return;
    rx = abs(rx); ry = abs(ry);
    float dx2 = (x0 - x) / 2.0f; float dy2 = (y0 - y) / 2.0f;
    float rad = angle * (float)M_PI / 180.0f;
    float cosA = cos(rad); float sinA = sin(rad);
    float x1 = cosA * dx2 + sinA * dy2; float y1 = -sinA * dx2 + cosA * dy2;
    float Prx = rx * rx; float Pry = ry * ry; float Px1 = x1 * x1; float Py1 = y1 * y1;
    float d = Px1 / Prx + Py1 / Pry;
    if (d > 1) { float s = sqrt(d); rx *= s; ry *= s; Prx = rx * rx; Pry = ry * ry; }
    float sign = (largeArc == sweep) ? -1.0f : 1.0f;
    float num = Prx * Pry - Prx * Py1 - Pry * Px1;
    float den = Prx * Py1 + Pry * Px1;
    float coef = sign * sqrt(max(0.0f, num / den));
    float cx1 = coef * ((rx * y1) / ry); float cy1 = coef * -((ry * x1) / rx);
    float sx2 = (x0 + x) / 2.0f; float sy2 = (y0 + y) / 2.0f;
    float cx = sx2 + (cosA * cx1 - sinA * cy1); float cy = sy2 + (sinA * cx1 + cosA * cy1);
    float ux = (x1 - cx1) / rx; float uy = (y1 - cy1) / ry;
    float vx = (-x1 - cx1) / rx; float vy = (-y1 - cy1) / ry;
    auto angleBetween = [](float ux, float uy, float vx, float vy) {
        float sign = (ux * vy - uy * vx < 0) ? -1.0f : 1.0f;
        float dot = ux * vx + uy * vy; return sign * acos(max(-1.0f, min(1.0f, dot)));
        };
    float startAngle = angleBetween(1, 0, ux, uy); float dAngle = angleBetween(ux, uy, vx, vy);
    if (sweep && dAngle < 0) dAngle += 2 * (float)M_PI; else if (!sweep && dAngle > 0) dAngle -= 2 * (float)M_PI;
    int segments = (int)ceil(abs(dAngle) / (M_PI / 12.0)); // Chia nhỏ 15 độ cho mịn
    if (segments < 1) segments = 1;
    float delta = dAngle / segments;
    float t = 8.0f / 3.0f * sin(delta / 4.0f) * sin(delta / 4.0f) / sin(delta / 2.0f);
    float currentX = x0; float currentY = y0; float theta1 = startAngle;
    float startTrueX = cosA * rx * cos(startAngle) - sinA * ry * sin(startAngle) + cx;
    float startTrueY = sinA * rx * cos(startAngle) + cosA * ry * sin(startAngle) + cy;

    if (abs(currentX - startTrueX) > 0.1f || abs(currentY - startTrueY) > 0.1f) {
        path.AddLine(currentX, currentY, startTrueX, startTrueY);
    }
    currentX = startTrueX;
    currentY = startTrueY;
    for (int i = 0; i < segments; ++i) {
        float theta2 = theta1 + delta;
        float cosTheta1 = cos(theta1); float sinTheta1 = sin(theta1);
        float cosTheta2 = cos(theta2); float sinTheta2 = sin(theta2);
        float epx = cosA * rx * cosTheta2 - sinA * ry * sinTheta2 + cx;
        float epy = sinA * rx * cosTheta2 + cosA * ry * sinTheta2 + cy;
        float dx1 = t * (-cosA * rx * sinTheta1 - sinA * ry * cosTheta1);
        float dy1 = t * (-sinA * rx * sinTheta1 + cosA * ry * cosTheta1);
        float dxe = t * (cosA * rx * sinTheta2 + sinA * ry * cosTheta2);
        float dye = t * (sinA * rx * sinTheta2 - cosA * ry * cosTheta2);
        path.AddBezier(currentX, currentY, currentX + dx1, currentY + dy1, epx - dxe, epy - dye, epx, epy);
        theta1 = theta2; currentX = epx; currentY = epy;
    }
}

void SVGPath::Parse(rapidxml::xml_node<>* node)
{
    // Gọi hàm Parse của lớp cha
    SVGElement::Parse(node);

    // FIX: Set default FillMode là Winding (NonZero) để tâm các hình phức tạp được tô màu đúng
    path.SetFillMode(FillModeWinding);

    // Xử lý thuộc tính fill-rule
    path.SetFillMode(fillRule);

    // Lấy dữ liệu đường dẫn (path data)
    rapidxml::xml_attribute<>* attr = node->first_attribute("d");
    if (!attr) return;

    const char* ptr = attr->value();
    PointF current(0, 0);
    PointF startFig(0, 0);
    PointF lastControl(0, 0);

    char command = ' ';
    char lastCommand = ' ';

    while (*ptr)
    {
        while (*ptr && strchr(" \t\n\r,", *ptr)) ptr++;
        if (!*ptr) break;
        if (isalpha(*ptr)) command = *ptr++;
        auto ReflectControl = [&](float curX, float curY) -> PointF
            {
                bool prevIsCurve = (lastCommand == 'C' || lastCommand == 'c' ||
                    lastCommand == 'S' || lastCommand == 's' ||
                    lastCommand == 'Q' || lastCommand == 'q' ||
                    lastCommand == 'T' || lastCommand == 't');

                if (prevIsCurve)
                {
                    return PointF(2 * curX - lastControl.X, 2 * curY - lastControl.Y);
                }
                return PointF(curX, curY);
            };

        switch (command)
        {
        case 'M':
        {
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);
            path.StartFigure();

            current = startFig = lastControl = PointF(x, y);
            lastCommand = 'M';
            command = 'L';
            break;
        }
        case 'm':
        {
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);

            if (path.GetPointCount() == 0)
            {
                current = startFig = PointF(dx, dy);
            }
            else
            {
                path.StartFigure();
                current = startFig = PointF(current.X + dx, current.Y + dy);
            }

            lastControl = current;
            lastCommand = 'm';
            command = 'l';
            break;
        }

        // Line To (L, l)
        case 'L':
        {
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);

            path.AddLine(current, PointF(x, y));

            current = lastControl = PointF(x, y);
            lastCommand = 'L';
            break;
        }
        case 'l':
        {
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);
            PointF p(current.X + dx, current.Y + dy);

            path.AddLine(current, p);

            current = lastControl = p;
            lastCommand = 'l';
            break;
        }

        // Horizontal Line (H, h)
        case 'H':
        {
            float x = GetNextNumber(ptr);

            path.AddLine(current, PointF(x, current.Y));

            current.X = x;
            lastControl = current;
            lastCommand = 'H';
            break;
        }
        case 'h':
        {
            float dx = GetNextNumber(ptr);
            PointF p(current.X + dx, current.Y);

            path.AddLine(current, p);

            current = lastControl = p;
            lastCommand = 'h';
            break;
        }

        // Vertical Line (V, v)
        case 'V':
        {
            float y = GetNextNumber(ptr);

            path.AddLine(current, PointF(current.X, y));

            current.Y = y;
            lastControl = current;
            lastCommand = 'V';
            break;
        }
        case 'v':
        {
            float dy = GetNextNumber(ptr);
            PointF p(current.X, current.Y + dy);

            path.AddLine(current, p);

            current = lastControl = p;
            lastCommand = 'v';
            break;
        }

        // Cubic Bezier (C, c)
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
            lastCommand = 'C';
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
            PointF p(current.X + dx, current.Y + dy);

            path.AddBezier(current, p1, p2, p);

            lastControl = p2;
            current = p;
            lastCommand = 'c';
            break;
        }

        // Smooth Cubic Bezier (S, s)
        case 'S':
        {
            PointF c1 = ReflectControl(current.X, current.Y);
            float x2 = GetNextNumber(ptr);
            float y2 = GetNextNumber(ptr);
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);

            path.AddBezier(current, c1, PointF(x2, y2), PointF(x, y));

            lastControl = PointF(x2, y2);
            current = PointF(x, y);
            lastCommand = 'S';
            break;
        }
        case 's':
        {
            PointF c1 = ReflectControl(current.X, current.Y);
            float dx2 = GetNextNumber(ptr);
            float dy2 = GetNextNumber(ptr);
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);

            PointF p2(current.X + dx2, current.Y + dy2);
            PointF p(current.X + dx, current.Y + dy);

            path.AddBezier(current, c1, p2, p);

            lastControl = p2;
            current = p;
            lastCommand = 's';
            break;
        }

        // Quadratic Bezier (Q, q)
        case 'Q':
        {
            float x1 = GetNextNumber(ptr);
            float y1 = GetNextNumber(ptr);
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);

            PointF c1(current.X + 2.0f / 3.0f * (x1 - current.X), current.Y + 2.0f / 3.0f * (y1 - current.Y));
            PointF c2(x + 2.0f / 3.0f * (x1 - x), y + 2.0f / 3.0f * (y1 - y));

            path.AddBezier(current, c1, c2, PointF(x, y));

            lastControl = PointF(x1, y1);
            current = PointF(x, y);
            lastCommand = 'Q';
            break;
        }
        case 'q':
        {
            float dx1 = GetNextNumber(ptr);
            float dy1 = GetNextNumber(ptr);
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);

            PointF p1(current.X + dx1, current.Y + dy1);
            PointF p(current.X + dx, current.Y + dy);

            PointF c1(current.X + 2.0f / 3.0f * (p1.X - current.X), current.Y + 2.0f / 3.0f * (p1.Y - current.Y));
            PointF c2(p.X + 2.0f / 3.0f * (p1.X - p.X), p.Y + 2.0f / 3.0f * (p1.Y - p.Y));

            path.AddBezier(current, c1, c2, p);

            lastControl = p1;
            current = p;
            lastCommand = 'q';
            break;
        }

        // Smooth Quadratic Bezier (T, t)
        case 'T':
        {
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);

            PointF p1 = ReflectControl(current.X, current.Y);

            PointF c1(current.X + 2.0f / 3.0f * (p1.X - current.X), current.Y + 2.0f / 3.0f * (p1.Y - current.Y));
            PointF c2(x + 2.0f / 3.0f * (p1.X - x), y + 2.0f / 3.0f * (p1.Y - y));

            path.AddBezier(current, c1, c2, PointF(x, y));

            lastControl = p1;
            current = PointF(x, y);
            lastCommand = 'T';
            break;
        }
        case 't':
        {
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);
            PointF p(current.X + dx, current.Y + dy);

            PointF p1 = ReflectControl(current.X, current.Y);

            PointF c1(current.X + 2.0f / 3.0f * (p1.X - current.X), current.Y + 2.0f / 3.0f * (p1.Y - current.Y));
            PointF c2(p.X + 2.0f / 3.0f * (p1.X - p.X), p.Y + 2.0f / 3.0f * (p1.Y - p.Y));

            path.AddBezier(current, c1, c2, p);

            lastControl = p1;
            current = p;
            lastCommand = 't';
            break;
        }

        // Arc (A, a)
        case 'A':
        {
            float rx = GetNextNumber(ptr);
            float ry = GetNextNumber(ptr);
            float angle = GetNextNumber(ptr);
            bool large = GetNextFlag(ptr);
            bool sweep = GetNextFlag(ptr);
            float x = GetNextNumber(ptr);
            float y = GetNextNumber(ptr);

            AddArcToBezier(path, current.X, current.Y, rx, ry, angle, large, sweep, x, y);

            current = lastControl = PointF(x, y);
            lastCommand = 'A';
            break;
        }
        case 'a':
        {
            float rx = GetNextNumber(ptr);
            float ry = GetNextNumber(ptr);
            float angle = GetNextNumber(ptr);
            bool large = GetNextFlag(ptr);
            bool sweep = GetNextFlag(ptr);
            float dx = GetNextNumber(ptr);
            float dy = GetNextNumber(ptr);
            float x = current.X + dx;
            float y = current.Y + dy;

            AddArcToBezier(path, current.X, current.Y, rx, ry, angle, large, sweep, x, y);

            current = lastControl = PointF(x, y);
            lastCommand = 'a';
            break;
        }

        // Close Path (Z, z)
        case 'Z':
        case 'z':
        {
            path.CloseFigure();
            current = lastControl = startFig;
            lastCommand = 'Z';
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
    g.SetSmoothingMode(SmoothingModeHighQuality);
    GraphicsState state = g.Save();
    g.MultiplyTransform(&transform);
    RectF bounds; path.GetBounds(&bounds);
    if (auto* brush = CreateFillBrush(bounds)) { g.FillPath(brush, &path); delete brush; }
    if (auto* pen = CreateStrokePen()) { g.DrawPath(pen, &path); delete pen; }
    g.Restore(state);
}

RectF SVGPath::GetBoundingBox()
{
    RectF bounds; path.GetBounds(&bounds); return bounds;
}