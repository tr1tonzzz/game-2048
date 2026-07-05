#include <Arduino.h>
#include <esp_system.h>
#include <math.h>
#include <Preferences.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "Game2048.h"
#include "config.h"

enum class ScreenMode : uint8_t {
  Game,
  Menu,
  Paused
};

enum class Theme : uint8_t {
  Classic,
  Ocean,
  Neon
};

struct TouchPoint {
  int16_t x = 0;
  int16_t y = 0;
};

struct TouchState {
  bool pressed = false;
  uint8_t count = 0;
  TouchPoint points[2];
};

class Buzzer {
public:
  void begin() {
    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);
    offAt_ = 0;
  }

  void tick() {
    if (offAt_ && millis() >= offAt_) {
      noTone(BUZZER_PIN);
      offAt_ = 0;
    }
  }

  void beep(uint16_t freq, uint16_t ms) {
    tone(BUZZER_PIN, freq);
    offAt_ = millis() + ms;
  }

  void move() { beep(1200, 25); }
  void merge() { beep(1700, 35); }
  void win() { beep(2300, 160); }
  void fail() { beep(220, 180); }

private:
  uint32_t offAt_ = 0;
};

class Storage {
public:
  void begin() {
    prefs_.begin("game2048", false);
  }

  void saveGame(const GameSnapshot &snap, Theme theme) {
    SaveBlob blob;
    blob.magic = SAVE_MAGIC;
    blob.snapshot = snap;
    blob.theme = static_cast<uint8_t>(theme);
    prefs_.putBytes("save", &blob, sizeof(blob));
  }

  bool loadGame(GameSnapshot &snap, Theme &theme) {
    SaveBlob blob;
    if (prefs_.getBytesLength("save") != sizeof(blob)) {
      return false;
    }
    prefs_.getBytes("save", &blob, sizeof(blob));
    if (blob.magic != SAVE_MAGIC || blob.snapshot.size < MIN_BOARD || blob.snapshot.size > MAX_BOARD) {
      return false;
    }
    snap = blob.snapshot;
    theme = static_cast<Theme>(constrain((int)blob.theme, 0, 2));
    return true;
  }

  uint32_t highScore(uint8_t boardSize) {
    char key[12];
    snprintf(key, sizeof(key), "hi%u", boardSize);
    return prefs_.getUInt(key, 0);
  }

  void updateHighScore(uint8_t boardSize, uint32_t score) {
    if (score <= highScore(boardSize)) {
      return;
    }
    char key[12];
    snprintf(key, sizeof(key), "hi%u", boardSize);
    prefs_.putUInt(key, score);
  }

private:
  struct SaveBlob {
    uint32_t magic = SAVE_MAGIC;
    GameSnapshot snapshot;
    uint8_t theme = 0;
  };

  Preferences prefs_;
};

class InputManager {
public:
  void begin(XPT2046_Touchscreen *touch) {
    touch_ = touch;
    pinMode(JOY_SW_PIN, INPUT_PULLUP);
    if (BTN_UP_PIN >= 0) pinMode(BTN_UP_PIN, INPUT_PULLUP);
    if (BTN_DOWN_PIN >= 0) pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    if (BTN_LEFT_PIN >= 0) pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
    if (BTN_RIGHT_PIN >= 0) pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);
  }

  bool getDirection(Direction &dir) {
    if (readSwipe(dir)) {
      return true;
    }
    if (readButtons(dir)) {
      return true;
    }
    return readJoystick(dir);
  }

  TouchState readTouch() {
    TouchState state;
    if (!touch_ || !touch_->touched()) {
      return state;
    }

    TS_Point p = touch_->getPoint();
    state.pressed = true;
    state.count = 1;
    state.points[0].x = map(constrain(p.x, TOUCH_MIN_X, TOUCH_MAX_X), TOUCH_MIN_X, TOUCH_MAX_X, 0, SCREEN_W);
    state.points[0].y = map(constrain(p.y, TOUCH_MIN_Y, TOUCH_MAX_Y), TOUCH_MIN_Y, TOUCH_MAX_Y, 0, SCREEN_H);
    return state;
  }

  bool joyPressed() {
    bool down = digitalRead(JOY_SW_PIN) == LOW;
    bool edge = down && !lastJoyButton_;
    lastJoyButton_ = down;
    return edge;
  }

  bool consumeTap(int16_t &x, int16_t &y) {
    if (!tapReady_) {
      return false;
    }
    tapReady_ = false;
    x = tapX_;
    y = tapY_;
    return true;
  }

  bool consumeTwoFingerSwipe(Direction &dir) {
    if (!twoFingerReady_) {
      return false;
    }
    twoFingerReady_ = false;
    dir = twoFingerDir_;
    return true;
  }

  int8_t consumePinch() {
    int8_t value = pinchDelta_;
    pinchDelta_ = 0;
    return value;
  }

  void pollGestures() {
    TouchState state = readTouch();
    handleTouchLifecycle(state);
    handleMultiTouch(state);
  }

