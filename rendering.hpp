#pragma once

#include <string>
#include <SFML/Graphics.hpp>

#include "types.hpp"
#include "textures.hpp"

struct DisplayState {
    const Board& board;
    GridPos selectedSquare;
    int elapsedMillis;
    const CapturedPieces& capturedBlack;
    const CapturedPieces& capturedWhite;
    int secLeftBlack;
    int secLeftWhite;
    bool gameRunning;
};

struct InputState {
    bool leftClicked;
    sf::Vector2i mousePosition;
};

void drawCapturedPieces(sf::RenderWindow& window, const CapturedPieces& captured,
                        const Rect& area, bool white, int pieceDim,
                        const std::array<sf::Texture, CHESS_TEXTURE_COUNT>& textures);

void drawClock(sf::RenderWindow& window, int secLeft, const Rect& area,
               int clockDim, const std::array<sf::Texture, CLOCK_TEXTURE_COUNT>& textures);

std::string renderSidePanel(sf::RenderWindow& window, int height, int width,
                            const DisplayState& state, bool& gameRunning,
                            const TextureManager& textures, const InputState& input);

GridPos renderChessBoard(sf::RenderWindow& window, int width, int height,
                         const DisplayState& state,
                         const TextureManager& textures, const InputState& input);

void renderPVZone(sf::RenderWindow& window, int windowWidth, int boardAreaHeight,
                  int pvZoneHeight, const std::string& pv,
                  const sf::Font& font, const std::string& statusText);
