#pragma once
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <cmath>
#include <queue>

static HWND _hwnd;
static HDC  _hdc;
static HPEN _pen;
static COLORREF _penColor = RGB(0,0,0);
static COLORREF _bgColor = RGB(255,255,255);
static int _lineWidth = 2;
static COLORREF _gradColor = RGB(0,0,0);
static bool _useGrad = false;

static void _applyPen() {
    DeleteObject(_pen);
    _pen = CreatePen(PS_SOLID, _lineWidth, _penColor);
    SelectObject(_hdc, _pen);
}
static LRESULT CALLBACK _proc(HWND h, UINT m, WPARAM wp, LPARAM lp) {
    if (m == WM_DESTROY) PostQuitMessage(0);
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

// 文本：优先使用 "Microsoft YaHei"，若系统无该字体则回退到 DEFAULT_GUI_FONT
static void text(int x, int y, const wchar_t* s, int size = 16) {
    LOGFONTW lf = {0};
    lf.lfHeight = size;
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = DEFAULT_QUALITY;
    wcscpy_s(lf.lfFaceName, L"Microsoft YaHei");

    HFONT hTry = CreateFontIndirectW(&lf);
    // 先选入设备上下文，查询实际被选中的字族名，以判断系统是否支持该字体
    HFONT hOld = (HFONT)SelectObject(_hdc, hTry);
    wchar_t face[LF_FACESIZE] = {0};
    GetTextFaceW(_hdc, LF_FACESIZE, face);
    bool haveYaHei = (_wcsicmp(face, L"Microsoft YaHei") == 0 || _wcsicmp(face, L"Microsoft YaHei UI") == 0);
    // 恢复之前的字体
    SelectObject(_hdc, hOld);

    if (haveYaHei) {
        // 使用我们创建的字体
        HFONT hUse = (HFONT)SelectObject(_hdc, hTry);
        SetTextColor(_hdc, _penColor);
        SetBkMode(_hdc, TRANSPARENT);
        TextOutW(_hdc, x, y, s, (int)wcslen(s));
        SelectObject(_hdc, hUse);
        DeleteObject(hTry);
    } else {
        // 没有微软雅黑，释放尝试的字体，使用系统默认 GUI 字体
        DeleteObject(hTry);
        HFONT def = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        HFONT prev = (HFONT)SelectObject(_hdc, def);
        SetTextColor(_hdc, _penColor);
        SetBkMode(_hdc, TRANSPARENT);
        TextOutW(_hdc, x, y, s, (int)wcslen(s));
        SelectObject(_hdc, prev);
    }
}
static void window(int w, int h, const wchar_t* title, void (*draw)()) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = _proc;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = L"GUI";
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    RegisterClass(&wc);

    _hwnd = CreateWindow(L"GUI", title, WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, w, h, 0,0,0,0);
    ShowWindow(_hwnd, SW_SHOW);
    _hdc = GetDC(_hwnd);
    _pen = CreatePen(PS_SOLID, 2, _penColor);

    MSG msg;
    while(true) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        draw();
    }
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
