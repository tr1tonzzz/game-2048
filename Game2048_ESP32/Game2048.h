#pragma once

#include <Arduino.h>
#include "config.h"

enum class Direction : uint8_t {
  Up,
  Down,
  Left,
  Right
};

enum class GameStatus : uint8_t {
  Playing,
  Won,
  GameOver
};

struct GameSnapshot {
  uint8_t size = DEFAULT_BOARD;
  uint16_t cells[MAX_BOARD][MAX_BOARD] = {};
  uint32_t score = 0;
  GameStatus status = GameStatus::Playing;
};

struct MoveResult {
  bool moved = false;
  bool merged = false;
  bool won = false;
};

class Game2048 {
public:
  void begin(uint8_t boardSize = DEFAULT_BOARD);
  void restart(uint8_t boardSize);
  MoveResult move(Direction dir);
  bool undo();
  void restore(const GameSnapshot &snapshot);
  GameSnapshot snapshot() const;

  uint8_t size() const { return boardSize_; }
  uint16_t cell(uint8_t row, uint8_t col) const { return cells_[row][col]; }
  uint32_t score() const { return score_; }
  GameStatus status() const { return status_; }
  bool canMove() const;

private:
  uint8_t boardSize_ = DEFAULT_BOARD;
  uint16_t cells_[MAX_BOARD][MAX_BOARD] = {};
  uint32_t score_ = 0;
  GameStatus status_ = GameStatus::Playing;
  GameSnapshot undoStack_[UNDO_DEPTH];
  uint8_t undoCount_ = 0;

  void clear();
  void addRandomTile();
  bool hasEmptyCell() const;
  bool hasWonTile() const;
  void updateStatus();
  void resetUndo();
  bool compressLine(uint16_t *line, uint8_t len, MoveResult &result);
};
