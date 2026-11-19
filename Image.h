#pragma once
#include "GUI.h"
#include <vector>
#include <utility>

// image() 使用的图片缓存：按完整路径缓存 GDI+ Bitmap。
static std::vector<std::pair<std::wstring, Gdiplus::Bitmap*>> _imageCache;

// 通过完整或相对路径直接加载并显示图片。
// 图片会按比例缩放并居中显示在指定矩形内。
// 支持 PNG、JPEG、BMP、GIF 和 TIFF。
static bool image(const wchar_t* path, int x, int y, int w, int h) {
    if (!path || !*path || w <= 0 || h <= 0 || !_gdiplusReady || !_hdc)
        return false;

    Gdiplus::Bitmap* bitmap = nullptr;
    for (const auto& item : _imageCache) {
        if (item.first == path) {
            bitmap = item.second;
            break;
        }
    }

    if (!bitmap) {
        bitmap = Gdiplus::Bitmap::FromFile(path, FALSE);
        if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
            delete bitmap;
            return false;
        }
        _imageCache.push_back({ std::wstring(path), bitmap });
    }

    UINT srcW = bitmap->GetWidth();
    UINT srcH = bitmap->GetHeight();
    if (srcW == 0 || srcH == 0)
        return false;

    double scale = (std::min)(static_cast<double>(w) / srcW,
                              static_cast<double>(h) / srcH);
    int drawW = static_cast<int>(srcW * scale);
    int drawH = static_cast<int>(srcH * scale);
    int drawX = x + (w - drawW) / 2;
    int drawY = y + (h - drawH) / 2;

    Gdiplus::Graphics graphics(_hdc);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    return graphics.DrawImage(bitmap, drawX, drawY, drawW, drawH) == Gdiplus::Ok;
}

// 释放通过 image() 加载并缓存的图片。
// 通常在主窗口关闭后调用。
static void clearImageCache() {
    for (auto& item : _imageCache)
        delete item.second;
    _imageCache.clear();
}
