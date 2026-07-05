#include "Game2048.h"
#include <esp_system.h>

void Game2048::begin(uint8_t boardSize) {
  randomSeed((uint32_t)esp_random());
  restart(boardSize);
}

void Game2048::restart(uint8_t boardSize) {
  boardSize_ = constrain((int)boardSize, (int)MIN_BOARD, (int)MAX_BOARD);
  clear();
  score_ = 0;
  status_ = GameStatus::Playing;
  resetUndo();
  addRandomTile();
  addRandomTile();
}

void Game2048::clear() {
  for (uint8_t r = 0; r < MAX_BOARD; r++) {
    for (uint8_t c = 0; c < MAX_BOARD; c++) {
      cells_[r][c] = 0;
    }
  }
}

void Game2048::resetUndo() {
  undoCount_ = 0;
}

bool Game2048::undo() {
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

GameSnapshot Game2048::snapshot() const {
  GameSnapshot snap;
  snap.size = boardSize_;
  snap.score = score_;
  snap.status = status_;
  for (uint8_t r = 0; r < MAX_BOARD; r++) {
    for (uint8_t c = 0; c < MAX_BOARD; c++) {
      snap.cells[r][c] = cells_[r][c];
    }
  }
  return snap;
}

void Game2048::restore(const GameSnapshot &snapshot) {
  boardSize_ = constrain((int)snapshot.size, (int)MIN_BOARD, (int)MAX_BOARD);
  score_ = snapshot.score;
  status_ = snapshot.status;
  for (uint8_t r = 0; r < MAX_BOARD; r++) {
    for (uint8_t c = 0; c < MAX_BOARD; c++) {
      cells_[r][c] = snapshot.cells[r][c];
    }
  }
}

bool Game2048::compressLine(uint16_t *line, uint8_t len, MoveResult &result) {
  uint16_t original[MAX_BOARD] = {};
  uint16_t compact[MAX_BOARD] = {};
  uint8_t write = 0;

  for (uint8_t i = 0; i < len; i++) {
    original[i] = line[i];
    if (line[i] != 0) {
      compact[write++] = line[i];
    }
  }

  for (uint8_t i = 0; i + 1 < len; i++) {
    if (compact[i] != 0 && compact[i] == compact[i + 1]) {
      compact[i] *= 2;
      score_ += compact[i];
      result.merged = true;
      if (compact[i] >= WIN_TILE) {
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

MoveResult Game2048::move(Direction dir) {
  MoveResult result;
  if (status_ != GameStatus::Playing) {
    return result;
  }

  GameSnapshot before = snapshot();
  uint16_t line[MAX_BOARD] = {};

  for (uint8_t index = 0; index < boardSize_; index++) {
    for (uint8_t i = 0; i < boardSize_; i++) {
      switch (dir) {
        case Direction::Left: line[i] = cells_[index][i]; break;
        case Direction::Right: line[i] = cells_[index][boardSize_ - 1 - i]; break;
        case Direction::Up: line[i] = cells_[i][index]; break;
        case Direction::Down: line[i] = cells_[boardSize_ - 1 - i][index]; break;
      }
    }

    bool changed = compressLine(line, boardSize_, result);
    result.moved = result.moved || changed;

    for (uint8_t i = 0; i < boardSize_; i++) {
      switch (dir) {
        case Direction::Left: cells_[index][i] = line[i]; break;
        case Direction::Right: cells_[index][boardSize_ - 1 - i] = line[i]; break;
        case Direction::Up: cells_[i][index] = line[i]; break;
        case Direction::Down: cells_[boardSize_ - 1 - i][index] = line[i]; break;
      }
    }
  }

  if (result.moved) {
    for (int i = UNDO_DEPTH - 1; i > 0; i--) {
      undoStack_[i] = undoStack_[i - 1];
    }
    undoStack_[0] = before;
    if (undoCount_ < UNDO_DEPTH) {
      undoCount_++;
    }
    addRandomTile();
    updateStatus();
    result.won = result.won || status_ == GameStatus::Won;
  }
  return result;
}

bool Game2048::hasEmptyCell() const {
  for (uint8_t r = 0; r < boardSize_; r++) {
    for (uint8_t c = 0; c < boardSize_; c++) {
      if (cells_[r][c] == 0) {
        return true;
      }
    }
  }
  return false;
}

void Game2048::addRandomTile() {
  uint8_t empty = 0;
  for (uint8_t r = 0; r < boardSize_; r++) {
    for (uint8_t c = 0; c < boardSize_; c++) {
      if (cells_[r][c] == 0) {
        empty++;
      }
    }
  }
  if (empty == 0) {
    return;
  }

  uint8_t target = random(empty);
  for (uint8_t r = 0; r < boardSize_; r++) {
    for (uint8_t c = 0; c < boardSize_; c++) {
      if (cells_[r][c] == 0) {
        if (target == 0) {
          cells_[r][c] = random(10) == 0 ? 4 : 2;
          return;
        }
        target--;
      }
    }
  }
}

bool Game2048::hasWonTile() const {
  for (uint8_t r = 0; r < boardSize_; r++) {
    for (uint8_t c = 0; c < boardSize_; c++) {
      if (cells_[r][c] >= WIN_TILE) {
        return true;
      }
    }
  }
  return false;
}

bool Game2048::canMove() const {
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

void Game2048::updateStatus() {
  if (hasWonTile()) {
    status_ = GameStatus::Won;
  } else if (!canMove()) {
    status_ = GameStatus::GameOver;
  } else {
    status_ = GameStatus::Playing;
  }
}
