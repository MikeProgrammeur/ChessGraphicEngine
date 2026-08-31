#include "layout.hpp"

#include <algorithm>
#include <cmath>

bool isHorizontal(int height, int width) {
    return width > height;
}

std::array<int, 8> squareInScreen(int height, int width) {
    int squareSide = std::min(height, width);
    auto dv = std::div(squareSide, 8);
    std::array<int, 8> result;
    for (int i = 0; i < 8; ++i) {
        result[i] = (i < dv.rem) ? (dv.quot + 1) : dv.quot;
    }
    return result;
}

Rect leftSpace(int height, int width) {
    int squareSide = std::min(height, width);
    if (isHorizontal(height, width)) {
        return {0, squareSide, height, width - squareSide};
    }
    return {squareSide, 0, height - squareSide, width};
}

Rect centerPlot(const Rect& area, int imageHeight, int imageWidth, float borderInPercent) {
    float clamped = std::min(std::max(borderInPercent, 0.0f), 50.0f);
    float borderMultiplier = 1.0f - 2.0f * clamped / 100.0f;

    float alpha = std::min(
        borderMultiplier * static_cast<float>(area.height) / imageHeight,
        borderMultiplier * static_cast<float>(area.width) / imageWidth);

    Rect result;
    result.height = static_cast<int>(imageHeight * alpha);
    result.width = static_cast<int>(imageWidth * alpha);
    result.top = area.top + (area.height - result.height) / 2;
    result.left = area.left + (area.width - result.width) / 2;
    return result;
}

int computeEatedWidth(const CapturedPieces& captured, int pieceDim) {
    float result = 0.0f;
    for (int i = 0; i < 6; ++i) {
        if (captured[i] > 0) {
            result += 1.0f + (static_cast<float>(captured[i]) - 1.0f) / 4.0f;
        }
    }
    return static_cast<int>(pieceDim * result);
}

bool checkMouseInBounds(const sf::Vector2i& mousePos, const Rect& area) {
    return mousePos.y >= area.top &&
           mousePos.y < area.top + area.height &&
           mousePos.x >= area.left &&
           mousePos.x < area.left + area.width;
}

std::array<int, 4> secToDigits(int n) {
    int mins = n / 60;
    int secs = n % 60;
    return {mins / 10, mins % 10, secs / 10, secs % 10};
}
