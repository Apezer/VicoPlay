#pragma once

#include <Adafruit_SSD1306.h>
#include <cstdio>
#include "game_2048.h"

inline void draw2048Screen(Adafruit_SSD1306& display, const Game2048& game) {
    // 横向 32 像素单元格容纳完整的四位数字，纵向保留顶部计分栏。
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            const uint16_t value = game.cells[row][column];
            if (!value) {
                continue;
            }
            const int x = column * 32;
            const int y = 14 + row * 12;
            const bool inverse = value >= 128;
            if (inverse) {
                display.fillRect(x + 1, y + 1, 31, 11, WHITE);
            }
            char digits[6];
            snprintf(digits, sizeof(digits), "%u", static_cast<unsigned>(value));
            display.setTextColor(inverse ? BLACK : WHITE);
            display.setCursor(x + (32 - static_cast<int>(strlen(digits)) * 6) / 2, y + 3);
            display.print(digits);
        }
    }
    display.setTextColor(WHITE);
    for (int row = 0; row <= 4; ++row) {
        display.drawFastHLine(0, 14 + row * 12, 128, WHITE);
    }
    for (int column = 0; column <= 4; ++column) {
        display.fillRect(column == 4 ? 127 : column * 32, 14, 1, 49, WHITE);
    }
}
