#pragma once
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <gdiplus.h>
#include <cmath>
#include <queue>
#include <string>

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Gdiplus.lib")

static HWND _hwnd;
static HDC  _hdc;
static HPEN _pen;
static COLORREF _penColor = RGB(0,0,0);
static COLORREF _bgColor = RGB(255,255,255);
static int _lineWidth = 2;
static COLORREF _gradColor = RGB(0,0,0);
static bool _useGrad = false;

// 图片选择框状态（当前控件只支持一个图片框）
static RECT _imageBoxRect = { 0 };
static Gdiplus::Bitmap* _uploadedImage = nullptr;
static ULONG_PTR _gdiplusToken = 0;
static bool _gdiplusReady = false;

// ----------------- 输入框状态 -----------------
static wchar_t _inputText[256] = L"";
static RECT _inputRect = { 0 };
static bool _inputFocused = false;
static int _inputMaxLen = 0;

static void _applyPen() {
    DeleteObject(_pen);
    _pen = CreatePen(PS_SOLID, _lineWidth, _penColor);
    SelectObject(_hdc, _pen);
}
static LRESULT CALLBACK _proc(HWND h, UINT m, WPARAM wp, LPARAM lp) {
    if (m == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    if (m == WM_LBUTTONDOWN) {
        POINT p;
        p.x = LOWORD(lp);
        p.y = HIWORD(lp);

        if (PtInRect(&_imageBoxRect, p)) {
            wchar_t path[32768] = L"";
            OPENFILENAMEW dialog = { sizeof(dialog) };
            dialog.hwndOwner = h;
            dialog.lpstrFile = path;
            dialog.nMaxFile = sizeof(path) / sizeof(path[0]);
            dialog.lpstrFilter = L"图片文件\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tif;*.tiff\0所有文件\0*.*\0";
            dialog.nFilterIndex = 1;
            dialog.lpstrTitle = L"选择图片";
            dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

            if (_gdiplusReady && GetOpenFileNameW(&dialog)) {
                Gdiplus::Bitmap* image = Gdiplus::Bitmap::FromFile(path, FALSE);
                if (image && image->GetLastStatus() == Gdiplus::Ok) {
                    delete _uploadedImage;
                    _uploadedImage = image;
                } else {
                    delete image;
                    MessageBoxW(h, L"无法读取此图片格式。", L"图片上传失败", MB_OK | MB_ICONERROR);
                }
                InvalidateRect(h, NULL, FALSE);
            }
            return 0;
        }

        if (PtInRect(&_inputRect, p)) {
            _inputFocused = true;
        } else {
            _inputFocused = false;
        }
        return 0;
    }

    if (m == WM_CHAR && _inputFocused && _inputMaxLen > 0) {
        wchar_t ch = (wchar_t)wp;
        size_t len = wcslen(_inputText);

        if (ch == VK_BACK) {
            if (len > 0) _inputText[len - 1] = L'\0';
            return 0;
        }

        if (ch == VK_RETURN || ch == VK_ESCAPE) {
            _inputFocused = false;
            return 0;
        }

        if (ch >= 32 && len + 1 < (size_t)_inputMaxLen && len + 1 < sizeof(_inputText) / sizeof(_inputText[0])) {
            _inputText[len] = ch;
            _inputText[len + 1] = L'\0';
        }
        return 0;
    }

    return DefWindowProc(h, m, wp, lp);
}
static void gradcolor(int R, int G, int B) {
    _gradColor = RGB(R,G,B);
    _useGrad = true;
}
static void color(int R, int G, int B, bool setBackground = false) {
    _penColor = RGB(R,G,B);
    if (setBackground) {
        _bgColor = _penColor;
    }
    _applyPen();
}
static void width(int w) {
    _lineWidth = w;
}
static void line(int x1,int y1,int x2,int y2) {
    _applyPen();
    MoveToEx(_hdc,x1,y1,0);
    LineTo(_hdc,x2,y2);
}

static void bend(int x1,int y1,int x2,int y2,int x3,int y3) {
    _applyPen();
    const int STEPS = 50;
    float cx = 2.0f * x2 - 0.5f * x1 - 0.5f * x3;
    float cy = 2.0f * y2 - 0.5f * y1 - 0.5f * y3;
    for (int i = 1; i <= STEPS; i++) {
        float t = (float)i / STEPS;
        float u = 1 - t;
        float bx = u*u*x1 + 2*u*t*cx + t*t*x3;
        float by = u*u*y1 + 2*u*t*cy + t*t*y3;
        if (i == 1) MoveToEx(_hdc, (int)bx, (int)by, 0);
        else LineTo(_hdc, (int)bx, (int)by);
    }
}

// 文本：优先使用 Microsoft YaHei / SimHei 等中文字体，并确保 size 参数生效
static void text(int x, int y, const wchar_t* s, int size = 16) {
    int dpi = GetDeviceCaps(_hdc, LOGPIXELSY);
    LOGFONTW lf = {0};
    lf.lfHeight = -MulDiv(size, dpi, 72);
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = CLEARTYPE_QUALITY;

    const wchar_t* candidates[] = {
        L"Microsoft YaHei",
        L"Microsoft YaHei UI",
        L"SimHei",
        L"Arial",
        L"Microsoft Sans Serif"
    };

    HFONT hFont = nullptr;
    for (const wchar_t* name : candidates) {
        wcscpy_s(lf.lfFaceName, name);
        HFONT hTry = CreateFontIndirectW(&lf);
        if (!hTry) continue;

        HFONT hOld = (HFONT)SelectObject(_hdc, hTry);
        wchar_t face[LF_FACESIZE] = {0};
        GetTextFaceW(_hdc, LF_FACESIZE, face);
        SelectObject(_hdc, hOld);

        bool matched = (_wcsicmp(face, name) == 0);
        if (matched) {
            hFont = hTry;
            break;
        }

        DeleteObject(hTry);
    }

    if (!hFont) {
        hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }

    HFONT hOld = (HFONT)SelectObject(_hdc, hFont);
    SetTextColor(_hdc, _penColor);
    SetBkMode(_hdc, TRANSPARENT);
    TextOutW(_hdc, x, y, s, (int)wcslen(s));
    SelectObject(_hdc, hOld);

    if (hFont != (HFONT)GetStockObject(DEFAULT_GUI_FONT)) {
        DeleteObject(hFont);
    }
}
static void window(int w, int h, const wchar_t* title, void (*draw)()) {
    Gdiplus::GdiplusStartupInput gdiplusInput;
    _gdiplusReady = Gdiplus::GdiplusStartup(&_gdiplusToken, &gdiplusInput, NULL) == Gdiplus::Ok;

    WNDCLASS wc = {0};
    wc.lpfnWndProc = _proc;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = L"GUI";
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    RegisterClass(&wc);

    _hwnd = CreateWindow(L"GUI", title, WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, w, h, 0,0,0,0);
    if (!_hwnd) {
        if (_gdiplusReady) Gdiplus::GdiplusShutdown(_gdiplusToken);
        _gdiplusReady = false;
        return;
    }
    ShowWindow(_hwnd, SW_SHOW);
    _hdc = GetDC(_hwnd);
    _pen = CreatePen(PS_SOLID, 2, _penColor);

    MSG msg;
    bool running = true;
    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (running) draw();
    }

    delete _uploadedImage;
    _uploadedImage = nullptr;
    ReleaseDC(_hwnd, _hdc);
    DeleteObject(_pen);
    if (_gdiplusReady) Gdiplus::GdiplusShutdown(_gdiplusToken);
    _gdiplusReady = false;
}

