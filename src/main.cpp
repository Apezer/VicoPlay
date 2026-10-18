#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <esp_system.h>
#include "rotary_encoder.h"
#include "battery_monitor.h"
#include "boot_animation.h"
#include "boot_screen.h"

BootAnimation::Timeline bootAnimation;

// 硬件与公共运行状态
// 按键数组按 K1～K8 排列，与原键盘固件编号保持一致。
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
BatteryMonitor battery;
Preferences preferences;
constexpr uint8_t KEY_PINS[] = {18, 17, 16, 15, 5, 6, 7, 4};

struct ButtonState {
    bool raw = false;     // 当前 GPIO 采样值。
    bool held = false;    // 消抖后的持续按下状态。
    bool pressed = false; // 仅在按下边沿置位一个循环。
    uint32_t changedAt = 0;
    uint32_t repeatedAt = 0;
} buttons[8];

enum Scene { MENU, PLAY, PAUSED, OVER } scene = MENU;

const char* GAME_NAMES[] = {"BREAKOUT", "SNAKE", "TETRIS"};
const char* SCORE_STORAGE_KEYS[] = {"breakout", "snake", "tetris"};
int selectedGame = 0;
int score = 0;
int highScores[3] = {};
uint32_t lastGameUpdateAt = 0;
uint32_t lastFrameAt = 0;
uint32_t lastActivityAt = 0;
bool sleeping = false, won = false;
int encoderSteps = 0;
bool encoderPressed = false;

// 公共绘图、结束处理与输入消抖
void drawText(int x, int y, const char* s) {
    oled.setCursor(x, y);
    oled.print(s);
}

void finishGame(bool victory = false) {
    won = victory;
    scene = OVER;
    if (score > highScores[selectedGame]) {
        highScores[selectedGame] = score;
        preferences.putInt(SCORE_STORAGE_KEYS[selectedGame], score);
    }
}

bool buttonRepeat(int k, uint32_t now) {
    if (buttons[k].pressed) {
        buttons[k].repeatedAt = now;
        return true;
    }
    if (buttons[k].held && now - buttons[k].repeatedAt >= 140) {
        buttons[k].repeatedAt = now;
        return true;
    }
    return false;
}

void scanInputs(uint32_t now) {
    for (int i = 0; i < 8; i++) {
        auto& k = buttons[i];
        k.pressed = false;
        bool v = digitalRead(KEY_PINS[i]) == LOW;
        if (v != k.raw) {
            k.raw = v;
            k.changedAt = now;
        }
        if (v != k.held && now - k.changedAt >= 10) {
            k.held = v;
            k.pressed = v;
            lastActivityAt = now;
        }
    }
    encoderSteps = RotaryEncoder::takeSteps();
    encoderPressed = RotaryEncoder::updateButton(now) == RotaryEncoder::ButtonEvent::PRESSED;
    if (encoderSteps || encoderPressed) {
        lastActivityAt = now;
    }
}

// 打砖块：每次更新分成两个子步，降低球穿过砖块的风险。
float ballX, ballY, ballVelocityX, ballVelocityY, paddleX;
bool bricks[4][10], launched = false;
int lives = 3, remainingBricks = 40;

void resetBall() {
    ballX = paddleX + 12;
    ballY = 56;
    ballVelocityX = 0.9f;
    ballVelocityY = -1.15f;
    launched = false;
}

void initBreakout() {
    paddleX = 52;
    lives = 3;
    remainingBricks = 40;
    for (auto& row : bricks) {
        for (bool& b : row) {
            b = true;
        }
    }
    resetBall();
}

void updateBreakout(uint32_t now) {
    paddleX = constrain(paddleX + encoderSteps * 4, 0.0f, 104.0f);
    if (!launched) {
        ballX = paddleX + 12;
        if (encoderPressed || buttons[3].pressed || buttons[5].pressed) {
            launched = true;
        }
    }
    if (now - lastGameUpdateAt < 16) {
        return;
    }
    lastGameUpdateAt = now;
    paddleX =
        constrain(paddleX + (buttons[2].held ? 2 : 0) - (buttons[0].held ? 2 : 0), 0.0f, 104.0f);
    if (!launched) {
        ballX = paddleX + 12;
        return;
    }
    for (int sub = 0; sub < 2; sub++) {
        float oldY = ballY;
        ballX += ballVelocityX * 0.5f;
        ballY += ballVelocityY * 0.5f;
        if (ballX < 1) {
            ballX = 1;
            ballVelocityX = fabsf(ballVelocityX);
        }
        if (ballX > 126) {
            ballX = 126;
            ballVelocityX = -fabsf(ballVelocityX);
        }
        if (ballY < 12) {
            ballY = 12;
            ballVelocityY = fabsf(ballVelocityY);
        }
        if (ballVelocityY > 0 && oldY <= 58 && ballY >= 58 && ballX >= paddleX - 1 &&
            ballX <= paddleX + 25) {
            ballY = 58;
            ballVelocityY = -fabsf(ballVelocityY);
            ballVelocityX = (ballX - (paddleX + 12)) * 0.12f;
        }
        bool hit = false;
        for (int r = 0; r < 4 && !hit; r++) {
            for (int c = 0; c < 10; c++) {
                if (bricks[r][c] && ballX >= c * 12 + 4 && ballX <= c * 12 + 14 &&
                    ballY >= r * 7 + 16 && ballY <= r * 7 + 21) {
                    bricks[r][c] = false;
                    ballVelocityY = -ballVelocityY;
                    score += 10;
                    remainingBricks--;
                    hit = true;
                    break;
                }
            }
        }
        if (!remainingBricks) {
            finishGame(true);
            return;
        }
        if (ballY > 64) {
            if (--lives == 0) {
                finishGame();
                return;
            }
            resetBall();
            return;
        }
    }
}

