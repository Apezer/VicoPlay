#pragma once

#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>
#include "boot_animation.h"

namespace BootAnimation {
// 单色版参考图：像素箭头、衬线字标、横线与微型副标题。
inline void draw(Adafruit_SSD1306& display, uint32_t elapsed) {
    constexpr int brandOffsetY = -9;
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);

    constexpr int pixelX[] = {10, 15, 20, 15, 10};
    constexpr int pixelY[] = {17, 22, 27, 32, 37};
    for (int i = 0; i < 5; ++i) {
        if (elapsed >= static_cast<uint32_t>(i * 70)) {
            display.fillRect(pixelX[i], pixelY[i], 5, 5, WHITE);
        }
    }

    if (elapsed >= RISE_START_MS) {
        display.setFont(&FreeSerif9pt7b);
        display.setCursor(36, 34 + brandOffsetY + titleOffset(elapsed));
        display.print("VicoPlay");
        display.setFont(nullptr);

        // 横线为固定裁切边界：只露出线以上的字形，形成从线中浮出的效果。
        display.fillRect(36, 40 + brandOffsetY, 92, 24 - brandOffsetY, BLACK);
    }
    if (elapsed >= LINE_START_MS) {
        const int width = elapsed >= RISE_START_MS
                              ? 84
                              : static_cast<int>((elapsed - LINE_START_MS) * 84 /
                                                 (RISE_START_MS - LINE_START_MS));
        display.drawFastHLine(36, 40 + brandOffsetY, width, WHITE);
    }

    if (elapsed >= RISE_END_MS) {
        // 3×5 大写字形，每字占 4 像素；副标题总宽 67 像素。
        constexpr char alphabet[] = "MIN GAECOSL";
        constexpr uint16_t glyphs[] = {0x5FED, 0x7497, 0x5FFD, 0x0000, 0x796F, 0x2BED,
                                       0x79A7, 0x7927, 0x7B6F, 0x79CF, 0x4927};
        constexpr char subtitle[] = "MINI GAME CONSOLE";
        const int count =
            elapsed >= DETAILS_END_MS
                ? 17
                : static_cast<int>((elapsed - RISE_END_MS) * 17 / (DETAILS_END_MS - RISE_END_MS));
        for (int i = 0; i < count; ++i) {
            const char* match = strchr(alphabet, subtitle[i]);
            if (!match) {
                continue;
            }
            const uint16_t bits = glyphs[match - alphabet];
            for (int y = 0; y < 5; ++y) {
                for (int x = 0; x < 3; ++x) {
                    if (bits & (1 << (14 - y * 3 - x))) {
                        display.fillRect(40 + i * 4 + x, 47 + brandOffsetY + y, 1, 1, WHITE);
                    }
                }
            }
        }
    }

    if (elapsed >= DETAILS_END_MS) {
        const int y = hintY(elapsed);
        // 箭头与文字作为一组浮动，不改变副标题的位置。
        display.drawFastHLine(102, y + 3, 7, WHITE);
        display.fillRect(106, y + 1, 1, 1, WHITE);
        display.fillRect(107, y + 2, 1, 1, WHITE);
        display.fillRect(107, y + 4, 1, 1, WHITE);
        display.fillRect(106, y + 5, 1, 1, WHITE);
        display.setCursor(18, y);
        display.print("PRESS ANY KEY");
    }
    display.display();
}
} // namespace BootAnimation