private:
  XPT2046_Touchscreen *touch_ = nullptr;
  bool touchActive_ = false;
  TouchPoint touchStart_;
  TouchPoint touchLast_;
  uint32_t touchStartMs_ = 0;
  bool lastJoyButton_ = false;
  uint32_t lastJoyMoveMs_ = 0;
  bool tapReady_ = false;
  int16_t tapX_ = 0;
  int16_t tapY_ = 0;
  bool twoFingerReady_ = false;
  Direction twoFingerDir_ = Direction::Up;
  int16_t initialPinchDistance_ = -1;
  int8_t pinchDelta_ = 0;

  bool readSwipe(Direction &dir) {
    pollGestures();
    int16_t dx = touchLast_.x - touchStart_.x;
    int16_t dy = touchLast_.y - touchStart_.y;
    if (touchActive_ || abs(dx) < 45 && abs(dy) < 45) {
      return false;
    }

    touchStart_ = touchLast_;
    if (abs(dx) > abs(dy)) {
      dir = dx > 0 ? Direction::Right : Direction::Left;
    } else {
      dir = dy > 0 ? Direction::Down : Direction::Up;
    }
    return true;
  }

  bool readJoystick(Direction &dir) {
    uint32_t now = millis();
    if (now - lastJoyMoveMs_ < JOY_REPEAT_MS) {
      return false;
    }

    int x = analogRead(JOY_X_PIN) - JOY_CENTER;
    int y = analogRead(JOY_Y_PIN) - JOY_CENTER;
    if (abs(x) < JOY_DEADZONE && abs(y) < JOY_DEADZONE) {
      return false;
    }

    if (abs(x) > abs(y)) {
      dir = x > 0 ? Direction::Right : Direction::Left;
    } else {
      dir = y > 0 ? Direction::Down : Direction::Up;
    }
    lastJoyMoveMs_ = now;
    return true;
  }

  bool readButtons(Direction &dir) {
    uint32_t now = millis();
    if (now - lastJoyMoveMs_ < JOY_REPEAT_MS) {
      return false;
    }

    if (BTN_UP_PIN >= 0 && digitalRead(BTN_UP_PIN) == LOW) {
      dir = Direction::Up;
    } else if (BTN_DOWN_PIN >= 0 && digitalRead(BTN_DOWN_PIN) == LOW) {
      dir = Direction::Down;
    } else if (BTN_LEFT_PIN >= 0 && digitalRead(BTN_LEFT_PIN) == LOW) {
      dir = Direction::Left;
    } else if (BTN_RIGHT_PIN >= 0 && digitalRead(BTN_RIGHT_PIN) == LOW) {
      dir = Direction::Right;
    } else {
      return false;
    }

    lastJoyMoveMs_ = now;
    return true;
  }

  void handleTouchLifecycle(const TouchState &state) {
    if (state.pressed && state.count == 1) {
      TouchPoint p = state.points[0];
      if (!touchActive_) {
        touchActive_ = true;
        touchStart_ = p;
        touchStartMs_ = millis();
      }
      touchLast_ = p;
      return;
    }

    if (touchActive_) {
      int16_t dx = touchLast_.x - touchStart_.x;
      int16_t dy = touchLast_.y - touchStart_.y;
      if (abs(dx) < 18 && abs(dy) < 18 && millis() - touchStartMs_ < 450) {
        tapReady_ = true;
        tapX_ = touchLast_.x;
        tapY_ = touchLast_.y;
      }
    }
    touchActive_ = false;
  }

  void handleMultiTouch(const TouchState &state) {
    if (state.count < 2) {
      initialPinchDistance_ = -1;
      return;
    }

    int16_t dx = state.points[1].x - state.points[0].x;
    int16_t dy = state.points[1].y - state.points[0].y;
    int16_t distance = sqrt(dx * dx + dy * dy);
    if (initialPinchDistance_ < 0) {
      initialPinchDistance_ = distance;
      return;
    }

    int16_t delta = distance - initialPinchDistance_;
    if (abs(delta) > 38) {
      pinchDelta_ = delta > 0 ? 1 : -1;
      initialPinchDistance_ = distance;
    }

    if (abs(dx) > 70 || abs(dy) > 70) {
      twoFingerDir_ = abs(dx) > abs(dy)
        ? (dx > 0 ? Direction::Right : Direction::Left)
        : (dy > 0 ? Direction::Down : Direction::Up);
      twoFingerReady_ = true;
    }
  }
};