void drawBreakout() {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 10; c++) {
            if (bricks[r][c]) {
                oled.fillRect(c * 12 + 4, r * 7 + 16, 10, 5, WHITE);
            }
        }
    }
    oled.fillRect((int)paddleX, 60, 24, 3, WHITE);
    oled.fillCircle((int)ballX, (int)ballY, 1, WHITE);
    if (!launched) {
        drawText(24, 47, "K4: LAUNCH");
    }
    oled.setCursor(107, 0);
    oled.print(lives);
    oled.print("HP");
}

// 贪吃蛇：状态栏下方为 32 × 13 网格，方向编号为右、下、左、上。
struct Cell {
    int x, y;
} snake[416], food;

int snakeLength = 4, direction = 0, nextDirection = 0;

bool sameCell(Cell a, Cell b) {
    return a.x == b.x && a.y == b.y;
}

void spawnFood() {
    int freeCount = 416 - snakeLength;
    if (freeCount <= 0) {
        finishGame(true);
        return;
    }
    int target = random(freeCount);
    for (int y = 0; y < 13; y++) {
        for (int x = 0; x < 32; x++) {
            bool occupied = false;
            for (int i = 0; i < snakeLength; i++) {
                if (sameCell({x, y}, snake[i])) {
                    occupied = true;
                    break;
                }
            }
            if (!occupied && target-- == 0) {
                food = {x, y};
                return;
            }
        }
    }
}

void initSnake() {
    snakeLength = 4;
    direction = nextDirection = 0;
    for (int i = 0; i < snakeLength; i++) {
        snake[i] = {12 - i, 6};
    }
    spawnFood();
}

void updateSnake(uint32_t now) {
    int requested = nextDirection;
    if (buttons[2].pressed) {
        requested = 0;
    }
    if (buttons[1].pressed) {
        requested = 1;
    }
    if (buttons[0].pressed) {
        requested = 2;
    }
    if (buttons[5].pressed) {
        requested = 3;
    }
    if (encoderSteps) {
        requested = (direction + (encoderSteps > 0 ? 1 : 3)) % 4;
    }
    if ((requested + 2) % 4 != direction) {
        nextDirection = requested;
    }
    if (now - lastGameUpdateAt < (uint32_t)max(65, 190 - score * 2)) {
        return;
    }
    lastGameUpdateAt = now;
    direction = nextDirection;
    const int dx[] = {1, 0, -1, 0}, dy[] = {0, 1, 0, -1};
    Cell head = {snake[0].x + dx[direction], snake[0].y + dy[direction]};
    bool eat = sameCell(head, food);
    if (head.x < 0 || head.x >= 32 || head.y < 0 || head.y >= 13) {
        finishGame();
        return;
    }
    // 不进食时尾部会在本步移走，因此允许蛇头进入当前尾部格。
    for (int i = 0; i < snakeLength - (eat ? 0 : 1); i++) {
        if (sameCell(head, snake[i])) {
            finishGame();
            return;
        }
    }
    if (eat) {
        snakeLength++;
        score++;
    }
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = head;
    if (eat) {
        spawnFood();
    }
}

void drawSnake() {
    for (int i = 0; i < snakeLength; i++) {
        oled.fillRect(snake[i].x * 4, 12 + snake[i].y * 4, 3, 3, WHITE);
    }
    oled.drawRect(food.x * 4, 12 + food.y * 4, 4, 4, WHITE);
}

// 俄罗斯方块：10 × 20 棋盘，每行低 10 位表示占用，方块使用 4 × 4 位图。
uint16_t board[20];
constexpr uint16_t shapes[] = {0x00F0, 0x0066, 0x0072, 0x0071, 0x0074, 0x0036, 0x0063};
int currentPiece, nextPiece, rotation, pieceX, pieceY, clearedLines;

