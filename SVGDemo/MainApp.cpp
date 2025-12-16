#include "stdafx.h"
#include "MainApp.h"
#include <iostream>
#include <gdiplus.h>

using namespace Gdiplus;

// Destructor
MainApp::~MainApp()
{
    // Chỉ cần gọi parser dọn dẹp là đủ
    parser.Clear();
}

// Hàm dọn dẹp (Wrapper)
void MainApp::Clear()
{
    // Ủy quyền hoàn toàn cho parser quản lý bộ nhớ
    parser.Clear();
}

bool MainApp::LoadSVG(const std::string& filePath)
{
    // 1. Dọn dẹp dữ liệu cũ trước khi load mới
    Clear();

    // 2. Yêu cầu parser đọc file
    bool ok = parser.ParseFile(filePath);
    if (!ok)
    {
        // Dùng MessageBox để báo lỗi dễ thấy hơn std::cerr
        std::wstring wFilePath(filePath.begin(), filePath.end());
        std::cerr << "Khong the doc file SVG: " << filePath << std::endl;
        return false;
    }

    // 3. Thông báo thành công (Lấy số lượng trực tiếp từ parser)
    std::cout << "Da doc thanh cong " << parser.GetElements().size() << " phan tu SVG." << std::endl;

    return true;
}

void MainApp::Render(Graphics& g)
{
    // 1. Nếu có hình thì vẽ hình
    if (!parser.GetElements().empty())
    {
        renderer.Render(g, parser.GetElements());
    }
    // 2. Nếu chưa có hình -> Hướng dẫn kéo thả
    else
    {
        SolidBrush brush(Color(128, 128, 128)); // Màu xám
        FontFamily fontFamily(L"Arial");
        Font font(&fontFamily, 16, FontStyleBold, UnitPoint);
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        // Lấy kích thước vùng vẽ
        RectF bounds;
        g.GetVisibleClipBounds(&bounds);

        RectF rect(0, 0, bounds.Width, bounds.Height);

        g.DrawString(L"Vui long keo tha file SVG vao day!", -1, &font, rect, &format, &brush);
    }
}