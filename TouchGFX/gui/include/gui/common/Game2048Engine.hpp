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

#pragma pack(push, 1)
struct Snapshot {
    uint32_t score = 0;
    uint16_t cells[MaxBoard][MaxBoard] = {};
    uint8_t size = DefaultBoard;
    Status status = Status::Playing;
    uint16_t padding = 0;
};
#pragma pack(pop)

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
    bool cellMerged(uint8_t row, uint8_t col) const { return cellsMerged_[row][col]; }
    uint8_t spawnRow() const { return spawnRow_; }
    uint8_t spawnCol() const { return spawnCol_; }
    bool hasSpawn() const { return hasSpawn_; }

private:
    uint8_t boardSize_ = DefaultBoard;
    uint16_t cells_[MaxBoard][MaxBoard] = {};
    uint32_t score_ = 0;
    Status status_ = Status::Playing;
    Snapshot undoStack_[UndoDepth];
    uint8_t undoCount_ = 0;
    uint32_t randomState_ = 1;
    bool cellsMerged_[MaxBoard][MaxBoard] = {};
    uint8_t spawnRow_ = 0;
    uint8_t spawnCol_ = 0;
    bool hasSpawn_ = false;

    void clear();
    void addRandomTile();
    bool hasEmptyCell() const;
    bool hasWonTile() const;
    void updateStatus();
    void resetUndo();
    bool compressLine(uint16_t* line, uint8_t len, MoveResult& result, bool* mergedLine);
    uint32_t nextRandom();
    uint32_t randomBelow(uint32_t limit);
    static uint8_t clampBoardSize(uint8_t boardSize);
};

} // namespace game2048
