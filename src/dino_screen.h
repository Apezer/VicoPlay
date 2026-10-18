#pragma once

#include <Adafruit_SSD1306.h>
#include "dino_game.h"

inline void drawDinoScreen(Adafruit_SSD1306& display, const DinoGame& dino) {
    const int y = DinoGame::GROUND_Y - DinoGame::HEIGHT - static_cast<int>(dino.height);
    const int x = DinoGame::X;
    // 自绘像素恐龙：头、身体、尾巴和交替奔跑的双腿。
    display.fillRect(x + 6, y, 6, 6, WHITE);
    display.fillRect(x + 10, y + 1, 1, 1, BLACK);
    display.fillRect(x + 9, y + 4, 3, 1, BLACK);
    display.fillRect(x + 4, y + 5, 5, 7, WHITE);
    display.fillRect(x + 2, y + 8, 3, 3, WHITE);
    display.fillRect(x, y + 6, 2, 3, WHITE);
    display.fillRect(x + 8, y + 7, 3, 1, WHITE);
    const bool stride = (static_cast<int>(dino.distance) / 6) % 2;
    display.fillRect(x + 4, y + 12, 2, dino.height > 0 || stride ? 4 : 2, WHITE);
    display.fillRect(x + 7, y + 12, 2, dino.height > 0 || !stride ? 4 : 2, WHITE);

    display.drawFastHLine(0, DinoGame::GROUND_Y, 128, WHITE);
    for (int i = 0; i < 5; ++i) {
        int groundX = (i * 29 - static_cast<int>(dino.distance)) % 145;
        if (groundX < 0) {
            groundX += 145;
        }
        display.drawFastHLine(groundX, 61, 3, WHITE);
    }
    const int cactusX = static_cast<int>(dino.obstacleX);
    const int cactusY = DinoGame::GROUND_Y - dino.obstacleHeight;
    display.fillRect(cactusX + 2, cactusY, dino.obstacleWidth - 4, dino.obstacleHeight, WHITE);
    display.fillRect(cactusX, cactusY + 3, 2, 5, WHITE);
    display.fillRect(cactusX, cactusY + 7, dino.obstacleWidth, 2, WHITE);
    display.fillRect(cactusX + dino.obstacleWidth - 2, cactusY + 1, 2, 7, WHITE);
    if (dino.distance < 40) {
        display.setCursor(49, 18);
        display.print("K4: JUMP");
    }
}