static void clear() {
    RECT rc; GetClientRect(_hwnd, &rc);
    HBRUSH b = CreateSolidBrush(_bgColor);
    FillRect(_hdc, &rc, b);
    DeleteObject(b);
}

static void rect(int x1,int y1,int x2,int y2) {
    _applyPen();
    HBRUSH old = (HBRUSH)SelectObject(_hdc, GetStockObject(NULL_BRUSH));
    Rectangle(_hdc, x1, y1, x2, y2);
    SelectObject(_hdc, old);
}

static void fullrect(int x1,int y1,int x2,int y2) {
    if (!_useGrad) {
        _applyPen();
        HBRUSH b = CreateSolidBrush(_penColor);
        HBRUSH old = (HBRUSH)SelectObject(_hdc, b);
        Rectangle(_hdc, x1, y1, x2, y2);
        SelectObject(_hdc, old);
        DeleteObject(b);
        return;
    }
    // 渐变版：y 方向从 color 渐变到 gc
    int height = y2 - y1;
    if (height <= 0) return;
    for (int y = 0; y < height; y++) {
        float t = (float)y / height;
        int R = (int)(GetRValue(_penColor) + t * (GetRValue(_gradColor) - GetRValue(_penColor)));
        int G = (int)(GetGValue(_penColor) + t * (GetGValue(_gradColor) - GetGValue(_penColor)));
        int B = (int)(GetBValue(_penColor) + t * (GetBValue(_gradColor) - GetBValue(_penColor)));
        HBRUSH b = CreateSolidBrush(RGB(R,G,B));
        RECT rc = { x1, y1 + y, x2, y1 + y + 1 };
        FillRect(_hdc, &rc, b);
        DeleteObject(b);
    }
}
static int mousex() {
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(_hwnd, &p);
    return p.x;
}
static int mousey() {
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(_hwnd, &p);
    return p.y;
}
static void circle(int x, int y, int r) {
    _applyPen();
    HBRUSH old = (HBRUSH)SelectObject(_hdc, GetStockObject(NULL_BRUSH));
    Ellipse(_hdc, x - r, y - r, x + r, y + r);
    SelectObject(_hdc, old);
}
static void fullcircle(int x, int y, int r) {
    if (!_useGrad) {
        _applyPen();
        HBRUSH b = CreateSolidBrush(_penColor);
        HBRUSH old = (HBRUSH)SelectObject(_hdc, b);
        Ellipse(_hdc, x - r, y - r, x + r, y + r);
        SelectObject(_hdc, old);
        DeleteObject(b);
        return;
    }
    // 逐行扫描，从上到下，每行渐变色填充
    for (int dy = -r; dy <= r; dy++) {
        int half = (int)sqrt((double)(r*r - dy*dy));
        float t = (float)(dy + r) / (2*r);
        int R = (int)(GetRValue(_penColor) + t * (GetRValue(_gradColor) - GetRValue(_penColor)));
        int G = (int)(GetGValue(_penColor) + t * (GetGValue(_gradColor) - GetGValue(_penColor)));
        int B = (int)(GetBValue(_penColor) + t * (GetBValue(_gradColor) - GetBValue(_penColor)));
        HBRUSH b = CreateSolidBrush(RGB(R,G,B));
        RECT rc = { x - half, y + dy, x + half, y + dy + 1 };
        FillRect(_hdc, &rc, b);
        DeleteObject(b);
    }
}
static void nograd() {
    _useGrad = false;
}
static bool mousedown() {
    return GetAsyncKeyState(VK_LBUTTON) & 0x8000;
}
// 油漆桶填充：从(x,y)开始，把相连同色区域填成当前 color 颜色
static void fill(int x, int y) {
    COLORREF target = GetPixel(_hdc, x, y);
    COLORREF newColor = _penColor;

    if (target == newColor) return;

    std::queue<std::pair<int,int>> q;
    q.push({x, y});
    while (!q.empty()) {
        auto [cx, cy] = q.front();
        q.pop();

        RECT rc; GetClientRect(_hwnd, &rc);
        if (cx < 0 || cy < 0 || cx >= rc.right || cy >= rc.bottom) continue;
        if (GetPixel(_hdc, cx, cy) != target) continue;

        SetPixel(_hdc, cx, cy, newColor);

        q.push({cx+1, cy});
        q.push({cx-1, cy});
        q.push({cx, cy+1});
        q.push({cx, cy-1});
    }
}

