#pragma once

#include <array>
#include <SFML/System/Vector2.hpp>

#include "types.hpp"

bool isHorizontal(int height, int width);
std::array<int, 8> squareInScreen(int height, int width);
Rect leftSpace(int height, int width);
Rect centerPlot(const Rect& area, int imageHeight, int imageWidth, float borderInPercent);
int computeEatedWidth(const CapturedPieces& captured, int pieceDim);
bool checkMouseInBounds(const sf::Vector2i& mousePos, const Rect& area);
std::array<int, 4> secToDigits(int n);
