# GUI

一个基于 Win32 GDI/GDI+ 的轻量级 C++ 绘图头文件库。使用前在程序中包含 `GUI.h`，并在 `main()` 中调用 `window()` 创建窗口。绘图函数通常放在传给 `window()` 的绘图回调中。

> 当前 `GUI.h` 中实际提供的接口使用小写名称，例如 `color()`、`line()`、`text()`。`setColor()`、`drawLine()`、`createWindow()` 等名称不是此版本头文件中的接口。

## 快速开始

```cpp
#include "GUI.h"

void draw() {
    clear();
    color(0, 0, 0);
    width(3);
    line(30, 40, 250, 40);
    text(30, 70, L"Hello GUI", 24);
}

int main() {
    window(800, 600, L"GUI Demo", draw);
    return 0;
}
```

图片选择框和图片显示使用 GDI+，已经在 `GUI.h` 里初始化。Visual Studio 使用头文件中的自动链接声明；MinGW 通常需要链接 `-lgdiplus -lcomdlg32`。

## 窗口与清屏

| 函数 | 功能 |
| --- | --- |
| `window(int w, int h, const wchar_t* title, void (*draw)())` | 创建指定宽高和标题的窗口，并在窗口运行期间反复调用 `draw`。绘图函数应放在回调中。窗口关闭后返回。 |
| `clear()` | 用当前背景色清空整个绘图区域。每帧重绘时可先调用它。 |

## 颜色与线条

| 函数 | 功能 |
| --- | --- |
| `color(int R, int G, int B, bool setBackground = false)` | 设置当前绘图颜色，RGB 分量范围为 0–255。第三个可选参数设为 `true` 时，同时把该颜色设为 `clear()` 使用的背景色。 |
| `width(int w)` | 设置线宽，影响后续线条和未填充图形的边框。 |
| `gradcolor(int R, int G, int B)` | 打开渐变填充，并设置渐变终点颜色；渐变从当前 `color()` 颜色过渡到这里设置的颜色。 |
| `nograd()` | 关闭渐变填充。 |
| `line(int x1, int y1, int x2, int y2)` | 从 `(x1, y1)` 绘制到 `(x2, y2)` 的直线。 |
| `bend(int x1, int y1, int x2, int y2, int x3, int y3)` | 绘制一条由三个控制点生成的曲线，按 `(x1,y1)`、`(x2,y2)`、`(x3,y3)` 指定路径控制点。 |

## 形状与填充

| 函数 | 功能 |
| --- | --- |
| `rect(int x1, int y1, int x2, int y2)` | 绘制左上角到右下角为 `(x1,y1)`、`(x2,y2)` 的空心矩形。 |
| `fullrect(int x1, int y1, int x2, int y2)` | 绘制填充矩形。普通模式使用当前颜色；调用 `gradcolor()` 后使用从当前颜色到渐变终点色的纵向渐变。 |
| `circle(int x, int y, int r)` | 以 `(x,y)` 为圆心、`r` 为半径绘制空心圆。 |
| `fullcircle(int x, int y, int r)` | 绘制填充圆；启用渐变后按纵向渐变填充。 |
| `fill(int x, int y)` | 油漆桶填充：从指定点开始，将画布上相连的同色区域替换为当前 `color()` 颜色。 |

## 文字

| 函数 | 功能 |
| --- | --- |
| `text(int x, int y, const wchar_t* s, int size = 16)` | 在 `(x,y)` 绘制宽字符串（使用 `L"..."` 字面量），`size` 为字号；优先尝试微软雅黑、黑体等字体。文字颜色使用当前绘图颜色。 |

## 鼠标

| 函数 | 功能 |
| --- | --- |
| `mousex()` | 返回鼠标指针在窗口客户区中的 X 坐标。 |
| `mousey()` | 返回鼠标指针在窗口客户区中的 Y 坐标。 |
| `mousedown()` | 左键按下期间返回 `true`，否则返回 `false`。 |

## 图片选择与显示

| 函数 | 功能 |
| --- | --- |
| `imagebox(int x, int y, int w, int h)` | 在指定矩形区域绘制图片框。点击框后打开系统文件选择对话框，选取本地图片并在框内按比例缩放显示。支持 PNG、JPEG、BMP、GIF、TIFF。 |

示例：

```cpp
#include "GUI.h"

void draw() {
    clear();
    imagebox(50, 50, 400, 300); // 点击框选择本地图片
}

int main() {
    window(800, 600, L"图片选择示例", draw);
    return 0;
}
```

这里的“选择图片”是从本地文件系统读取并显示，不会将图片上传到网络。当前实现只有一个图片框状态；多次调用 `imagebox()` 时，点击区域是最近一次绘制的图片框。

## 输入框

| 函数 | 功能 |
| --- | --- |
| `inputbox(int x, int y, int w, int h, wchar_t* buffer, int maxLen, int fontSize = 16)` | 绘制并启用一个文本输入框。点击框后可键入文字，退格删除，回车或 Esc 结束输入；文本同步写入 `buffer`。`buffer` 必须是可写的宽字符数组，建议 `maxLen` 不超过 256，且不得超过数组容量。 |

示例：

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

## 说明

- 坐标原点 `(0,0)` 在窗口客户区左上角，X 向右增加，Y 向下增加。
- 颜色、画笔宽度等设置会影响之后绘制的图形。
- `_proc`、`_applyPen` 及以下划线开头的全局变量属于内部实现细节，不是供调用的绘图接口。
