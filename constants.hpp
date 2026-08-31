#pragma once

#include <SFML/Graphics.hpp>

constexpr int WINDOW_WIDTH = 1280;
constexpr int BOARD_AREA_HEIGHT = 720;
constexpr int PV_ZONE_HEIGHT = 80;
constexpr int WINDOW_HEIGHT = BOARD_AREA_HEIGHT + PV_ZONE_HEIGHT;
constexpr int PIECE_DIM = 16;
constexpr int MAX_FPS = 30;

constexpr int CHESS_TEXTURE_COUNT = 12;
constexpr int BUTTON_TEXTURE_COUNT = 4;
constexpr int CLOCK_TEXTURE_COUNT = 12;
constexpr int SELECTED_TEXTURE_COUNT = 2;

constexpr float HOVER_SCALE = 1.1f;
constexpr float HOVER_MARGIN = 0.05f;

constexpr sf::Color COLOR_LIGHT_SQUARE{0xEBECD0FF};
constexpr sf::Color COLOR_DARK_SQUARE{0x779556FF};