bool isPieceCell(int type, int rot, int x, int y) {
    for (int i = 0; i < rot; i++) {
        int old = x;
        x = y;
        y = 3 - old;
    }
    return (shapes[type] >> (y * 4 + x)) & 1;
}

bool canPlacePiece(int x, int y, int rot) {
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            if (isPieceCell(currentPiece, rot, i, j)) {
                int xx = x + i, yy = y + j;
                if (xx < 0 || xx >= 10 || yy >= 20) {
                    return false;
                }
                if (yy >= 0 && (board[yy] & (1 << xx))) {
                    return false;
                }
            }
        }
    }
    return true;
}

void spawnPiece() {
    currentPiece = nextPiece;
    nextPiece = random(7);
    pieceX = 3;
    pieceY = -1;
    rotation = 0;
    if (!canPlacePiece(pieceX, pieceY, rotation)) {
        finishGame();
    }
}

void initTetris() {
    memset(board, 0, sizeof(board));
    clearedLines = 0;
    nextPiece = random(7);
    spawnPiece();
}

void lockPiece() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (isPieceCell(currentPiece, rotation, x, y)) {
                if (pieceY + y < 0) {
                    finishGame();
                    return;
                }
                board[pieceY + y] |= 1 << (pieceX + x);
            }
        }
    }
    int cleared = 0;
    for (int y = 19; y >= 0; y--) {
        if (board[y] == 1023) {
            for (int j = y; j > 0; j--) {
                board[j] = board[j - 1];
            }
            board[0] = 0;
            y++;
            cleared++;
        }
    }
    const int points[] = {0, 100, 300, 500, 800};
    score += points[cleared];
    clearedLines += cleared;
    spawnPiece();
}

void updateTetris(uint32_t now) {
    int shift = (buttonRepeat(2, now) ? 1 : 0) - (buttonRepeat(0, now) ? 1 : 0);
    if (encoderSteps) {
        shift = encoderSteps > 0 ? 1 : -1;
    }
    if (shift && canPlacePiece(pieceX + shift, pieceY, rotation)) {
        pieceX += shift;
    }
    if (encoderPressed || buttons[5].pressed || buttons[3].pressed) {
        int r = (rotation + 1) % 4;
        if (canPlacePiece(pieceX, pieceY, r)) {
            rotation = r;
        } else if (canPlacePiece(pieceX - 1, pieceY, r)) {
            pieceX--;
            rotation = r;
        } else if (canPlacePiece(pieceX + 1, pieceY, r)) {
            pieceX++;
            rotation = r;
        }
    }
    if (buttons[4].pressed) {
        while (canPlacePiece(pieceX, pieceY + 1, rotation)) {
            pieceY++;
            score += 2;
        }
        lockPiece();
        lastGameUpdateAt = now;
        return;
    }
    uint32_t interval = buttons[1].held ? 45 : max(100, 650 - clearedLines * 15);
    if (now - lastGameUpdateAt < interval) {
        return;
    }
    lastGameUpdateAt = now;
    if (canPlacePiece(pieceX, pieceY + 1, rotation)) {
        pieceY++;
    } else {
        lockPiece();
    }
}

void drawTetris() {
    oled.drawRect(49, 12, 22, 42, WHITE);
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 10; x++) {
            if (board[y] & (1 << x)) {
                oled.fillRect(50 + x * 2, 13 + y * 2, 2, 2, WHITE);
            }
        }
    }
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (isPieceCell(currentPiece, rotation, x, y) && pieceY + y >= 0) {
                oled.fillRect(50 + (pieceX + x) * 2, 13 + (pieceY + y) * 2, 2, 2, WHITE);
            }
        }
    }
    drawText(80, 15, "NEXT");
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (isPieceCell(nextPiece, 0, x, y)) {
                oled.fillRect(84 + x * 3, 26 + y * 3, 2, 2, WHITE);
            }
        }
    }
    drawText(0, 19, "LINES");
    oled.setCursor(0, 30);
    oled.print(clearedLines);
    drawText(0, 47, "K5:DROP");
    drawText(80, 47, "K6:ROT");
}

// 游戏切换与菜单绘制
void startGame() {
    score = 0;
    won = false;
    scene = PLAY;
    lastGameUpdateAt = millis();
    if (selectedGame == 0) {
        initBreakout();
    }
    if (selectedGame == 1) {
        initSnake();
    }
    if (selectedGame == 2) {
        initTetris();
    }
}

void drawHeader() {
    oled.setCursor(0, 0);
    oled.print(scene == MENU ? "VICO PLAY" : "S:");
    if (scene != MENU) {
        oled.print(score);
    }
    oled.drawFastHLine(0, 10, 128, WHITE);
}

