#pragma once
#define SSD1306_SWITCHCAPVCC 2
#define SSD1306_DISPLAYON 3
#define SSD1306_DISPLAYOFF 4

struct Adafruit_SSD1306 {
    template <class T> Adafruit_SSD1306(int, int, T*, int) {}

    bool begin(int, int) {
        return true;
    }

    void setCursor(int, int) {}

    template <class T> void print(T) {}

    void clearDisplay() {}

    void display() {}

    void setTextSize(int) {}

    void setFont(const void*) {}

    void setTextColor(int) {}

    void drawFastHLine(int, int, int, int) {}

    void fillRoundRect(int, int, int, int, int, int) {}

    void fillRect(int, int, int, int, int) {}

    void drawRect(int, int, int, int, int) {}

    void fillCircle(int, int, int, int) {}

    void ssd1306_command(int) {}
};
