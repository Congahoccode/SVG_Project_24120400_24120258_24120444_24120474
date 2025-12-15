#include "stdafx.h"
#include "SVGPath.h"
#include <iostream>

using namespace std;
using namespace Gdiplus;

// Giữ nguyên hàm GetNextNumber ...
float GetNextNumber(const string& s, size_t& pos)
{
    // ... (Code cũ của bạn giữ nguyên) ...
    // Copy lại hàm GetNextNumber từ tin nhắn trước
    while (pos < s.length() && (s[pos] == ' ' || s[pos] == ',' || s[pos] == '\t' || s[pos] == '\n')) pos++;
    if (pos >= s.length()) return 0.0f;
    size_t start = pos;
    if (s[pos] == '-' || s[pos] == '+') pos++;
    while (pos < s.length() && (isdigit(s[pos]) || s[pos] == '.')) pos++;
    if (pos < s.length() && (s[pos] == 'e' || s[pos] == 'E')) {
        pos++;
        if (pos < s.length() && (s[pos] == '-' || s[pos] == '+')) pos++;
        while (pos < s.length() && isdigit(s[pos])) pos++;
    }
    string numStr = s.substr(start, pos - start);
    try { return stof(numStr); }
    catch (...) { return 0.0f; }
}

void SVGPath::Parse(xml_node<>* node)
{
    SVGElement::Parse(node);

    // 1. Xử lý Fill Rule (Để vẽ đúng các hình có lỗ)
    xml_attribute<>* attrRule = node->first_attribute("fill-rule");
    if (attrRule) {
        string r = attrRule->value();
        if (r == "evenodd") path.SetFillMode(FillModeAlternate);
        else if (r == "nonzero") path.SetFillMode(FillModeWinding);
    }

    xml_attribute<>* attr = node->first_attribute("d");
    if (!attr) return;

    string d = attr->value();
    size_t pos = 0;
    PointF current(0, 0);
    PointF startFig(0, 0);
    PointF lastControl(0, 0); // Dùng cho lệnh S và T
    char command = ' ';

    while (pos < d.length())
    {
        if (d[pos] == ' ' || d[pos] == ',') { pos++; continue; }

        if (isalpha(d[pos])) {
            command = d[pos];
            pos++;
        }

        switch (command)
        {
        case 'M': // Move
        {
            float x = GetNextNumber(d, pos);
            float y = GetNextNumber(d, pos);
            path.StartFigure();
            current = PointF(x, y);
            startFig = current;
            lastControl = current;
            break;
        }
        case 'L': // Line
        {
            float x = GetNextNumber(d, pos);
            float y = GetNextNumber(d, pos);
            path.AddLine(current, PointF(x, y));
            current = PointF(x, y);
            lastControl = current;
            break;
        }
        case 'H': // Horizontal Line
        {
            float x = GetNextNumber(d, pos);
            path.AddLine(current, PointF(x, current.Y));
            current.X = x;
            lastControl = current;
            break;
        }
        case 'V': // Vertical Line
        {
            float y = GetNextNumber(d, pos);
            path.AddLine(current, PointF(current.X, y));
            current.Y = y;
            lastControl = current;
            break;
        }
        case 'C': // Cubic Bezier
        {
            float x1 = GetNextNumber(d, pos);
            float y1 = GetNextNumber(d, pos);
            float x2 = GetNextNumber(d, pos);
            float y2 = GetNextNumber(d, pos);
            float x = GetNextNumber(d, pos);
            float y = GetNextNumber(d, pos);

            path.AddBezier(current, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            lastControl = PointF(x2, y2); // Lưu điểm điều khiển cuối
            current = PointF(x, y);
            break;
        }
        case 'S': // Smooth Cubic Bezier
        {
            // Điểm điều khiển 1 là phản chiếu của lastControl qua current
            float x1 = 2 * current.X - lastControl.X;
            float y1 = 2 * current.Y - lastControl.Y;
            float x2 = GetNextNumber(d, pos);
            float y2 = GetNextNumber(d, pos);
            float x = GetNextNumber(d, pos);
            float y = GetNextNumber(d, pos);

            path.AddBezier(current, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            lastControl = PointF(x2, y2);
            current = PointF(x, y);
            break;
        }
        case 'Q': // Quadratic Bezier (Mới thêm)
        {
            float x1 = GetNextNumber(d, pos);
            float y1 = GetNextNumber(d, pos);
            float x = GetNextNumber(d, pos);
            float y = GetNextNumber(d, pos);

            // GDI+ không có AddQuadratic, ta chuyển đổi sang Cubic
            // Công thức xấp xỉ Quadratic bằng Cubic
            float c1x = current.X + (2.0f / 3.0f) * (x1 - current.X);
            float c1y = current.Y + (2.0f / 3.0f) * (y1 - current.Y);
            float c2x = x + (2.0f / 3.0f) * (x1 - x);
            float c2y = y + (2.0f / 3.0f) * (y1 - y);

            path.AddBezier(current, PointF(c1x, c1y), PointF(c2x, c2y), PointF(x, y));

            lastControl = PointF(x1, y1); // Lưu điểm điều khiển Q
            current = PointF(x, y);
            break;
        }
        case 'm': // MoveTo Relative
        {
            float dx = GetNextNumber(d, pos);
            float dy = GetNextNumber(d, pos);

            // Nếu là lệnh đầu tiên của path, nó hoạt động như M tuyệt đối
            if (pos == 0) {
                current = PointF(dx, dy);
                startFig = current;
            }
            else {
                // Cộng dồn vào vị trí cũ
                path.StartFigure();
                current = PointF(current.X + dx, current.Y + dy);
                startFig = current; // Cập nhật điểm bắt đầu của hình mới
            }
            lastControl = current;
            break;
        }

        case 'l': // LineTo Relative
        {
            float dx = GetNextNumber(d, pos);
            float dy = GetNextNumber(d, pos);

            PointF nextP(current.X + dx, current.Y + dy); // Tính điểm đến
            path.AddLine(current, nextP);

            current = nextP;
            lastControl = current;
            break;
        }

        case 'h': // Horizontal Relative
        {
            float dx = GetNextNumber(d, pos);
            PointF nextP(current.X + dx, current.Y);
            path.AddLine(current, nextP);
            current = nextP;
            lastControl = current;
            break;
        }

        case 'v': // Vertical Relative
        {
            float dy = GetNextNumber(d, pos);
            PointF nextP(current.X, current.Y + dy);
            path.AddLine(current, nextP);
            current = nextP;
            lastControl = current;
            break;
        }

        case 'c': // Cubic Bezier Relative
        {
            float dx1 = GetNextNumber(d, pos);
            float dy1 = GetNextNumber(d, pos);
            float dx2 = GetNextNumber(d, pos);
            float dy2 = GetNextNumber(d, pos);
            float dx = GetNextNumber(d, pos);
            float dy = GetNextNumber(d, pos);

            // Tất cả điểm điều khiển đều cộng thêm current.X, current.Y
            PointF p1(current.X + dx1, current.Y + dy1);
            PointF p2(current.X + dx2, current.Y + dy2);
            PointF p3(current.X + dx, current.Y + dy);

            path.AddBezier(current, p1, p2, p3);

            lastControl = p2; // Lưu điểm điều khiển cuối (tuyệt đối)
            current = p3;
            break;
        }

        case 's': // Smooth Cubic Relative
        {
            // Điểm phản chiếu (Logic giống S hoa)
            float ctrl1X = 2 * current.X - lastControl.X;
            float ctrl1Y = 2 * current.Y - lastControl.Y;

            float dx2 = GetNextNumber(d, pos);
            float dy2 = GetNextNumber(d, pos);
            float dx = GetNextNumber(d, pos);
            float dy = GetNextNumber(d, pos);

            PointF p2(current.X + dx2, current.Y + dy2);
            PointF p3(current.X + dx, current.Y + dy);

            path.AddBezier(current, PointF(ctrl1X, ctrl1Y), p2, p3);

            lastControl = p2;
            current = p3;
            break;
        }

        case 'q': // Quadratic Relative
        {
            float dx1 = GetNextNumber(d, pos);
            float dy1 = GetNextNumber(d, pos);
            float dx = GetNextNumber(d, pos);
            float dy = GetNextNumber(d, pos);

            PointF p1(current.X + dx1, current.Y + dy1); // Điểm điều khiển Q
            PointF pEnd(current.X + dx, current.Y + dy); // Điểm cuối

            // Chuyển đổi sang Cubic (Giống hệt Q hoa, chỉ khác input là p1, pEnd đã tính ở trên)
            float c1x = current.X + (2.0f / 3.0f) * (p1.X - current.X);
            float c1y = current.Y + (2.0f / 3.0f) * (p1.Y - current.Y);
            float c2x = pEnd.X + (2.0f / 3.0f) * (p1.X - pEnd.X);
            float c2y = pEnd.Y + (2.0f / 3.0f) * (p1.Y - pEnd.Y);

            path.AddBezier(current, PointF(c1x, c1y), PointF(c2x, c2y), pEnd);

            lastControl = p1;
            current = pEnd;
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
        // Lưu ý: Lệnh A (Arc) rất phức tạp về toán học.
        // Tạm thời ta bỏ qua các tham số của nó để không bị lệch các lệnh sau.
        case 'A':
        {
            GetNextNumber(d, pos); // rx
            GetNextNumber(d, pos); // ry
            GetNextNumber(d, pos); // rotation
            GetNextNumber(d, pos); // large-arc-flag
            GetNextNumber(d, pos); // sweep-flag
            float x = GetNextNumber(d, pos);
            float y = GetNextNumber(d, pos);

            // Tạm thời vẽ đường thẳng thay vì cung tròn để không gãy hình
            path.AddLine(current, PointF(x, y));
            current = PointF(x, y);
            lastControl = current;
            break;
        }
        default:
            pos++;
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