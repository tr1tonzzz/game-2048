#pragma once

#include <Arduino.h>

constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 320;

constexpr int JOY_X_PIN = 34;
constexpr int JOY_Y_PIN = 35;
constexpr int JOY_SW_PIN = 32;
constexpr int BUZZER_PIN = 25;

// Set these GPIOs to real pins if you want 4 direction buttons instead of joystick.
// Use INPUT_PULLUP wiring: button pressed = GPIO connected to GND.
constexpr int BTN_UP_PIN = -1;
constexpr int BTN_DOWN_PIN = -1;
constexpr int BTN_LEFT_PIN = -1;
constexpr int BTN_RIGHT_PIN = -1;

constexpr int JOY_CENTER = 2048;
constexpr int JOY_DEADZONE = 950;
constexpr uint32_t JOY_REPEAT_MS = 260;

constexpr uint8_t MIN_BOARD = 3;
constexpr uint8_t MAX_BOARD = 6;
constexpr uint8_t DEFAULT_BOARD = 4;
constexpr uint8_t UNDO_DEPTH = 5;

constexpr int WIN_TILE = 2048;

constexpr uint16_t TOUCH_MIN_X = 260;
constexpr uint16_t TOUCH_MAX_X = 3860;
constexpr uint16_t TOUCH_MIN_Y = 320;
constexpr uint16_t TOUCH_MAX_Y = 3800;

constexpr uint32_t SAVE_MAGIC = 0x20482026;
