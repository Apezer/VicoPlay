#pragma once

#include <Arduino.h>

struct RacerGame {
    static constexpr int PLAYER_Y = 47;
    static constexpr int CAR_HEIGHT = 12;

    struct Traffic {
        int lane = 0;
        float y = -20;
        bool active = false;
    } traffic[3];

    int lane = 1;
    int passed = 0;
    int score = 0;
    float distance = 0;
    uint32_t lastUpdateAt = 0, pendingMs = 0, spawnMs = 600;
    bool crashed = false;

    void begin(uint32_t now) {
        for (auto& car : traffic) {
            car = {};
        }
        lane = 1;
        passed = score = 0;
        distance = 0;
        lastUpdateAt = now;
        pendingMs = 0;
        spawnMs = 600;
        crashed = false;
    }

    void resume(uint32_t now) {
        lastUpdateAt = now;
        pendingMs = 0;
    }

    float speed() const {
        return 42.0f + (score < 200 ? score : 200) * 0.28f;
    }

    bool collides() const {
        for (const auto& car : traffic) {
            if (car.active && car.lane == lane && car.y + CAR_HEIGHT - 1 > PLAYER_Y + 1 &&
                car.y + 1 < PLAYER_Y + CAR_HEIGHT - 1) {
                return true;
            }
        }
        return false;
    }

    bool update(uint32_t now, int shift) {
        if (crashed) {
            return true;
        }
        lane = constrain(lane + shift, 0, 2);
        // 换道同样检查碰撞，禁止通过快速输入穿过同一帧中的来车。
        if (collides()) {
            return crashed = true;
        }
        const uint32_t elapsed = now - lastUpdateAt;
        lastUpdateAt = now;
        pendingMs += elapsed > 64 ? 64 : elapsed;
        while (pendingMs >= 8) {
            pendingMs -= 8;
            const float travel = speed() * 0.008f;
            distance += travel;
            for (auto& car : traffic) {
                if (!car.active) {
                    continue;
                }
                car.y += travel;
                if (car.y >= 64) {
                    car.active = false;
                    ++passed;
                }
            }
            score = passed * 10 + static_cast<int>(distance / 25);
            if (collides()) {
                return crashed = true;
            }
            if (spawnMs > 8) {
                spawnMs -= 8;
            } else {
                // 每次只生成一辆；车距大于车身，保留可通过的车道。
                for (auto& car : traffic) {
                    if (!car.active) {
                        car.lane = static_cast<int>(random(3));
                        car.y = 0;
                        car.active = true;
                        break;
                    }
                }
                spawnMs = 1100 + random(350);
            }
        }
        return false;
    }
};
