#include "../src/main.cpp"
#include <cassert>
#include <cstdio>
SerialStub Serial;
WireStub Wire;

void BatteryMonitor::begin() {}

bool BatteryMonitor::update(uint32_t) {
    return false;
}

void RotaryEncoder::begin() {}

void RotaryEncoder::discard() {}

int8_t RotaryEncoder::takeSteps() {
    return 0;
}

RotaryEncoder::ButtonEvent RotaryEncoder::updateButton(uint32_t) {
    return ButtonEvent::NONE;
}

void clearInput() {
    encoderSteps = 0;
    encoderPressed = false;
    for (auto& k : buttons) {
        k = {};
    }
}

int main() {
    MenuAnimation::Motion menu;
    menu.enter(0, 0, 58);
    assert(menu.y() == 13 && menu.width() == 58);
    assert(menu.rowOffset(0, 0) == 18);
    assert(menu.rowOffset(500, 2) == 0);
    // 尚未落位时反向选择，新目标应接管同一个连续运动状态。
    menu.update(32, 2, 46);
    const int movingY = menu.y();
    assert(movingY > 13 && movingY < 37);
    menu.update(32, 0, 58);
    assert(menu.y() == movingY);
    for (uint32_t time = 40; time <= 1800; time += 8) {
        menu.update(time, 1, 40);
        assert(menu.y() >= 13 && menu.y() <= 37);
    }
    assert(menu.y() == 25 && menu.width() == 40);
    menu.launch(1800);
    menu.launch(1900); // 重复确认不重置过渡计时。
    assert(!menu.launchReady(1999));
    assert(menu.wipeWidth(1900) == 64);
    assert(menu.launchReady(2000) && menu.wipeWidth(2000) == 128);
    menu.enter(UINT32_MAX - 49, 2, 46);
    menu.update(10, 0, 58);
    assert(menu.y() >= 13 && menu.y() <= 37);
    assert(!menu.launching());
    BootAnimation::Timeline timeline;
    timeline.begin(100);
    assert(!timeline.complete(100 + BootAnimation::DURATION_MS - 1, false));
    assert(timeline.complete(100 + BootAnimation::DURATION_MS, false));
    assert(!timeline.active());
    assert(!timeline.complete(1901, true));
    timeline.begin(2000);
    assert(timeline.complete(2001, true));
    timeline.begin(UINT32_MAX - 99);
    assert(timeline.elapsed(100) == 200);
    assert(!timeline.complete(100, false));
    assert(timeline.complete(BootAnimation::DURATION_MS - 100, false));
    assert(BootAnimation::titleOffset(0) == 21);
    assert(BootAnimation::titleOffset(BootAnimation::RISE_END_MS) == 0);
    int previousOffset = 21;
    for (uint32_t time = 0; time <= BootAnimation::DURATION_MS; time += 10) {
        const int offset = BootAnimation::titleOffset(time);
        assert(offset >= 0 && offset <= previousOffset);
        previousOffset = offset;
    }
    selectedGame = 1;
    startGame();
    clearInput();
    buttons[0].pressed = true;
    updateSnake(1000);
    assert(direction == 0 && snake[0].x == 13); // no reverse
    clearInput();
    snake[0] = {31, 6};
    direction = nextDirection = 0;
    updateSnake(2000);
    assert(scene == OVER);
    startGame();
    clearInput();
    food = {13, 6};
    updateSnake(1000);
    assert(snakeLength == 5 && score == 1);
    selectedGame = 2;
    startGame();
    clearInput();
    for (auto& row : board) {
        row = 0;
    }
    currentPiece = 0;
    rotation = 0;
    pieceX = 3;
    pieceY = 18;
    board[19] = 1023 & ~(15 << 3);
    lockPiece();
    assert(clearedLines == 1 && score == 100);
    for (int type = 0; type < 7; type++) {
        for (int r = 0; r < 4; r++) {
            int count = 0;
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 4; x++) {
                    count += isPieceCell(type, r, x, y);
                }
            }
            assert(count == 4);
        }
    }
    selectedGame = 0;
    startGame();
    clearInput();
    launched = true;
    lives = 1;
    ballY = 65;
    updateBreakout(1000);
    assert(scene == OVER);
    startGame();
    clearInput();
    encoderSteps = 100;
    updateBreakout(1000);
    assert(paddleX == 104);
    puts("PASS: snake reversal/wall/growth, tetris line clear/rotations, breakout lives/bounds");
}
