// TouchGFX View for STM32F429I-DISC1 2048.
// Target display: onboard 240x320 LCD with onboard capacitive touch.

#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/Unicode.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <cstdlib>
#include <cmath>
#include "stm32f4xx_hal.h"

namespace {
constexpr uint8_t BtnMenu = 0;
constexpr uint8_t BtnPause = 1;
constexpr uint8_t BtnUndo = 2;
constexpr uint8_t BtnNew = 3;
constexpr uint8_t BtnSave = 4;
constexpr uint8_t BtnLoad = 5;
constexpr uint8_t BtnSizeDown = 6;
constexpr uint8_t BtnSizeUp = 7;
constexpr uint8_t BtnTheme = 8;
constexpr uint8_t BtnResume = 9;
constexpr uint8_t BtnRestart = 10;
constexpr uint8_t BtnMenuSave = 11;
constexpr uint8_t BtnMenuLoad = 12;

constexpr int16_t TopButtonX = 160;
constexpr int16_t TopButtonW = 72;
constexpr int16_t MenuPanelX = 16;
constexpr int16_t MenuPanelY = 96;
constexpr int16_t MenuPanelW = 208;
constexpr int16_t MenuPanelH = 192;
}

Screen1View::Screen1View() {
}

void Screen1View::setupScreen() {
    Screen1ViewBase::setupScreen();

    if (appstorage::FlashStorage::load(saveData_)) {
        for (uint8_t i = game2048::MinBoard; i <= game2048::MaxBoard; i++) {
            highScore_[i] = saveData_.highScore[i];
        }
        theme_ = saveData_.theme % 3;
        game_.begin(HAL_GetTick(), game2048::DefaultBoard);
        if (saveData_.hasSnapshot != 0U) {
            game_.restore(saveData_.snapshot);
        }
    } else {
        appstorage::FlashStorage::makeValid(saveData_);
        game_.begin(HAL_GetTick(), game2048::DefaultBoard);
    }

    background_.setPosition(0, 0, 240, 320);
    headerBackground_.setPosition(4, 4, 150, 72);
    boardBackground_.setPosition(BoardLeft, BoardTop, BoardPixels, BoardPixels);
    statusBackground_.setPosition(MenuPanelX, MenuPanelY, MenuPanelW, MenuPanelH);
    add(background_);
    add(headerBackground_);
    add(boardBackground_);

    titleText_.setPosition(8, 6, 90, 24);
    scoreText_.setPosition(8, 30, 146, 22);
    bestText_.setPosition(8, 52, 146, 22);
    statusText_.setPosition(28, 126, 184, 38);
    add(titleText_);
    add(scoreText_);
    add(bestText_);

    for (uint8_t r = 0; r < MaxBoard; r++) {
        for (uint8_t c = 0; c < MaxBoard; c++) {
            add(tiles_[r][c]);
            tileText_[r][c].setWildcard(tileBuffers_[r][c]);
            tileText_[r][c].setTypedText(TypedText(T_T_SINGLEUSEID1));
            tileText_[r][c].setColor(touchgfx::Color::getColorFromRGB(30, 30, 30));
            add(tileText_[r][c]);
        }
    }

    add(statusBackground_);

    for (uint8_t i = 0; i < ButtonCount; i++) {
        buttonText_[i].setWildcard(buttonBuffers_[i]);
        buttonText_[i].setTypedText(TypedText(T_T_SINGLEUSEID1));
        buttonText_[i].setColor(touchgfx::Color::getColorFromRGB(255, 255, 255));
        add(buttonBoxes_[i]);
        add(buttonText_[i]);
    }

    add(statusText_);

    titleText_.setWildcard(titleBuffer_);
    Unicode::snprintf(titleBuffer_, 16, "2048");
    titleText_.setTypedText(TypedText(T_T_SINGLEUSEID1));
    titleText_.setColor(touchgfx::Color::getColorFromRGB(45, 45, 45));

    scoreText_.setWildcard(scoreBuffer_);
    scoreText_.setTypedText(TypedText(T_T_SINGLEUSEID1));
    scoreText_.setColor(touchgfx::Color::getColorFromRGB(45, 45, 45));

    bestText_.setWildcard(bestBuffer_);
    bestText_.setTypedText(TypedText(T_T_SINGLEUSEID1));
    bestText_.setColor(touchgfx::Color::getColorFromRGB(45, 45, 45));

    statusText_.setWildcard(statusBuffer_);
    statusText_.setTypedText(TypedText(T_T_SINGLEUSEID1));
    statusText_.setColor(touchgfx::Color::getColorFromRGB(255, 255, 255));

    Unicode::snprintf(feedbackBuffer_, 32, "");
    drawAll();
}