class Renderer {
public:
  void begin(TFT_eSPI *tft) {
    tft_ = tft;
    tft_->init();
    tft_->setRotation(0);
    tft_->fillScreen(TFT_BLACK);
  }

  void draw(const Game2048 &game, ScreenMode mode, Theme theme, uint32_t highScore) {
    if (mode == ScreenMode::Menu) {
      drawMenu(game, theme, highScore);
      return;
    }
    drawGame(game, mode, theme, highScore);
  }

  void animateMove(const Game2048 &game, Theme theme, uint32_t highScore) {
    for (uint8_t i = 0; i < 3; i++) {
      drawGame(game, ScreenMode::Game, theme, highScore, i);
      delay(24);
    }
  }

private:
  TFT_eSPI *tft_ = nullptr;

  uint16_t bg(Theme theme) const {
    switch (theme) {
      case Theme::Ocean: return tft_->color565(8, 42, 56);
      case Theme::Neon: return TFT_BLACK;
      default: return tft_->color565(250, 248, 239);
    }
  }

  uint16_t panel(Theme theme) const {
    switch (theme) {
      case Theme::Ocean: return tft_->color565(18, 92, 110);
      case Theme::Neon: return tft_->color565(34, 36, 54);
      default: return tft_->color565(187, 173, 160);
    }
  }

  uint16_t textColor(Theme theme) const {
    return theme == Theme::Classic ? tft_->color565(90, 80, 70) : TFT_WHITE;
  }

  uint16_t tileColor(uint16_t value, Theme theme) const {
    if (value == 0) {
      return theme == Theme::Classic ? tft_->color565(205, 193, 180) : tft_->color565(42, 47, 66);
    }
    uint8_t level = 0;
    uint16_t v = value;
    while (v > 2) {
      v >>= 1;
      level++;
    }
    const uint16_t classic[] = {
      0xEF5B, 0xEED7, 0xF58B, 0xF4A7, 0xF3E3, 0xF31F, 0xEE88, 0xEE66, 0xEE44, 0xEE22, 0xD5BA
    };
    const uint16_t ocean[] = {
      0x96DF, 0x75BD, 0x4D3A, 0x2C97, 0x1394, 0x03B1, 0xFDCB, 0xFD05, 0xFBA0, 0xB1E5, 0xFFFF
    };
    const uint16_t neon[] = {
      0x7DFF, 0x57FF, 0x07FF, 0x07F0, 0xFFE0, 0xFDA0, 0xF81F, 0xC01F, 0x801F, 0xFFFF, 0xF800
    };
    if (level > 10) {
      level = 10;
    }
    if (theme == Theme::Ocean) return ocean[level];
    if (theme == Theme::Neon) return neon[level];
    return classic[level];
  }

  void button(int x, int y, int w, int h, const char *label, Theme theme) {
    tft_->fillRoundRect(x, y, w, h, 5, panel(theme));
    tft_->drawRoundRect(x, y, w, h, 5, textColor(theme));
    tft_->setTextColor(TFT_WHITE, panel(theme));
    tft_->setTextDatum(MC_DATUM);
    tft_->drawString(label, x + w / 2, y + h / 2, 2);
  }

