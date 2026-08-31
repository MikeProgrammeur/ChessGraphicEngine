#pragma once

#include <array>

constexpr int BOARD_DIM = 8;
constexpr int EMPTY_SQUARE = 12;

struct Rect {
    int top = 0;
    int left = 0;
    int height = 0;
    int width = 0;
};

struct GridPos {
    int row = -1;
    int col = -1;

    bool isValid() const { return row >= 0 && row < 8 && col >= 0 && col < 8; }
    bool operator==(const GridPos& o) const { return row == o.row && col == o.col; }
    bool operator!=(const GridPos& o) const { return !(*this == o); }
};

using Board = std::array<std::array<int, BOARD_DIM>, BOARD_DIM>;
using CapturedPieces = std::array<int, 6>;