void Screen1View::tearDownScreen() {
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleClickEvent(const ClickEvent& event) {
    Screen1ViewBase::handleClickEvent(event);

    if (event.getType() == ClickEvent::PRESSED) {
        dragActive_ = true;
        dragStartX_ = event.getX();
        dragStartY_ = event.getY();
        return;
    }

    if (event.getType() == ClickEvent::RELEASED && dragActive_) {
        onTap(event.getX(), event.getY());
        dragActive_ = false;
    }
}

void Screen1View::handleDragEvent(const DragEvent& event) {
    if (dragActive_) {
        int16_t dx = event.getNewX() - dragStartX_;
        int16_t dy = event.getNewY() - dragStartY_;

        if (std::abs(dx) > 25 || std::abs(dy) > 25) {
            if (std::abs(dx) > std::abs(dy)) {
                applyMove(dx > 0 ? game2048::Direction::Right : game2048::Direction::Left);
            } else {
                applyMove(dy > 0 ? game2048::Direction::Down : game2048::Direction::Up);
            }
            dragActive_ = false;
        }
    }
}

void Screen1View::handleTickEvent() {
    Screen1ViewBase::handleTickEvent();
    tickCounter_++;

    if (animActive_) {
        animTick_++;
        if (animTick_ >= animMaxTicks_) {
            animActive_ = false;
        }
        drawBoard();
        invalidate();
    }

    if (feedbackTicks_ > 0U) {
        feedbackTicks_--;
        if (feedbackTicks_ == 0U && !paused_ && !menuVisible_) {
            drawStatus();
            invalidate();
        }
    }
}

void Screen1View::drawAll() {
    background_.setColor(bgColor());
    headerBackground_.setColor(panelColor());
    boardBackground_.setColor(panelColor());
    drawHeader();
    drawBoard();
    drawButtons();
    drawStatus();
    invalidate();
}

void Screen1View::drawHeader() {
    Unicode::snprintf(scoreBuffer_, 24, "SCORE %u", static_cast<uint32_t>(game_.score()));
    Unicode::snprintf(bestBuffer_, 24, "BEST %u", static_cast<uint32_t>(highScore_[game_.size()]));

    uint16_t textColor = touchgfx::Color::getColorFromRGB(255, 255, 255);
    titleText_.setColor(textColor);
    scoreText_.setColor(textColor);
    bestText_.setColor(textColor);
    titleText_.invalidate();
    scoreText_.invalidate();
    bestText_.invalidate();
}

void Screen1View::drawBoard() {
    int cellPx = (BoardPixels - Gap * (game_.size() + 1)) / game_.size();

    for (uint8_t r = 0; r < MaxBoard; r++) {
        for (uint8_t c = 0; c < MaxBoard; c++) {
            bool visible = r < game_.size() && c < game_.size();
            tiles_[r][c].setVisible(visible);
            tileText_[r][c].setVisible(visible);
            if (!visible) {
                continue;
            }

            int x = BoardLeft + Gap + c * (cellPx + Gap);
            int y = BoardTop + Gap + r * (cellPx + Gap);
            uint16_t value = game_.cell(r, c);

            float scale = 1.0f;
            if (animActive_ && value > 0) {
                uint8_t type = animType_[r][c];
                if (type == 1) { // Spawn
                    float t = static_cast<float>(animTick_) / animMaxTicks_;
                    scale = 0.2f + 0.8f * t;
                } else if (type == 2) { // Merge
                    float t = static_cast<float>(animTick_) / animMaxTicks_;
                    scale = 1.0f + 0.15f * std::sin(t * 3.14159265f);
                }
            }

            int s = static_cast<int>(cellPx * scale);
            int x_anim = x + (cellPx - s) / 2;
            int y_anim = y + (cellPx - s) / 2;

            tiles_[r][c].setPosition(x_anim, y_anim, s, s);
            tiles_[r][c].setColor(tileColor(value));

            int16_t fontY = y_anim + s / 2 - 10;
            tileText_[r][c].setPosition(x_anim, fontY, s, 24);
            tileText_[r][c].setColor(value >= 8
                ? touchgfx::Color::getColorFromRGB(255, 255, 255)
                : touchgfx::Color::getColorFromRGB(60, 58, 50));

            if (value == 0) {
                Unicode::snprintf(tileBuffers_[r][c], 8, "");
            } else {
                Unicode::snprintf(tileBuffers_[r][c], 8, "%u", value);
            }
        }
    }
}

void Screen1View::drawButtons() {
    configureButton(BtnMenu, TopButtonX, 8, TopButtonW, 24, menuVisible_ ? "BACK" : "MENU", true);
    configureButton(BtnPause, TopButtonX, 38, TopButtonW, 24, paused_ ? "RESUME" : "PAUSE", !menuVisible_);

    bool showPlayButtons = !menuVisible_;
    configureButton(BtnUndo, 8, 290, 53, 26, "UNDO", showPlayButtons);
    configureButton(BtnNew, 65, 290, 53, 26, "NEW", showPlayButtons);
    configureButton(BtnSave, 122, 290, 53, 26, "SAVE", showPlayButtons);
    configureButton(BtnLoad, 179, 290, 53, 26, "LOAD", showPlayButtons);

    configureButton(BtnSizeDown, 30, 146, 82, 30, "SIZE -", menuVisible_);
    configureButton(BtnSizeUp, 128, 146, 82, 30, "SIZE +", menuVisible_);
    configureButton(BtnTheme, 30, 184, 82, 30, "THEME", menuVisible_);
    configureButton(BtnResume, 128, 184, 82, 30, "PLAY", menuVisible_);
    configureButton(BtnRestart, 30, 222, 82, 30, "RESET", menuVisible_);
    configureButton(BtnMenuSave, 128, 222, 82, 30, "SAVE", menuVisible_);
    configureButton(BtnMenuLoad, 79, 260, 82, 30, "LOAD", menuVisible_);
}

void Screen1View::drawStatus() {
    bool isMoveFeedback = 
        (feedbackBuffer_[0] == 'M' && feedbackBuffer_[1] == 'O' && feedbackBuffer_[2] == 'V' && feedbackBuffer_[3] == 'E' && feedbackBuffer_[4] == 0) ||
        (feedbackBuffer_[0] == 'M' && feedbackBuffer_[1] == 'E' && feedbackBuffer_[2] == 'R' && feedbackBuffer_[3] == 'G' && feedbackBuffer_[4] == 'E' && feedbackBuffer_[5] == 0) ||
        (feedbackBuffer_[0] == 'I' && feedbackBuffer_[1] == 'N' && feedbackBuffer_[2] == 'V' && feedbackBuffer_[3] == 'A' && feedbackBuffer_[4] == 'L' && feedbackBuffer_[5] == 'I' && feedbackBuffer_[6] == 'D' && feedbackBuffer_[7] == 0);

    bool showPanel = menuVisible_ || paused_ ||
        game_.status() == game2048::Status::Won ||
        game_.status() == game2048::Status::GameOver ||
        (feedbackTicks_ > 0U && !isMoveFeedback);

    statusBackground_.setVisible(showPanel);
    statusBackground_.setColor(panelColor());
    statusText_.setVisible(showPanel);

    if (!showPanel) {
        Unicode::snprintf(statusBuffer_, 32, "");
        return;
    }

    if (menuVisible_) {
        Unicode::snprintf(statusBuffer_, 32, "BOARD %u", static_cast<uint32_t>(game_.size()));
        statusText_.setPosition(28, 112, 184, 30);
    } else if (paused_) {
        Unicode::snprintf(statusBuffer_, 32, "PAUSED");
        statusText_.setPosition(28, 148, 184, 30);
    } else if (game_.status() == game2048::Status::Won) {
        Unicode::snprintf(statusBuffer_, 32, "YOU WIN");
        statusText_.setPosition(28, 148, 184, 30);
    } else if (game_.status() == game2048::Status::GameOver) {
        Unicode::snprintf(statusBuffer_, 32, "GAME OVER");
        statusText_.setPosition(28, 148, 184, 30);
    } else {
        Unicode::strncpy(statusBuffer_, feedbackBuffer_, 32);
        statusBuffer_[31] = 0;
        statusText_.setPosition(28, 148, 184, 30);
    }
}

void Screen1View::applyMove(game2048::Direction direction) {
    if (paused_ || menuVisible_) {
        return;
    }

    game2048::MoveResult result = game_.move(direction);
    if (!result.moved) {
        setFeedback("INVALID", 18);
        drawAll();
        return;
    }

    // Set up animations
    animActive_ = true;
    animTick_ = 0;
    for (uint8_t r = 0; r < game_.size(); r++) {
        for (uint8_t c = 0; c < game_.size(); c++) {
            if (game_.hasSpawn() && r == game_.spawnRow() && c == game_.spawnCol()) {
                animType_[r][c] = 1; // Spawn
            } else if (game_.cellMerged(r, c)) {
                animType_[r][c] = 2; // Merge
            } else {
                animType_[r][c] = 0; // None
            }
        }
    }

    if (game_.score() > highScore_[game_.size()]) {
        highScore_[game_.size()] = game_.score();
    }

    if (game_.status() == game2048::Status::Won) {
        setFeedback("WIN", 50);
        persist(false);
    } else if (game_.status() == game2048::Status::GameOver) {
        setFeedback("NO MOVE", 50);
        persist(false);
    } else if (result.merged) {
        setFeedback("MERGE", 12);
    } else {
        setFeedback("MOVE", 8);
    }

    drawAll();
}

void Screen1View::onTap(int16_t x, int16_t y) {
    animActive_ = false;
    if (hit(x, y, TopButtonX, 8, TopButtonW, 24)) {
        menuVisible_ = !menuVisible_;
        drawAll();
        return;
    }

    if (!menuVisible_ && hit(x, y, TopButtonX, 38, TopButtonW, 24)) {
        paused_ = !paused_;
        drawAll();
        return;
    }

    if (menuVisible_) {
        if (hit(x, y, 30, 146, 82, 30)) {
            changeBoard(-1);
        } else if (hit(x, y, 128, 146, 82, 30)) {
            changeBoard(1);
        } else if (hit(x, y, 30, 184, 82, 30)) {
            cycleTheme();
        } else if (hit(x, y, 128, 184, 82, 30)) {
            menuVisible_ = false;
            paused_ = false;
        } else if (hit(x, y, 30, 222, 82, 30)) {
            restartCurrentBoard();
        } else if (hit(x, y, 128, 222, 82, 30)) {
            saveGame();
        } else if (hit(x, y, 79, 260, 82, 30)) {
            loadGame();
        }
        drawAll();
        return;
    }

    if (y >= 280) {
        if (x < 62) {
            if (!game_.undo()) {
                setFeedback("NO UNDO", 20);
            }
        } else if (x < 119) {
            restartCurrentBoard();
        } else if (x < 176) {
            saveGame();
        } else {
            loadGame();
        }
    }
    drawAll();
}

void Screen1View::restartCurrentBoard() {
    game_.restart(game_.size());
    paused_ = false;
    menuVisible_ = false;
    setFeedback("NEW GAME", 25);
    persist(false);
}

void Screen1View::changeBoard(int delta) {
    int next = static_cast<int>(game_.size()) + delta;
    if (next < game2048::MinBoard) {
        next = game2048::MaxBoard;
    }
    if (next > game2048::MaxBoard) {
        next = game2048::MinBoard;
    }

    if (next != game_.size()) {
        game_.restart(static_cast<uint8_t>(next));
        paused_ = false;
        setFeedback("BOARD", 20);
        persist(false);
    }
}

void Screen1View::cycleTheme() {
    theme_ = static_cast<uint8_t>((theme_ + 1U) % 3U);
    setFeedback("THEME", 20);
    persist(false);
}

void Screen1View::saveGame() {
    if (persist(true)) {
        setFeedback("SAVED", 35);
    } else {
        setFeedback("SAVE ERR", 35);
    }
}

void Screen1View::loadGame() {
    if (appstorage::FlashStorage::load(saveData_)) {
        for (uint8_t i = game2048::MinBoard; i <= game2048::MaxBoard; i++) {
            highScore_[i] = saveData_.highScore[i];
        }
        theme_ = saveData_.theme % 3;
        if (saveData_.hasSnapshot != 0U) {
            game_.restore(saveData_.snapshot);
        }
        paused_ = false;
        menuVisible_ = false;
        setFeedback("LOADED", 35);
    } else {
        setFeedback("NO SAVE", 35);
    }
}

bool Screen1View::persist(bool includeSnapshot) {
    appstorage::FlashStorage::makeValid(saveData_);
    for (uint8_t i = game2048::MinBoard; i <= game2048::MaxBoard; i++) {
        saveData_.highScore[i] = highScore_[i];
    }
    saveData_.theme = theme_;
    saveData_.hasSnapshot = includeSnapshot ? 1U : saveData_.hasSnapshot;
    if (includeSnapshot) {
        saveData_.snapshot = game_.snapshot();
    }
    return appstorage::FlashStorage::save(saveData_);
}

void Screen1View::setFeedback(const char* text, uint8_t ticks) {
    Unicode::strncpy(feedbackBuffer_, text, 32);
    feedbackBuffer_[31] = 0;
    feedbackTicks_ = ticks;
}

void Screen1View::configureButton(uint8_t index, int16_t x, int16_t y, int16_t w, int16_t h, const char* label, bool visible) {
    if (index >= ButtonCount) {
        return;
    }

    buttonBoxes_[index].setPosition(x, y, w, h);
    buttonBoxes_[index].setColor(buttonColor());
    buttonBoxes_[index].setVisible(visible);

    int16_t textY = y + ((h >= 30) ? 6 : 2);
    buttonText_[index].setPosition(x + 2, textY, w - 4, 24);
    buttonText_[index].setVisible(visible);
    buttonText_[index].setColor(touchgfx::Color::getColorFromRGB(255, 255, 255));
    Unicode::strncpy(buttonBuffers_[index], label, 12);
    buttonBuffers_[index][11] = 0;
}

uint16_t Screen1View::bgColor() const {
    if (theme_ == 1) return touchgfx::Color::getColorFromRGB(8, 42, 56);
    if (theme_ == 2) return touchgfx::Color::getColorFromRGB(0, 0, 0);
    return touchgfx::Color::getColorFromRGB(250, 248, 239);
}

uint16_t Screen1View::panelColor() const {
    if (theme_ == 1) return touchgfx::Color::getColorFromRGB(18, 92, 110);
    if (theme_ == 2) return touchgfx::Color::getColorFromRGB(34, 36, 54);
    return touchgfx::Color::getColorFromRGB(187, 173, 160);
}

uint16_t Screen1View::tileColor(uint16_t value) const {
    if (value == 0) {
        return touchgfx::Color::getColorFromRGB(205, 193, 180);
    }

    uint8_t level = 0;
    while (value > 2) {
        value >>= 1;
        level++;
    }
    if (level > 10) level = 10;

    static const uint8_t colors[11][3] = {
        {238, 228, 218}, {237, 224, 200}, {242, 177, 121}, {245, 149, 99},
        {246, 124, 95},  {246, 94, 59},   {237, 207, 114}, {237, 204, 97},
        {237, 200, 80},  {237, 197, 63},  {60, 58, 50}
    };
    return touchgfx::Color::getColorFromRGB(colors[level][0], colors[level][1], colors[level][2]);
}

uint16_t Screen1View::buttonColor() const {
    if (theme_ == 1) return touchgfx::Color::getColorFromRGB(214, 95, 74);
    if (theme_ == 2) return touchgfx::Color::getColorFromRGB(90, 96, 140);
    return touchgfx::Color::getColorFromRGB(143, 122, 102);
}

bool Screen1View::hit(int16_t x, int16_t y, int16_t bx, int16_t by, int16_t bw, int16_t bh) const {
    return x >= bx && x < bx + bw && y >= by && y < by + bh;
}
