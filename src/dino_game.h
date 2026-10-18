#pragma once

#include <Arduino.h>

// 跑酷逻辑与绘图分离，使用固定小步推进，避免帧率变化导致跳跃和碰撞失真。
struct DinoGame {
    static constexpr int X = 18;
    static constexpr int WIDTH = 12;
    static constexpr int HEIGHT = 16;
    static constexpr int GROUND_Y = 58;

    float height = 0;
    float velocity = 0;
    float distance = 0;
    float obstacleX = 160;
    int obstacleWidth = 7;
    int obstacleHeight = 12;
    int score = 0;
    uint32_t lastUpdateAt = 0;
    uint32_t pendingMs = 0;
    bool crashed = false;

    void begin(uint32_t now) {
        height = velocity = distance = 0;
        obstacleX = 160;
        obstacleWidth = 7;
        obstacleHeight = 12;
        score = 0;
        lastUpdateAt = now;
        pendingMs = 0;
        crashed = false;
    }

    void resume(uint32_t now) {
        lastUpdateAt = now;
        pendingMs = 0;
    }

    float speed() const {
        return 48.0f + (score < 150 ? score : 150) * 0.35f;
    }

    bool collides() const {
        // 碰撞盒略小于外观，为头、尾和仙人掌分枝留出容错。
        return X + WIDTH - 2 > obstacleX + 1 && X + 2 < obstacleX + obstacleWidth - 1 &&
               height + 2 < obstacleHeight;
    }

    bool update(uint32_t now, bool jump) {
        if (crashed) {
            return true;
        }
        if (jump && height == 0) {
            velocity = 118.0f;
        }
        const uint32_t elapsed = now - lastUpdateAt;
        lastUpdateAt = now;
        pendingMs += elapsed > 64 ? 64 : elapsed;
        while (pendingMs >= 8) {
            pendingMs -= 8;
            constexpr float dt = 0.008f;
            if (velocity != 0 || height > 0) {
                height += velocity * dt;
                velocity -= 310.0f * dt;
                if (height < 0) {
                    height = velocity = 0;
                }
            }
            const float travel = speed() * dt;
            distance += travel;
            obstacleX -= travel;
            score = static_cast<int>(distance / 10);
            if (collides()) {
                crashed = true;
                return true;
            }
            if (obstacleX + obstacleWidth < 0) {
                obstacleX = 140 + random(45);
                obstacleWidth = random(2) ? 7 : 10;
                obstacleHeight = 10 + random(7);
            }
        }
        return false;
    }
};