// 图片上传框：点击后选择本地图片，支持 PNG/JPEG/BMP/GIF/TIFF，并按比例缩放显示。
static void imagebox(int x, int y, int w, int h) {
    if (!_hdc || w <= 0 || h <= 0) return;
    _imageBoxRect = { x, y, x + w, y + h };

    HBRUSH background = CreateSolidBrush(RGB(245, 245, 245));
    FillRect(_hdc, &_imageBoxRect, background);
    DeleteObject(background);

    if (_uploadedImage && _gdiplusReady) {
        UINT imageWidth = _uploadedImage->GetWidth();
        UINT imageHeight = _uploadedImage->GetHeight();
        if (imageWidth > 0 && imageHeight > 0) {
            double scale = (std::min)(static_cast<double>(w) / imageWidth,
                                      static_cast<double>(h) / imageHeight);
            int drawWidth = static_cast<int>(imageWidth * scale);
            int drawHeight = static_cast<int>(imageHeight * scale);
            int drawX = x + (w - drawWidth) / 2;
            int drawY = y + (h - drawHeight) / 2;
            Gdiplus::Graphics graphics(_hdc);
            graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            graphics.DrawImage(_uploadedImage, drawX, drawY, drawWidth, drawHeight);
        }
    } else {
        SetBkMode(_hdc, TRANSPARENT);
        SetTextColor(_hdc, RGB(90, 90, 90));
        DrawTextW(_hdc, L"点击选择图片", -1, &_imageBoxRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    HPEN border = CreatePen(PS_SOLID, 1, RGB(160, 160, 160));
    HPEN oldPen = (HPEN)SelectObject(_hdc, border);
    HBRUSH oldBrush = (HBRUSH)SelectObject(_hdc, GetStockObject(NULL_BRUSH));
    Rectangle(_hdc, x, y, x + w, y + h);
    SelectObject(_hdc, oldBrush);
    SelectObject(_hdc, oldPen);
    DeleteObject(border);
}

// ----------------- 输入框函数 -----------------
static void inputbox(int x, int y, int w, int h, wchar_t* buffer, int maxLen, int fontSize = 16) {
    if (buffer == nullptr || maxLen <= 0) return;

    if (_inputText[0] == L'\0' && buffer[0] != L'\0') {
        wcscpy_s(_inputText, 256, buffer);
    }

    _inputRect = { x, y, x + w, y + h };
    _inputMaxLen = maxLen;

    HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
    RECT bgRect = { x, y, x + w, y + h };
    FillRect(_hdc, &bgRect, bgBrush);
    DeleteObject(bgBrush);

    HPEN borderPen = CreatePen(PS_SOLID, 2, _inputFocused ? RGB(60, 120, 255) : RGB(180, 180, 180));
    HPEN oldPen = (HPEN)SelectObject(_hdc, borderPen);
    Rectangle(_hdc, x, y, x + w, y + h);
    SelectObject(_hdc, oldPen);
    DeleteObject(borderPen);

    int dpi = GetDeviceCaps(_hdc, LOGPIXELSY);
    LOGFONTW lf = {0};
    lf.lfHeight = -MulDiv(fontSize, dpi, 72);
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = CLEARTYPE_QUALITY;

    HFONT hFont = CreateFontIndirectW(&lf);
    HFONT oldFont = (HFONT)SelectObject(_hdc, hFont);
    SetBkMode(_hdc, TRANSPARENT);
    SetTextColor(_hdc, RGB(0, 0, 0));

    int padding = 8;
    TextOutW(_hdc, x + padding, y + (h - fontSize) / 2, _inputText, (int)wcslen(_inputText));

    if (_inputFocused) {
        int caretX = x + padding + (int)(wcslen(_inputText) * (fontSize * 0.6f));
        int caretY = y + 4;
        MoveToEx(_hdc, caretX, caretY, NULL);
        LineTo(_hdc, caretX, caretY + h - 8);
    }

    SelectObject(_hdc, oldFont);
    DeleteObject(hFont);

    wcscpy_s(buffer, maxLen, _inputText);
}
