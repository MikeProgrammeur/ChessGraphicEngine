#include "textures.hpp"

static bool loadTexture(sf::Texture& tex, const std::string& path) {
    if (!tex.loadFromFile(path)) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return false;
    }
    return true;
}

bool TextureManager::loadAll() {
    const char* blackNames[] = {"Pawn", "Rook", "Knight", "Bishop", "Queen", "King"};
    const char* whiteNames[] = {"Pawn", "Rook", "Knight", "Bishop", "Queen", "King"};

    for (int i = 0; i < 6; ++i) {
        if (!loadTexture(m_chess[i], "./Assets/Pieces/Black/" + std::string(blackNames[i]) + ".png"))
            return false;
        if (!loadTexture(m_chess[i + 6], "./Assets/Pieces/White/" + std::string(whiteNames[i]) + ".png"))
            return false;
    }

    const char* buttonNames[] = {"Pause", "Play", "Snap", "Reset"};
    for (int i = 0; i < 4; ++i) {
        if (!loadTexture(m_buttons[i], "./Assets/Buttons/" + std::string(buttonNames[i]) + ".png"))
            return false;
    }

    const char* digitNames[] = {
        "Zero", "One", "Two", "Three", "Four", "Five",
        "Six", "Seven", "Eight", "Nine", "Clock", "DoublePoints"};
    for (int i = 0; i < 12; ++i) {
        if (!loadTexture(m_clock[i], "./Assets/Clock/" + std::string(digitNames[i]) + ".png"))
            return false;
    }

    if (!loadTexture(m_selected[0], "./Assets/Pieces/SelectedWhiteSquare.png"))
        return false;
    if (!loadTexture(m_selected[1], "./Assets/Pieces/SelectedBlackSquare.png"))
        return false;

    return true;
}
