#include "rendering.hpp"
#include "layout.hpp"
#include "constants.hpp"

#include <SFML/Graphics.hpp>
#include <string>

static void positionAndScaleSprite(sf::Sprite& sprite, const Rect& area, int imgWidth, int imgHeight, float scaleMultiplier = 1.0f) {
    float sx = static_cast<float>(area.width) / imgWidth;
    float sy = static_cast<float>(area.height) / imgHeight;
    if (scaleMultiplier != 1.0f) {
        float offset = (scaleMultiplier - 1.0f) / 2.0f;
        sprite.setPosition(sf::Vector2f(
            static_cast<float>(area.left) - offset * area.width,
            static_cast<float>(area.top) - offset * area.height));
        sprite.setScale(sf::Vector2f(scaleMultiplier * sx, scaleMultiplier * sy));
    } else {
        sprite.setPosition(sf::Vector2f(
            static_cast<float>(area.left), static_cast<float>(area.top)));
        sprite.setScale(sf::Vector2f(sx, sy));
    }
}

static bool drawButton(sf::RenderWindow& window, const sf::Texture& texture, const Rect& area, int imgWidth, int imgHeight, const InputState& input) {
    sf::Sprite sprite(texture);
    if (checkMouseInBounds(input.mousePosition, area)) {
        positionAndScaleSprite(sprite, area, imgWidth, imgHeight, HOVER_SCALE);
        window.draw(sprite);
        return input.leftClicked;
    }
    positionAndScaleSprite(sprite, area, imgWidth, imgHeight);
    window.draw(sprite);
    return false;
}

// Captured pieces
void drawCapturedPieces(sf::RenderWindow& window, const CapturedPieces& captured,
                        const Rect& area, bool white, int pieceDim, const std::array<sf::Texture, CHESS_TEXTURE_COUNT>& textures) {
    int colorOffset = white ? 6 : 0;
    int xPos = area.left + area.width - area.height;

    for (int i = 5; i >= 0; --i) {
        sf::Sprite sprite(textures[i + colorOffset]);
        for (int j = 1; j <= captured[i]; ++j) {
            sprite.setPosition(sf::Vector2f(static_cast<float>(xPos), static_cast<float>(area.top)));
            float scale = static_cast<float>(area.height) / pieceDim;
            sprite.setScale(sf::Vector2f(scale, scale));
            window.draw(sprite);
            if (j < captured[i]) {
                xPos -= area.height / 4;
            }
        }
        if (captured[i] > 0) {
            xPos -= area.height;
        }
    }
}

// Clock
void drawClock(sf::RenderWindow& window, int secLeft, const Rect& area, int clockDim, const std::array<sf::Texture, CLOCK_TEXTURE_COUNT>& textures) {
    int xPos = area.left;

    sf::Sprite clockSprite(textures[10]);
    clockSprite.setPosition(sf::Vector2f(static_cast<float>(xPos),
                                        static_cast<float>(area.top)));
    float scale = static_cast<float>(area.height) / clockDim;
    clockSprite.setScale(sf::Vector2f(scale, scale));
    window.draw(clockSprite);

    xPos += area.height;

    auto digits = secToDigits(secLeft);
    for (int i = 0; i < 4; ++i) {
        sf::Sprite digitSprite(textures[digits[i]]);
        digitSprite.setPosition(sf::Vector2f(static_cast<float>(xPos),
                                            static_cast<float>(area.top)));
        digitSprite.setScale(sf::Vector2f(scale, scale));
        window.draw(digitSprite);
        xPos += area.height * 5 / 8;

        if (i == 1) {
            sf::Sprite colonSprite(textures[11]);
            colonSprite.setPosition(sf::Vector2f(static_cast<float>(xPos),
                                                static_cast<float>(area.top)));
            colonSprite.setScale(sf::Vector2f(scale, scale));
            window.draw(colonSprite);
            xPos += area.height * 3 / 8;
        }
    }
}

