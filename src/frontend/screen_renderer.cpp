#include "../../inc/frontend/screen_renderer.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

void ScreenRenderer::draw_char(unsigned int x, unsigned int y, const uint8_t* bitmap, const sf::Color& foreground, const sf::Color& background) {
    for (unsigned int y2 = 0; y2 < CHAR_HEIGHT; ++y2) {
        for (unsigned int x2 = 0; x2 < 8; ++x2) {
            const bool a = (bitmap[y2] >> x2) & 1;
            image.setPixel({x * 8 + x2, y * CHAR_HEIGHT + y2}, a ? foreground : background);
        }
    }
}

void ScreenRenderer::draw_screen() {
    auto& memory = screen->memory();

    for (unsigned int y = 0; y < screen->height; ++y) {
        for (unsigned int x = 0; x < screen->width; ++x) {
            const size_t memory_index = (y * screen->width + x) * 2;
            const uint8_t* bitmap = FONT + memory.read(memory_index).value * CHAR_HEIGHT;
            const uint8_t color_index = memory.read(memory_index + 1).value;
            
            draw_char(x, y, bitmap, COLOR[color_index >> 4], COLOR[color_index & 0x0F]);
        }
    }
    std::cout << memory.read(0).value << std::endl;
}

ScreenRenderer::ScreenRenderer(Screen* screen, unsigned int offset_x, unsigned int offset_y, float scale) :
    screen(screen),
    image({ screen->width * 8, screen->height * CHAR_HEIGHT }),
    texture(image),
    sprite(texture)
{
    sprite.setPosition({ (float)offset_x, (float)offset_y });
    sprite.setScale({ (float)scale, (float)scale });
}

void ScreenRenderer::draw(sf::RenderWindow& window) {
    draw_screen();
    texture.update(image);
    window.draw(sprite);
}