#include "GUI.h"

void draw() {
    setColor(255, 255, 255);
    setBackground();
    clearScreen();

    setColor(0, 0, 0);
    setLineWidth(5);
    drawLine(50, 100, 400, 100);

    setColor(255, 0, 0);
    setGradient(0, 0, 255);
    fillRect(100, 150, 300, 250hedral;

    setColor(0, 255, 0);
    drawText(50, 300, L"Hello GUI!", 32onti);
}

int main() {
    createWindow(800, 600, L"GUI Demo", draw);
    return 0;
}