// Side panel (buttons, clocks, captured pieces)
std::string renderSidePanel(sf::RenderWindow& window, int height, int width,
                            const DisplayState& state, bool& gameRunning, const TextureManager& textures, const InputState& input) {
    std::string uciRequest;

    Rect ls = leftSpace(height, width);

    int imgH = 16;
    int eatedMaxW = static_cast<int>(8.5f * imgH);
    int timerH = 8;
    int timerW = 31;
    int btnW = 64;

    int eatedWhiteW = computeEatedWidth(state.capturedWhite, imgH);
    int eatedBlackW = computeEatedWidth(state.capturedBlack, imgH);

    if (isHorizontal(height, width)) {
        int rowH = ls.height / 7;

        // Row 1: captured white pieces
        Rect row1 = {0, ls.left, rowH, ls.width};
        Rect centered = centerPlot(row1, imgH, eatedMaxW, 0);
        centered.width = static_cast<int>(
            static_cast<float>(eatedWhiteW) * centered.height / imgH);
        drawCapturedPieces(window, state.capturedWhite, centered, true, imgH, textures.chess());

        // Row 2: black timer
        Rect row2 = {rowH, ls.left, rowH, ls.width};
        Rect centeredTimer = centerPlot(row2, timerH, timerW, 2);
        drawClock(window, state.secLeftBlack, centeredTimer, timerH, textures.clock());

        // Row 3: pause/play button
        Rect row3 = {2 * rowH, ls.left, rowH, ls.width};
        Rect centeredBtn = centerPlot(row3, imgH, btnW, 5);
        {
            int texIdx = gameRunning ? 0 : 1;
            if (drawButton(window, textures.buttons()[texIdx], centeredBtn, btnW, imgH, input)) {
                gameRunning = !gameRunning;
            }
        }

        // Row 4: screenshot button
        Rect row4 = {3 * rowH, ls.left, rowH, ls.width};
        Rect centeredSnap = centerPlot(row4, imgH, btnW, 5);
        if (drawButton(window, textures.buttons()[2], centeredSnap, btnW, imgH, input)) {
            uciRequest = "__screenshot__";
        }

        // Row 5: reset button
        Rect row5 = {4 * rowH, ls.left, rowH, ls.width};
        Rect centeredReset = centerPlot(row5, imgH, btnW, 5);
        if (drawButton(window, textures.buttons()[3], centeredReset, btnW, imgH, input)) {
            uciRequest = "ucinewgame";
        }

        // Row 6: white timer
        Rect row6 = {5 * rowH, ls.left, rowH, ls.width};
        Rect centeredWhiteTimer = centerPlot(row6, timerH, timerW, 2);
        drawClock(window, state.secLeftWhite, centeredWhiteTimer, timerH, textures.clock());

        // Row 7: captured black pieces
        Rect row7 = {6 * rowH, ls.left, rowH, ls.width};
        Rect centeredBlack = centerPlot(row7, imgH, eatedMaxW, 0);
        centeredBlack.width = static_cast<int>(
            static_cast<float>(eatedBlackW) * centeredBlack.height / imgH);
        drawCapturedPieces(window, state.capturedBlack, centeredBlack, false, imgH, textures.chess());
    }

    return uciRequest;
}

// Chess board
GridPos renderChessBoard(sf::RenderWindow& window, int width, int height,
                         const DisplayState& state, const TextureManager& textures, const InputState& input) {
    GridPos mouseInGrid;
    auto casesSides = squareInScreen(height, width);

    int curY = 0;
    for (int i = 0; i < BOARD_DIM; ++i) {
        int rowH = casesSides[i];
        int curX = 0;
        for (int j = 0; j < BOARD_DIM; ++j) {
            int colW = casesSides[j];

            Rect cell = {curY, curX, rowH, colW};
            bool mouseInCell = checkMouseInBounds(input.mousePosition, cell);
            if (mouseInCell) {
                mouseInGrid = {i, j};
            }

            // Draw square background
            sf::RectangleShape rect(sf::Vector2f( static_cast<float>(colW), static_cast<float>(rowH)));
            rect.setFillColor(((i + j) % 2 == 0) ? COLOR_LIGHT_SQUARE : COLOR_DARK_SQUARE);
            rect.setPosition(sf::Vector2f( static_cast<float>(curX), static_cast<float>(curY)));
            window.draw(rect);

            // Draw piece
            int piece = state.board[i][j];
            if (piece != EMPTY_SQUARE) {
                sf::Sprite sprite(textures.chess()[piece]);
                if (mouseInCell) {
                    positionAndScaleSprite(sprite, cell, PIECE_DIM, PIECE_DIM, HOVER_SCALE);
                } else {
                    positionAndScaleSprite(sprite, cell, PIECE_DIM, PIECE_DIM);
                }
                window.draw(sprite);
            }

            // Flickering selection overlay
            if (state.selectedSquare.isValid() &&
                state.selectedSquare == GridPos{i, j} &&
                (state.elapsedMillis % 1000) < 500) {
                int colorIdx = (i + j) % 2;
                sf::Sprite selSprite(textures.selected()[colorIdx]);
                positionAndScaleSprite(selSprite, cell, PIECE_DIM, PIECE_DIM);
                window.draw(selSprite);
            }

            curX += colW;
        }
        curY += rowH;
    }
    return mouseInGrid;
}

// PV zone (below the board)
void renderPVZone(sf::RenderWindow& window, int windowWidth, int boardAreaHeight,
                  int pvZoneHeight, const std::string& pv, const sf::Font& font, const std::string& statusText) {
    int y0 = boardAreaHeight;

    sf::RectangleShape bg(sf::Vector2f(static_cast<float>(windowWidth), static_cast<float>(pvZoneHeight)));
    bg.setPosition(sf::Vector2f(0.f, static_cast<float>(y0)));
    bg.setFillColor(sf::Color(0x2D2D2DFF));
    window.draw(bg);

    float fontSize = static_cast<float>(pvZoneHeight) * 0.38f;
    if (fontSize < 10.f) fontSize = 10.f;
    auto ufontSize = static_cast<unsigned int>(fontSize);

    float textY = static_cast<float>(y0) + 6.f;

    if (!statusText.empty()) {
        sf::Text statusTxt(font, sf::String(statusText), ufontSize);
        statusTxt.setFillColor(sf::Color(0xFF4444FF));
        statusTxt.setPosition(sf::Vector2f(12.f, textY));
        window.draw(statusTxt);
        textY += fontSize + 4.f;
    }

    if (!pv.empty()) {
        sf::Text pvText(font, sf::String(pv), static_cast<unsigned int>(fontSize * 0.9f));
        pvText.setFillColor(sf::Color(0x44FF44FF));
        pvText.setPosition(sf::Vector2f(12.f, textY));
        window.draw(pvText);
    }
}