  void drawHeader(const Game2048 &game, Theme theme, uint32_t highScore) {
    tft_->setTextColor(textColor(theme), bg(theme));
    tft_->setTextDatum(TL_DATUM);
    tft_->drawString("2048 ESP32", 8, 6, 2);
    tft_->drawString("Score", 8, 28, 2);
    tft_->drawNumber(game.score(), 62, 28, 2);
    tft_->drawString("Best", 8, 48, 2);
    tft_->drawNumber(highScore, 62, 48, 2);
    button(150, 8, 82, 24, "MENU", theme);
    button(150, 38, 82, 24, "PAUSE", theme);
  }

  void drawGame(const Game2048 &game, ScreenMode mode, Theme theme, uint32_t highScore, uint8_t anim = 0) {
    tft_->fillScreen(bg(theme));
    drawHeader(game, theme, highScore);

    int boardPx = 224;
    int top = 78;
    int left = 8;
    int gap = 4;
    int cellPx = (boardPx - gap * (game.size() + 1)) / game.size();

    tft_->fillRoundRect(left, top, boardPx, boardPx, 6, panel(theme));
    for (uint8_t r = 0; r < game.size(); r++) {
      for (uint8_t c = 0; c < game.size(); c++) {
        uint16_t value = game.cell(r, c);
        int x = left + gap + c * (cellPx + gap);
        int y = top + gap + r * (cellPx + gap);
        int inset = value && anim < 2 ? 2 - anim : 0;
        tft_->fillRoundRect(x + inset, y + inset, cellPx - 2 * inset, cellPx - 2 * inset, 5, tileColor(value, theme));
        if (value) {
          tft_->setTextDatum(MC_DATUM);
          tft_->setTextColor(value <= 4 && theme == Theme::Classic ? textColor(theme) : TFT_WHITE, tileColor(value, theme));
          uint8_t font = game.size() <= 4 ? 4 : 2;
          tft_->drawNumber(value, x + cellPx / 2, y + cellPx / 2, font);
        }
      }
    }

    button(8, 306, 50, 14, "UNDO", theme);
    button(66, 306, 50, 14, "NEW", theme);
    button(124, 306, 50, 14, "SAVE", theme);
    button(182, 306, 50, 14, "LOAD", theme);

    if (mode == ScreenMode::Paused || game.status() != GameStatus::Playing) {
      tft_->fillRoundRect(28, 128, 184, 72, 7, tft_->color565(40, 40, 45));
      tft_->drawRoundRect(28, 128, 184, 72, 7, TFT_WHITE);
      tft_->setTextDatum(MC_DATUM);
      tft_->setTextColor(TFT_WHITE, tft_->color565(40, 40, 45));
      const char *msg = mode == ScreenMode::Paused ? "PAUSED" : (game.status() == GameStatus::Won ? "YOU WIN!" : "GAME OVER");
      tft_->drawString(msg, 120, 152, 4);
      tft_->drawString("Tap NEW or MENU", 120, 182, 2);
    }
  }

  void drawMenu(const Game2048 &game, Theme theme, uint32_t highScore) {
    tft_->fillScreen(bg(theme));
    tft_->setTextColor(textColor(theme), bg(theme));
    tft_->setTextDatum(MC_DATUM);
    tft_->drawString("2048 MENU", 120, 24, 4);
    tft_->drawString(("Board " + String(game.size()) + "x" + String(game.size())).c_str(), 120, 58, 2);
    tft_->drawString(("Best " + String(highScore)).c_str(), 120, 78, 2);
    button(30, 106, 82, 30, "SIZE -", theme);
    button(128, 106, 82, 30, "SIZE +", theme);
    button(30, 148, 82, 30, "THEME", theme);
    button(128, 148, 82, 30, "RESUME", theme);
    button(30, 190, 82, 30, "SAVE", theme);
    button(128, 190, 82, 30, "LOAD", theme);
    button(30, 232, 82, 30, "RESTART", theme);
    button(128, 232, 82, 30, "BACK", theme);
  }
};

TFT_eSPI tft;
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
Game2048 game;
Renderer renderer;
InputManager input;
Buzzer buzzer;
Storage storage;

ScreenMode screenMode = ScreenMode::Game;
Theme theme = Theme::Classic;
bool dirty = true;
uint32_t lastAutoSaveMs = 0;

bool hit(int16_t x, int16_t y, int bx, int by, int bw, int bh) {
  return x >= bx && x < bx + bw && y >= by && y < by + bh;
}

