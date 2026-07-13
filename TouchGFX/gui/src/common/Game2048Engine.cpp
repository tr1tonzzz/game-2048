#include <gui/common/Game2048Engine.hpp>

namespace game2048 {

void Engine::begin(uint32_t seed, uint8_t boardSize) {
    randomState_ = seed == 0 ? 1 : seed;
    restart(boardSize);
}

void Engine::restart(uint8_t boardSize) {
    boardSize_ = clampBoardSize(boardSize);
    clear();
    score_ = 0;
    status_ = Status::Playing;
    resetUndo();
    hasSpawn_ = false;
    for (uint8_t r = 0; r < MaxBoard; r++) {
        for (uint8_t c = 0; c < MaxBoard; c++) {
            cellsMerged_[r][c] = false;
        }
    }
    addRandomTile();
    addRandomTile();
}

void Engine::clear() {
    for (uint8_t r = 0; r < MaxBoard; r++) {
        for (uint8_t c = 0; c < MaxBoard; c++) {
            cells_[r][c] = 0;
        }
    }
}

void Engine::resetUndo() {
    undoCount_ = 0;
}

bool Engine::undo() {
    if (undoCount_ == 0) {
        return false;
    }

    restore(undoStack_[0]);
    for (uint8_t i = 1; i < undoCount_; i++) {
        undoStack_[i - 1] = undoStack_[i];
    }
    undoCount_--;
    return true;
}

Snapshot Engine::snapshot() const {
    Snapshot snap;
    snap.size = boardSize_;
    snap.score = score_;
    snap.status = status_;
    for (uint8_t r = 0; r < MaxBoard; r++) {
        for (uint8_t c = 0; c < MaxBoard; c++) {
            snap.cells[r][c] = cells_[r][c];
        }
    }
    return snap;
}

void Engine::restore(const Snapshot& snapshot) {
    boardSize_ = clampBoardSize(snapshot.size);
    score_ = snapshot.score;
    status_ = snapshot.status;
    for (uint8_t r = 0; r < MaxBoard; r++) {
        for (uint8_t c = 0; c < MaxBoard; c++) {
            cells_[r][c] = snapshot.cells[r][c];
        }
    }
}

bool Engine::compressLine(uint16_t* line, uint8_t len, MoveResult& result, bool* mergedLine) {
    uint16_t original[MaxBoard] = {};
    uint16_t compact[MaxBoard] = {};
    uint8_t write = 0;

    for (uint8_t i = 0; i < len; i++) {
        mergedLine[i] = false;
        original[i] = line[i];
        if (line[i] != 0) {
            compact[write++] = line[i];
        }
    }

    for (uint8_t i = 0; i + 1 < len; i++) {
        if (compact[i] != 0 && compact[i] == compact[i + 1]) {
            compact[i] *= 2;
            score_ += compact[i];
            mergedLine[i] = true;
            result.merged = true;
            if (compact[i] >= WinTile) {
                result.won = true;
            }
            for (uint8_t j = i + 1; j + 1 < len; j++) {
                compact[j] = compact[j + 1];
            }
            compact[len - 1] = 0;
        }
    }

    bool changed = false;
    for (uint8_t i = 0; i < len; i++) {
        line[i] = compact[i];
        if (line[i] != original[i]) {
            changed = true;
        }
    }
    return changed;
}

MoveResult Engine::move(Direction direction) {
    MoveResult result;
    if (status_ != Status::Playing) {
        return result;
    }

    Snapshot before = snapshot();
    uint16_t line[MaxBoard] = {};
    bool mergedLine[MaxBoard] = {};

    for (uint8_t r = 0; r < MaxBoard; r++) {
        for (uint8_t c = 0; c < MaxBoard; c++) {
            cellsMerged_[r][c] = false;
        }
    }
    hasSpawn_ = false;

    for (uint8_t index = 0; index < boardSize_; index++) {
        for (uint8_t i = 0; i < boardSize_; i++) {
            switch (direction) {
                case Direction::Left: line[i] = cells_[index][i]; break;
                case Direction::Right: line[i] = cells_[index][boardSize_ - 1 - i]; break;
                case Direction::Up: line[i] = cells_[i][index]; break;
                case Direction::Down: line[i] = cells_[boardSize_ - 1 - i][index]; break;
            }
        }

        result.moved = compressLine(line, boardSize_, result, mergedLine) || result.moved;

        for (uint8_t i = 0; i < boardSize_; i++) {
            switch (direction) {
                case Direction::Left:
                    cells_[index][i] = line[i];
                    cellsMerged_[index][i] = mergedLine[i];
                    break;
                case Direction::Right:
                    cells_[index][boardSize_ - 1 - i] = line[i];
                    cellsMerged_[index][boardSize_ - 1 - i] = mergedLine[i];
                    break;
                case Direction::Up:
                    cells_[i][index] = line[i];
                    cellsMerged_[i][index] = mergedLine[i];
                    break;
                case Direction::Down:
                    cells_[boardSize_ - 1 - i][index] = line[i];
                    cellsMerged_[boardSize_ - 1 - i][index] = mergedLine[i];
                    break;
            }
        }
    }

    if (result.moved) {
        for (int i = UndoDepth - 1; i > 0; i--) {
            undoStack_[i] = undoStack_[i - 1];
        }
        undoStack_[0] = before;
        if (undoCount_ < UndoDepth) {
            undoCount_++;
        }
        addRandomTile();
        updateStatus();
        result.won = result.won || status_ == Status::Won;
    }
    return result;
}

bool Engine::hasEmptyCell() const {
    for (uint8_t r = 0; r < boardSize_; r++) {
        for (uint8_t c = 0; c < boardSize_; c++) {
            if (cells_[r][c] == 0) {
                return true;
            }
        }
    }
    return false;
}

void Engine::addRandomTile() {
    uint8_t empty = 0;
    for (uint8_t r = 0; r < boardSize_; r++) {
        for (uint8_t c = 0; c < boardSize_; c++) {
            if (cells_[r][c] == 0) {
                empty++;
            }
        }
    }
    if (empty == 0) {
        hasSpawn_ = false;
        return;
    }

    uint8_t target = static_cast<uint8_t>(randomBelow(empty));
    for (uint8_t r = 0; r < boardSize_; r++) {
        for (uint8_t c = 0; c < boardSize_; c++) {
            if (cells_[r][c] == 0) {
                if (target == 0) {
                    cells_[r][c] = randomBelow(10) == 0 ? 4 : 2;
                    spawnRow_ = r;
                    spawnCol_ = c;
                    hasSpawn_ = true;
                    return;
                }
                target--;
            }
        }
    }
}

bool Engine::hasWonTile() const {
    for (uint8_t r = 0; r < boardSize_; r++) {
        for (uint8_t c = 0; c < boardSize_; c++) {
            if (cells_[r][c] >= WinTile) {
                return true;
            }
        }
    }
    return false;
}

bool Engine::canMove() const {
    if (hasEmptyCell()) {
        return true;
    }

    for (uint8_t r = 0; r < boardSize_; r++) {
        for (uint8_t c = 0; c < boardSize_; c++) {
            if (r + 1 < boardSize_ && cells_[r][c] == cells_[r + 1][c]) {
                return true;
            }
            if (c + 1 < boardSize_ && cells_[r][c] == cells_[r][c + 1]) {
                return true;
            }
        }
    }
    return false;
}

void Engine::updateStatus() {
    if (hasWonTile()) {
        status_ = Status::Won;
    } else if (!canMove()) {
        status_ = Status::GameOver;
    } else {
        status_ = Status::Playing;
    }
}

uint32_t Engine::nextRandom() {
    randomState_ = 1664525UL * randomState_ + 1013904223UL;
    return randomState_;
}

uint32_t Engine::randomBelow(uint32_t limit) {
    return limit == 0 ? 0 : nextRandom() % limit;
}

uint8_t Engine::clampBoardSize(uint8_t boardSize) {
    if (boardSize < MinBoard) return MinBoard;
    if (boardSize > MaxBoard) return MaxBoard;
    return boardSize;
}

} // namespace game2048
