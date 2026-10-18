#pragma once

#include <Arduino.h>
#include <cmath>

namespace MenuAnimation {
constexpr int ROW_SPACING = 10;
constexpr uint32_t FRAME_MS = 28;
constexpr uint32_t LAUNCH_MS = 200;

// 显示坐标与选择索引分离：快速旋转只改变目标，保留当前速度。
class Motion {
  public:
    void enter(uint32_t now, int selection, int width) {
        enteredAt_ = updatedAt_ = now;
        y_ = 13 + selection * ROW_SPACING;
        scroll_ = selection > 3 ? (selection - 3) * ROW_SPACING : 0;
        scrollVelocity_ = 0;
        width_ = width;
        yVelocity_ = widthVelocity_ = 0;
        launching_ = false;
    }

    void update(uint32_t now, int selection, int width) {
        uint32_t remaining = now - updatedAt_;
        updatedAt_ = now;
        // 从熄屏或长阻塞恢复时不积累动画时间；小步积分保证弹簧稳定。
        if (remaining > 64) {
            remaining = 64;
        }
        while (remaining) {
            const uint32_t step = remaining > 8 ? 8 : remaining;
            spring(y_, yVelocity_, 13 + selection * ROW_SPACING, step * 0.001f);
            const float targetScroll = selection > 3 ? (selection - 3) * ROW_SPACING : 0;
            spring(scroll_, scrollVelocity_, targetScroll, step * 0.001f);
            spring(width_, widthVelocity_, width, step * 0.001f);
            remaining -= step;
        }
    }

    int y() const {
        return static_cast<int>(y_ - scroll_ + 0.5f);
    }

    int scroll() const {
        return static_cast<int>(scroll_ + 0.5f);
    }

    int width() const {
        return static_cast<int>(width_ + 0.5f);
    }

    int rowOffset(uint32_t now, int row) const {
        const uint32_t elapsed = now - enteredAt_;
        const uint32_t delay = row * 45;
        if (elapsed <= delay) {
            return 18;
        }
        if (elapsed >= delay + 280) {
            return 0;
        }
        const float rest = 1.0f - (elapsed - delay) / 280.0f;
        return static_cast<int>(18 * rest * rest * rest + 0.5f);
    }

    void launch(uint32_t now) {
        if (!launching_) {
            launchedAt_ = now;
            launching_ = true;
        }
    }

    bool launching() const {
        return launching_;
    }

    bool launchReady(uint32_t now) const {
        return launching_ && now - launchedAt_ >= LAUNCH_MS;
    }

    int wipeWidth(uint32_t now) const {
        if (!launching_) {
            return 0;
        }
        if (launchReady(now)) {
            return 128;
        }
        const float progress = static_cast<float>(now - launchedAt_) / LAUNCH_MS;
        const float eased = progress * progress * (3 - 2 * progress);
        return static_cast<int>(128 * eased);
    }

  private:
    static void spring(float& value, float& velocity, float target, float dt) {
        velocity += ((target - value) * 200 - velocity * 24) * dt;
        value += velocity * dt;
        if (fabsf(target - value) < 0.03f && fabsf(velocity) < 0.1f) {
            value = target;
            velocity = 0;
        }
    }

    float y_ = 13, width_ = 58;
    float scroll_ = 0, scrollVelocity_ = 0;
    float yVelocity_ = 0, widthVelocity_ = 0;
    uint32_t enteredAt_ = 0, updatedAt_ = 0, launchedAt_ = 0;
    bool launching_ = false;
};
} // namespace MenuAnimation