void cycleTheme() {
  theme = static_cast<Theme>((static_cast<uint8_t>(theme) + 1) % 3);
  dirty = true;
}

void changeBoard(int delta) {
  int next = constrain((int)game.size() + delta, (int)MIN_BOARD, (int)MAX_BOARD);
  game.restart(next);
  dirty = true;
}

void saveNow() {
  storage.saveGame(game.snapshot(), theme);
  storage.updateHighScore(game.size(), game.score());
  buzzer.beep(1500, 40);
}

void loadNow() {
  GameSnapshot snap;
  Theme savedTheme;
  if (storage.loadGame(snap, savedTheme)) {
    game.restore(snap);
    theme = savedTheme;
    buzzer.beep(1300, 40);
  } else {
    buzzer.fail();
  }
  dirty = true;
}

void handleTap(int16_t x, int16_t y) {
  if (screenMode == ScreenMode::Menu) {
    if (hit(x, y, 30, 106, 82, 30)) changeBoard(-1);
    else if (hit(x, y, 128, 106, 82, 30)) changeBoard(1);
    else if (hit(x, y, 30, 148, 82, 30)) cycleTheme();
    else if (hit(x, y, 128, 148, 82, 30)) screenMode = ScreenMode::Game;
    else if (hit(x, y, 30, 190, 82, 30)) saveNow();
    else if (hit(x, y, 128, 190, 82, 30)) loadNow();
    else if (hit(x, y, 30, 232, 82, 30)) game.restart(game.size());
    else if (hit(x, y, 128, 232, 82, 30)) screenMode = ScreenMode::Game;
    dirty = true;
    return;
  }

  if (hit(x, y, 150, 8, 82, 24)) screenMode = ScreenMode::Menu;
  else if (hit(x, y, 150, 38, 82, 24)) screenMode = screenMode == ScreenMode::Paused ? ScreenMode::Game : ScreenMode::Paused;
  else if (hit(x, y, 8, 306, 50, 14)) {
    if (!game.undo()) buzzer.fail();
  } else if (hit(x, y, 66, 306, 50, 14)) game.restart(game.size());
  else if (hit(x, y, 124, 306, 50, 14)) saveNow();
  else if (hit(x, y, 182, 306, 50, 14)) loadNow();
  dirty = true;
}

void applyMove(Direction dir) {
  if (screenMode != ScreenMode::Game) {
    return;
  }

  MoveResult result = game.move(dir);
  if (!result.moved) {
    buzzer.fail();
    return;
  }

  storage.updateHighScore(game.size(), game.score());
  renderer.animateMove(game, theme, storage.highScore(game.size()));
  if (result.won) buzzer.win();
  else if (result.merged) buzzer.merge();
  else buzzer.move();
  dirty = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(JOY_X_PIN, INPUT);
  pinMode(JOY_Y_PIN, INPUT);
  storage.begin();
  buzzer.begin();
  SPI.begin();
  touch.begin();
  touch.setRotation(0);
  renderer.begin(&tft);
  input.begin(&touch);

  GameSnapshot snap;
  Theme savedTheme;
  if (storage.loadGame(snap, savedTheme)) {
    game.restore(snap);
    theme = savedTheme;
  } else {
    game.begin(DEFAULT_BOARD);
  }
}

void loop() {
  buzzer.tick();

  Direction dir;
  if (input.getDirection(dir)) {
    applyMove(dir);
  }

  Direction twoFingerDir;
  if (input.consumeTwoFingerSwipe(twoFingerDir)) {
    applyMove(twoFingerDir);
  }

  int8_t pinch = input.consumePinch();
  if (pinch != 0) {
    changeBoard(pinch > 0 ? 1 : -1);
  }

  if (input.joyPressed()) {
    screenMode = screenMode == ScreenMode::Paused ? ScreenMode::Game : ScreenMode::Paused;
    dirty = true;
  }

  int16_t x, y;
  if (input.consumeTap(x, y)) {
    handleTap(x, y);
  }

  if (millis() - lastAutoSaveMs > 15000) {
    storage.saveGame(game.snapshot(), theme);
    lastAutoSaveMs = millis();
  }

  if (dirty) {
    renderer.draw(game, screenMode, theme, storage.highScore(game.size()));
    dirty = false;
  }

  delay(8);
}