void drawScreen() {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    drawHeader();
    if (scene == MENU) {
        if (battery.valid()) {
            oled.setCursor(97, 0);
            oled.print(battery.percent());
            oled.print('%');
        }
        for (int i = 0; i < 3; i++) {
            int y = 15 + i * 12;
            if (i == selectedGame) {
                oled.fillRoundRect(0, y - 2, 128, 11, 2, WHITE);
                oled.setTextColor(BLACK);
            }
            drawText(5, y, GAME_NAMES[i]);
            oled.setTextColor(WHITE);
        }
        oled.setCursor(0, 55);
        oled.print("BEST ");
        oled.print(highScores[selectedGame]);
        drawText(77, 55, "SW:PLAY");
    } else if (scene == PLAY) {
        if (selectedGame == 0) {
            drawBreakout();
        }
        if (selectedGame == 1) {
            drawSnake();
        }
        if (selectedGame == 2) {
            drawTetris();
        }
    } else {
        drawText(25, 17, scene == PAUSED ? "PAUSED" : won ? "YOU WIN!" : "GAME OVER");
        oled.setCursor(18, 30);
        oled.print("SCORE ");
        oled.print(score);
        drawText(8, 43, scene == PAUSED ? "SW/K4: RESUME" : "SW/K4: RETRY");
        drawText(8, 55, "K8: MENU");
    }
    oled.display();
}

// Arduino 入口：初始化外设，主循环依次处理输入、状态和画面。
void setup() {
    Serial.begin(115200);
    randomSeed(esp_random());
    for (auto pin : KEY_PINS) {
        pinMode(pin, INPUT_PULLUP);
    }
    pinMode(38, INPUT_PULLDOWN);
    pinMode(8, OUTPUT);
    digitalWrite(8, LOW);
    RotaryEncoder::begin();
    Wire.begin(10, 9);
    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED init failed");
        while (true) {
            delay(1000);
        }
    }
    oled.clearDisplay();
    oled.display();
    battery.begin();
    preferences.begin("vico-game", false);
    for (int i = 0; i < 3; i++) {
        highScores[i] = preferences.getInt(SCORE_STORAGE_KEYS[i], 0);
    }
    lastActivityAt = millis();
    bootAnimation.begin(lastActivityAt);
    BootAnimation::draw(oled, 0);
}

void loop() {
    uint32_t now = millis();
    scanInputs(now);
    battery.update(now);
    if (bootAnimation.active()) {
        bool skip = encoderPressed;
        for (const auto& button : buttons) {
            skip = skip || button.held;
        }
        if (bootAnimation.complete(now, skip)) {
            // 本次按下仅用于跳过；下一循环不会将它再次当作菜单确认。
            encoderSteps = 0;
            encoderPressed = false;
            RotaryEncoder::discard();
            lastActivityAt = now;
            lastFrameAt = now;
            drawScreen();
        } else if (now - lastFrameAt >= BootAnimation::FRAME_MS) {
            lastFrameAt = now;
            BootAnimation::draw(oled, bootAnimation.elapsed(now));
        }
        delay(1);
        return;
    }
    // 唤醒时消费本次输入，避免同一次按键触发菜单操作。
    if (sleeping) {
        if (now - lastActivityAt < 100) {
            sleeping = false;
            oled.ssd1306_command(SSD1306_DISPLAYON);
            encoderSteps = 0;
            encoderPressed = false;
            for (auto& k : buttons) {
                k.pressed = false;
            }
        } else {
            delay(1);
            return;
        }
    }
    if (scene == MENU) {
        int delta = encoderSteps + (buttons[1].pressed ? 1 : 0) - (buttons[5].pressed ? 1 : 0);
        selectedGame = ((selectedGame + delta) % 3 + 3) % 3;
        if (encoderPressed || buttons[3].pressed) {
            startGame();
        }
    } else if (buttons[7].pressed) {
        scene = MENU;
    } else if (scene == PLAY) {
        if (buttons[6].pressed) {
            scene = PAUSED;
        } else {
            if (selectedGame == 0) {
                updateBreakout(now);
            }
            if (selectedGame == 1) {
                updateSnake(now);
            }
            if (selectedGame == 2) {
                updateTetris(now);
            }
        }
    } else if (encoderPressed || buttons[3].pressed || buttons[6].pressed) {
        if (scene == PAUSED) {
            scene = PLAY;
            lastGameUpdateAt = now;
        } else {
            startGame();
        }
    }
    if (scene != PLAY && digitalRead(38) == LOW && now - lastActivityAt > 120000) {
        oled.ssd1306_command(SSD1306_DISPLAYOFF);
        sleeping = true;
    }
    if (!sleeping && now - lastFrameAt >= 40) {
        lastFrameAt = now;
        drawScreen();
    }
    delay(1);
}
