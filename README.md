# GUI

一个基于 Windows GDI/GDI+ 的轻量级 C++ 图形库。所有坐标均以窗口客户区左上角为原点：X 向右增加，Y 向下增加。所有绘图 API 都在 `GUI.h` 中��直接 `#include "GUI.h"` 即可。

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

## 窗口与绘图环境

| 函数 | 功能 |
| --- | --- |
| `window(int w, int h, const wchar_t* title, void (*draw)())` | 创建窗口并循环调用绘图回调，窗口关闭后返回。 |
| `clear()` | 使用当前背景色清空窗口客户区；通常在每帧绘图开始时调用。 |

## 颜色与画笔

| 函数 | 功能 |
| --- | --- |
| `color(int R, int G, int B, bool setBackground = false)` | 设置当前绘图颜色，RGB 分量范围为 0–255。可选参数为 `true` 时，同时设置清屏背景色。 |
| `width(int w)` | 设置后续线条和空心图形的线宽。 |
| `gradcolor(int R, int G, int B)` | 启用渐变填充并设置渐变终点颜色；渐变起点为当前绘图颜色。 |
| `nograd()` | 关闭渐变填充。 |

## 图形、文字和填充

| 函数 | 功能 |
| --- | --- |
| `line(int x1, int y1, int x2, int y2)` | 绘制从 `(x1,y1)` 到 `(x2,y2)` 的直线。 |
| `bend(int x1, int y1, int x2, int y2, int x3, int y3)` | 根据三个控制点绘制曲线。 |
| `text(int x, int y, const wchar_t* s, int size = 16)` | 绘制宽字符串；中文请使用 `L"文字"`。文字颜色为当前绘图颜色。 |
| `rect(int x1, int y1, int x2, int y2)` | 绘制空心矩形。 |
| `fullrect(int x1, int y1, int x2, int y2)` | 绘制纯色或渐变填充矩形。 |
| `circle(int x, int y, int r)` | 绘制以 `(x,y)` 为圆心、`r` 为半径的空心圆。 |
| `fullcircle(int x, int y, int r)` | 绘制纯色或渐变填充圆。 |
| `fill(int x, int y)` | 从指定点开始，将画布上相连的同色区域填为当前颜色。 |

## 鼠标

| 函数 | 功能 |
| --- | --- |
| `mousex()` | 返回鼠标在客户区的 X 坐标。 |
| `mousey()` | 返回鼠标在客户区的 Y 坐标。 |
| `mousedown()` | 左键按住时返回 `true`，否则返回 `false`。 |

## 输入框

`inputbox(int x, int y, int w, int h, wchar_t* buffer, int maxLen, int fontSize = 16)` 绘制文本输入框。点击后可输入文字，退格删除，回车或 Esc 结束焦点，内容同步到 `buffer`。`buffer` 必须是可写宽字符数组；其容量需与 `maxLen` 一致，建议不超过 256。

```cpp
#include "GUI.h"

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

## 图片

### 点击选择本地图片

`imagebox(int x, int y, int w, int h)` 绘制图片选择框。点击框后打开系统文件选择对话框，选取本地图片并按比例缩放显示。支持 PNG、JPEG、BMP、GIF 和 TIFF；不会上传到网络。当前实现维护一个图片框状态。

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

### 在代码中指定图片路径

`bool image(const wchar_t* path, int x, int y, int w, int h)` 从相对或绝对路径加载图片，按比例缩放并居中绘制到指定区域。成功返回 `true`，失败返回 `false`。同一路径重复绘制时会复用缓存；缓存会在 `window()` 退出时自动释放，无需调用清理函数。

```cpp
#include "GUI.h"
void draw() {
    clear();
    if (!image(L"assets/photo.png", 50, 50, 400, 300))
        text(50, 360, L"图片加载失败", 18);
}
int main() {
    window(800, 600, L"图片路径示例", draw);
    return 0;
}
```

相对路径基于程序的当前工作目录。绝对路径示例：

```cpp
image(L"C:\\Users\\Alice\\Pictures\\photo.jpg", 50, 50, 400, 300);
```

## 编译要求

此库使用 Windows API、Common Dialog 和 GDI+。Visual Studio 链接库已通过头文件声明；MinGW 通常需要添加：

```text
-lgdiplus -lcomdlg32
```

## 说明

`_proc`、`_applyPen` 及以下划线开头的全局变量属于内部实现，不是公开 API。