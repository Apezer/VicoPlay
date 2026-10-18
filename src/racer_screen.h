#pragma once

#include <Adafruit_SSD1306.h>
#include "racer_game.h"

inline void racerRect(Adafruit_SSD1306& display, int x, int y, int w, int h, int color) {
    // 来车进入道路时裁掉状态栏以上的部分。
    const int bottom = y + h < 64 ? y + h : 64;
    const int top = y < 12 ? 12 : y;
    if (bottom > top) {
        display.fillRect(x, top, w, bottom - top, color);
    }
}

inline void drawRacerCar(Adafruit_SSD1306& display, int lane, int y) {
    const int x = 44 + lane * 16;
    racerRect(display, x + 2, y, 6, 12, WHITE);
    racerRect(display, x, y + 2, 10, 3, WHITE);
    racerRect(display, x, y + 8, 10, 3, WHITE);
    racerRect(display, x + 3, y + 3, 4, 2, BLACK);
    racerRect(display, x + 3, y + 8, 4, 2, BLACK);
}

inline void drawRacerScreen(Adafruit_SSD1306& display, const RacerGame& racer) {
    racerRect(display, 39, 12, 1, 52, WHITE);
    racerRect(display, 89, 12, 1, 52, WHITE);
    const int scroll = static_cast<int>(racer.distance) % 12;
    for (int y = 0; y < 76; y += 12) {
        racerRect(display, 56, y + scroll - 12, 1, 6, WHITE);
        racerRect(display, 72, y + scroll - 12, 1, 6, WHITE);
    }
    for (const auto& car : racer.traffic) {
        if (car.active) {
            drawRacerCar(display, car.lane, static_cast<int>(car.y));
        }
    }
    drawRacerCar(display, racer.lane, RacerGame::PLAYER_Y);
    display.setCursor(0, 17);
    display.print("PASS");
    display.setCursor(0, 28);
    display.print(racer.passed);
    display.setCursor(96, 17);
    display.print("SPD");
    display.setCursor(96, 28);
    display.print(static_cast<int>(racer.speed()));
    display.setCursor(0, 45);
    display.print("K1:<");
    display.setCursor(96, 45);
    display.print("K3:>");
}
