#pragma once

#include <Arduino.h>
#include <cstring>

struct Game2048 {
    enum Direction { LEFT, RIGHT, UP, DOWN };

    uint16_t cells[4][4] = {};
    int score = 0;
    bool won = false;

    void begin() {
        memset(cells, 0, sizeof(cells));
        score = 0;
        won = false;
        spawnTile();
        spawnTile();
    }

    bool spawnTile() {
        int emptyCount = 0;
        for (const auto& row : cells) {
            for (auto value : row) {
                if (!value) {
                    ++emptyCount;
                }
            }
        }
        if (!emptyCount) {
            return false;
        }
        int target = random(emptyCount);
        for (auto& row : cells) {
            for (auto& value : row) {
                if (!value && target-- == 0) {
                    value = random(10) == 0 ? 4 : 2;
                    return true;
                }
            }
        }
        return false;
    }

    bool canMove() const {
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                if (!cells[y][x]) {
                    return true;
                }
                if (x < 3 && cells[y][x] == cells[y][x + 1]) {
                    return true;
                }
                if (y < 3 && cells[y][x] == cells[y + 1][x]) {
                    return true;
                }
            }
        }
        return false;
    }

    // 将四种方向映射为四条从移动端开始读取的直线。
    uint16_t& at(Direction direction, int line, int position) {
        switch (direction) {
        case LEFT:
            return cells[line][position];
        case RIGHT:
            return cells[line][3 - position];
        case UP:
            return cells[position][line];
        default:
            return cells[3 - position][line];
        }
    }

    bool slide(Direction direction) {
        bool changed = false;
        for (int line = 0; line < 4; ++line) {
            uint16_t packed[4] = {}, result[4] = {};
            int count = 0, output = 0;
            for (int i = 0; i < 4; ++i) {
                const uint16_t value = at(direction, line, i);
                if (value) {
                    packed[count++] = value;
                }
            }
            for (int i = 0; i < count; ++i) {
                uint16_t value = packed[i];
                if (i + 1 < count && value == packed[i + 1]) {
                    value *= 2;
                    score += value;
                    ++i; // 新合并的方块本回合不再参与合并。
                }
                result[output++] = value;
                if (value >= 2048) {
                    won = true;
                }
            }
            for (int i = 0; i < 4; ++i) {
                uint16_t& value = at(direction, line, i);
                changed = changed || value != result[i];
                value = result[i];
            }
        }
        return changed;
    }

    bool move(Direction direction) {
        if (won || !slide(direction)) {
            return false;
        }
        if (!won) {
            spawnTile();
        }
        return true;
    }
};
