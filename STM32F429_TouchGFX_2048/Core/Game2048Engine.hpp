#pragma once

#include <cstdint>

namespace game2048 {

constexpr uint8_t MinBoard = 3;
constexpr uint8_t MaxBoard = 6;
constexpr uint8_t DefaultBoard = 4;
constexpr uint8_t UndoDepth = 5;
constexpr uint16_t WinTile = 2048;

enum class Direction : uint8_t {
    Up,
    Down,
    Left,
    Right
};

enum class Status : uint8_t {
    Playing,
    Won,
    GameOver
};

struct Snapshot {
    uint8_t size = DefaultBoard;
    uint16_t cells[MaxBoard][MaxBoard] = {};
    uint32_t score = 0;
    Status status = Status::Playing;
};

struct MoveResult {
    bool moved = false;
    bool merged = false;
    bool won = false;
};

class Engine {
public:
    void begin(uint32_t seed, uint8_t boardSize = DefaultBoard);
    void restart(uint8_t boardSize);
    MoveResult move(Direction direction);
    bool undo();
    void restore(const Snapshot& snapshot);
    Snapshot snapshot() const;

    uint8_t size() const { return boardSize_; }
    uint16_t cell(uint8_t row, uint8_t col) const { return cells_[row][col]; }
    uint32_t score() const { return score_; }
    Status status() const { return status_; }
    bool canMove() const;

private:
    uint8_t boardSize_ = DefaultBoard;
    uint16_t cells_[MaxBoard][MaxBoard] = {};
    uint32_t score_ = 0;
    Status status_ = Status::Playing;
    Snapshot undoStack_[UndoDepth];
    uint8_t undoCount_ = 0;
    uint32_t randomState_ = 1;

    void clear();
    void addRandomTile();
    bool hasEmptyCell() const;
    bool hasWonTile() const;
    void updateStatus();
    void resetUndo();
    bool compressLine(uint16_t* line, uint8_t len, MoveResult& result);
    uint32_t nextRandom();
    uint32_t randomBelow(uint32_t limit);
    static uint8_t clampBoardSize(uint8_t boardSize);
};

} // namespace game2048
