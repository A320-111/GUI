# GUI

一个基于 Windows GDI/GDI+ 的轻量级 C++ 图形库。所有坐标均以窗口客户区左上角为原点：X 向右增加，Y 向下增加。

## 快速开始

```cpp
#include "GUI.h"

void draw() {
    clear();

    color(20, 20, 20);
    width(3);
    line(30, 40, 300, 40);

    text(30, 70, L"Hello GUI", 24);
}

int main() {
    window(800, 600, L"GUI Demo", draw);
    return 0;
}
```

使用图片路径加载接口时，包含 `Image.h`：

```cpp
#include "Image.h"

void draw() {
    clear();
    image(L"assets/photo.png", 50, 50, 400, 300);
}

int main() {
    window(800, 600, L"图片示例", draw);
    clearImageCache();
    return 0;
}
```

`Image.h` 已包含 `GUI.h`，不要同时重复包含两者。

## 窗口与绘图环境

| 函数 | 功能 |
| --- | --- |
| `window(w, h, title, draw)` | 创建宽 `w`、高 `h`、标题为 `title` 的窗口，并反复调用绘图回调 `draw`。窗口关闭时结束循环。 |
| `clear()` | 使用当前背景色清空整个窗口客户区。建议每帧重绘前先调用。 |

## 颜色与画笔

| 函数 | 功能 |
| --- | --- |
| `color(R, G, B, setBackground = false)` | 设置当前绘图颜色。`R/G/B` 的范围是 0–255；传入 `true` 时，同时把该颜色设为背景色。 |
| `width(w)` | 设置后续直线、曲线和空心图形的线宽。 |
| `gradcolor(R, G, B)` | 启用渐变并设置终点颜色。填充图形会从当前 `color()` 颜色渐变到此颜色。 |
| `nograd()` | 关闭渐变填充。后续填充矩形和填充圆恢复为纯色。 |

## 基本绘图

| 函数 | 功能 |
| --- | --- |
| `line(x1, y1, x2, y2)` | 绘制一条直线。 |
| `bend(x1, y1, x2, y2, x3, y3)` | 绘制由三个控制点组成的曲线。 |
| `text(x, y, text, size = 16)` | 在指定位置绘制宽字符串；使用中文或特殊字符时请使用 `L"文字"`。文字颜色为当前画笔颜色。 |
| `rect(x1, y1, x2, y2)` | 绘制空心矩形。 |
| `fullrect(x1, y1, x2, y2)` | 绘制纯色或渐变填充矩形。 |
| `circle(x, y, r)` | 绘制空心圆。 |
| `fullcircle(x, y, r)` | 绘制纯色或渐变填充圆。 |
| `fill(x, y)` | 在指定点处执行油漆桶填充；把与该点相连且颜色相同的区域改为当前颜色。 |

## 鼠标

| 函数 | 功能 |
| --- | --- |
| `mousex()` | 返回鼠标在窗口客户区中的 X 坐标。 |
| `mousey()` | 返回鼠标在窗口客户区中的 Y 坐标。 |
| `mousedown()` | 左键按下时返回 `true`，松开时返回 `false`。 |

## 文本输入

| 函数 | 功能 |
| --- | --- |
| `inputbox(x, y, w, h, buffer, maxLen, fontSize = 16)` | 绘制文本输入框并接收键盘输入。点击框后可输入，退格删除，回车或 Esc 结束焦点。输入内容同步写入 `buffer`。 |

```cpp
wchar_t name[64] = L"";

void draw() {
    clear();
    inputbox(40, 40, 300, 40, name, 64, 18);
    text(40, 100, name, 18);
}

int main() {
    window(500, 300, L"输入框示例", draw);
    return 0;
}
```

注意：`buffer` 必须指向可写的宽字符数组，`maxLen` 和数组容量应保持一致。内部最多使用 256 个字符的输入缓冲，因此建议数组容量不超过 256。

## 图片选择与图片加载

### 通过文件选择框加载

| 函数 | 功能 |
| --- | --- |
| `imagebox(x, y, w, h)` | 绘制图片选择区域。点击区域会打开系统文件选择框；选取图片后按比例缩放并居中显示在框内。 |

支持 PNG、JPEG、BMP、GIF 和 TIFF。此功能只读取本地文件，不会上传到网络。

```cpp
#include "GUI.h"

void draw() {
    clear();
    imagebox(50, 50, 400, 300);
}

int main() {
    window(800, 600, L"图片选择示例", draw);
    return 0;
}
```

### 通过代码指定路径加载

| 函数 | 功能 |
| --- | --- |
| `image(path, x, y, w, h)` | 从指定路径加载图片并按比例缩放、居中绘制到矩形区域。加载成功返回 `true`，失败返回 `false`。同一路径会被缓存以减少重复读取。 |
| `clearImageCache()` | 释放 `image()` 创建的图片缓存。通常在窗口关闭后调用。 |

相对路径以进程的当前工作目录为基准；也可以使用绝对宽字符串路径。

```cpp
#include "Image.h"

void draw() {
    clear();

    if (!image(L"assets/photo.png", 50, 50, 400, 300)) {
        text(50, 360, L"图片加载失败", 18);
    }
}

int main() {
    window(800, 600, L"图片路径示例", draw);
    clearImageCache();
    return 0;
}
```

Windows 绝对路径示例：

```cpp
image(L"C:\\Users\\Alice\\Pictures\\photo.jpg", 50, 50, 400, 300);
```

## 编译要求

`GUI.h` 使用 Windows API、Common Dialog 和 GDI+。它已包含可直接使用的 Visual Studio 链接声明：

- `Comdlg32.lib`
- `Gdiplus.lib`

如果使用 MinGW 等工具链，链接时可使用：

```text
-lgdiplus -lcomdlg32
```

## 内部实现说明

以下名称属于当前头文件的内部实现，不建议作为公开 API 使用：

- `_proc`
- `_applyPen`
- `_hwnd`
- `_hdc`
- `_pen`
- `_inputText`
- `_uploadedImage`
- `_gdiplusToken`
- `_imageCache`

以后如果调整内部实现，这些名称可能发生变化。
