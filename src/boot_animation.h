#pragma once

#include <Arduino.h>

namespace BootAnimation {
constexpr uint32_t LINE_START_MS = 350;
constexpr uint32_t RISE_START_MS = 650;
constexpr uint32_t RISE_END_MS = 1650;
constexpr uint32_t DETAILS_END_MS = 2000;
constexpr uint32_t WIPE_START_MS = 3750;
constexpr uint32_t DURATION_MS = 4100;
constexpr uint32_t FRAME_MS = 40;

// 字标从横线下方上浮 21 像素，三次缓出使落位前逐渐减速。
inline int titleOffset(uint32_t elapsed) {
    if (elapsed <= RISE_START_MS) {
        return 21;
    }
    if (elapsed >= RISE_END_MS) {
        return 0;
    }
    const float remaining =
        static_cast<float>(RISE_END_MS - elapsed) / static_cast<float>(RISE_END_MS - RISE_START_MS);
    return static_cast<int>(21 * remaining * remaining * remaining + 0.5f);
}

// 独立计时状态，使用无符号差值兼容 millis() 回绕。
class Timeline {
  public:
    void begin(uint32_t now) {
        startedAt_ = now;
        active_ = true;
    }

    bool active() const {
        return active_;
    }

    uint32_t elapsed(uint32_t now) const {
        return now - startedAt_;
    }

    bool complete(uint32_t now, bool skip) {
        if (!active_ || (!skip && elapsed(now) < DURATION_MS)) {
            return false;
        }
        active_ = false;
        return true;
    }

  private:
    uint32_t startedAt_ = 0;
    bool active_ = false;
};
} // namespace BootAnimation
