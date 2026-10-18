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
    Game2048 tiles;
    tiles.begin();
    int tileCount = 0;
    for (const auto& row : tiles.cells) {
        for (auto value : row) {
            if (value) {
                ++tileCount;
            }
        }
    }
    assert(tileCount == 2 && tiles.score == 0);
    tiles = {};
    tiles.cells[0][0] = tiles.cells[0][1] = tiles.cells[0][2] = tiles.cells[0][3] = 2;
    assert(tiles.slide(Game2048::LEFT));
    assert(tiles.cells[0][0] == 4 && tiles.cells[0][1] == 4 && tiles.cells[0][2] == 0);
    assert(tiles.score == 8); // 新形成的两个 4 不在同回合继续合成 8。
    tiles = {};
    tiles.cells[0][0] = tiles.cells[0][1] = 2;
    tiles.cells[0][2] = tiles.cells[0][3] = 4;
    tiles.slide(Game2048::RIGHT);
    assert(tiles.cells[0][3] == 8 && tiles.cells[0][2] == 4 && tiles.score == 12);
    tiles = {};
    tiles.cells[1][2] = tiles.cells[3][2] = 8;
    tiles.slide(Game2048::UP);
    assert(tiles.cells[0][2] == 16 && tiles.score == 16);
    tiles.slide(Game2048::DOWN);
    assert(tiles.cells[3][2] == 16);
    tiles = {};
    tiles.cells[0][0] = 2;
    assert(!tiles.move(Game2048::LEFT));
    assert(tiles.cells[0][0] == 2 && tiles.cells[0][1] == 0 && tiles.score == 0);
    assert(tiles.move(Game2048::RIGHT));
    tileCount = 0;
    for (const auto& row : tiles.cells) {
        for (auto value : row) {
            if (value) {
                ++tileCount;
            }
        }
    }
    assert(tileCount == 2); // 有效移动才生成新方块。
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            tiles.cells[y][x] = (x + y) % 2 ? 2 : 4;
        }
    }
    assert(!tiles.canMove() && !tiles.spawnTile());
    tiles.cells[0][0] = 2;
    assert(tiles.canMove());
    tiles = {};
    tiles.cells[0][0] = tiles.cells[0][1] = 1024;
    assert(tiles.move(Game2048::LEFT) && tiles.won && tiles.score == 2048);
    assert(!tiles.move(Game2048::DOWN));
    selectedGame = 5;
    startGame();
    score = 2048;
    finishGame(true);
    assert(scene == OVER && won && highScores[5] == 2048);
    RacerGame driving;
    driving.begin(0);
    assert(driving.lane == 1 && !driving.crashed);
    driving.update(0, -1);
    driving.update(0, -1);
    assert(driving.lane == 0);
    driving.update(0, 1);
    driving.update(0, 1);
    driving.update(0, 1);
    assert(driving.lane == 2);
    driving.traffic[0] = {2, 45, true};
    assert(driving.update(8, 0));
    driving.begin(0);
    driving.traffic[0] = {1, 45, true};
    assert(!driving.update(8, -1)); // 提前换道可避开来车。
    driving.begin(0);
    driving.traffic[0] = {2, 45, true};
    assert(driving.update(0, 1)); // 换入已占用车道会立即碰撞。
    driving.begin(0);
    driving.traffic[0] = {0, 63.9f, true};
    driving.update(8, 0);
    assert(driving.passed == 1 && driving.score >= 10 && driving.speed() > 42);
    const float pausedRoad = driving.distance;
    driving.resume(100000);
    driving.update(100000, 0);
    assert(driving.distance == pausedRoad);
    driving.begin(0);
    for (uint32_t time = 8; time <= 600; time += 8) {
        driving.update(time, 0);
    }
    int cars = 0;
    for (const auto& car : driving.traffic) {
        if (car.active) {
            ++cars;
        }
    }
    assert(cars == 1);
    driving.begin(UINT32_MAX - 7);
    driving.update(8, 0);
    assert(driving.distance > 0);
    selectedGame = 4;
    startGame();
    score = 99;
    finishGame();
    assert(highScores[4] == 99 && scene == OVER);
    DinoGame runner;
    runner.begin(0);
    runner.obstacleX = 1000;
    runner.update(16, true);
    assert(runner.height > 0 && runner.velocity > 0);
    const float jumpVelocity = runner.velocity;
    runner.update(32, true);
    assert(runner.velocity < jumpVelocity); // 空中重复按下不能二段跳。
    for (uint32_t time = 48; time <= 1000; time += 16) {
        runner.update(time, false);
    }
    assert(runner.height == 0 && runner.velocity == 0);
    assert(runner.score > 0 && runner.speed() > 48);
    const float pausedDistance = runner.distance;
    runner.resume(100000);
    runner.update(100000, false);
    assert(runner.distance == pausedDistance); // 恢复时不补算暂停期间的路程。
    runner.begin(0);
    runner.obstacleX = DinoGame::X + 4;
    assert(runner.update(8, false));
    assert(runner.crashed);
    runner.begin(0);
    runner.height = 20;
    runner.obstacleX = DinoGame::X + 4;
    assert(!runner.update(8, false)); // 跳得足够高时可越过障碍。
    runner.begin(0);
    runner.obstacleX = -20;
    runner.update(8, false);
    assert(runner.obstacleX >= 140 && runner.obstacleHeight >= 10 && runner.obstacleHeight <= 16);
    runner.begin(UINT32_MAX - 7);
    runner.update(8, true);
    assert(runner.height > 0); // millis() 回绕不影响时间步进。
    selectedGame = 3;
    startGame();
    assert(GAME_COUNT == 6 && scene == PLAY && score == 0 && !dino.crashed);
    dino.score = 123;
    score = dino.score;
    finishGame();
    assert(highScores[3] == 123 && scene == OVER);
    bool hintMovedUp = false, hintMovedDown = false;
    for (uint32_t time = 2000; time <= 5600; time += 40) {
        const int y = BootAnimation::hintY(time);
        assert(y >= 52 && y + 8 <= 64);
        assert(y == BootAnimation::hintY(time + BootAnimation::HINT_PERIOD_MS));
        hintMovedUp |= y == 52;
        hintMovedDown |= y == 54;
    }
    assert(hintMovedUp && hintMovedDown);
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
    assert(menu.y() == 23 && menu.width() == 40);
    menu.launch(1800);
    menu.launch(1900); // 重复确认不重置过渡计时。
    assert(!menu.launchReady(1999));
    assert(menu.wipeWidth(1900) == 64);
    assert(menu.launchReady(2000) && menu.wipeWidth(2000) == 128);
    menu.enter(UINT32_MAX - 49, 2, 46);
    menu.update(10, 0, 58);
    assert(menu.y() >= 13 && menu.y() <= 37);
    assert(!menu.launching());
    menu.enter(0, 3, 58);
    assert(menu.y() == 43 && menu.y() + 11 <= 55); // 第四项不覆盖底部最高分。
    menu.enter(0, 4, 40);
    assert(menu.y() == 43 && menu.scroll() == 10);
    for (uint32_t time = 8; time <= 2000; time += 8) {
        menu.update(time, 0, 58);
    }
    assert(menu.scroll() == 0 && menu.y() == 13);
    menu.enter(0, 5, 34);
    assert(menu.scroll() == 20 && menu.y() == 43);
    BootAnimation::Timeline timeline;
    timeline.begin(100);
    assert(!timeline.complete(false));
    assert(timeline.elapsed(60100) > BootAnimation::DURATION_MS);
    assert(!timeline.complete(false)); // 动画播放完甚至等待一分钟也不自动进菜单。
    assert(timeline.active());
    assert(timeline.complete(true));
    assert(!timeline.active());
    assert(!timeline.complete(true));
    timeline.begin(2000);
    assert(timeline.complete(true));
    timeline.begin(UINT32_MAX - 99);
    assert(timeline.elapsed(100) == 200);
    assert(!timeline.complete(false));
    assert(timeline.complete(true));
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
    puts("PASS: racer/dino physics/scoring, scroll menu/boot animations, snake/tetris/breakout");
}
