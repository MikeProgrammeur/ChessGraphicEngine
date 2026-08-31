#pragma once

#include <array>
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>

#include "constants.hpp"

class TextureManager {
public:
    bool loadAll();

    const std::array<sf::Texture, CHESS_TEXTURE_COUNT>& chess() const { return m_chess; }
    const std::array<sf::Texture, BUTTON_TEXTURE_COUNT>& buttons() const { return m_buttons; }
    const std::array<sf::Texture, CLOCK_TEXTURE_COUNT>& clock() const { return m_clock; }
    const std::array<sf::Texture, SELECTED_TEXTURE_COUNT>& selected() const { return m_selected; }

private:
    std::array<sf::Texture, CHESS_TEXTURE_COUNT> m_chess{};
    std::array<sf::Texture, BUTTON_TEXTURE_COUNT> m_buttons{};
    std::array<sf::Texture, CLOCK_TEXTURE_COUNT> m_clock{};
    std::array<sf::Texture, SELECTED_TEXTURE_COUNT> m_selected{};
};
