#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <string>

#include "memory.hpp"

// 0: BLACK
// 1: WHITE
// 2: RED
// 3: ORANGE
// 4: YELLOW
// 5: GREEN
// 6: BLUE
// 7: PURPLE
// 8: DARK RED
// 9: DARK ORANGE
// A: DARK YELLOW
// B: DARK GREEN
// C: DARK BLUE
// D: DARK PURPLE
// E: GRAY
// F: DARK GRAY

class Screen {
private:
    static constexpr unsigned int CHAR_HEIGHT = 8;
    static const uint8_t FONT[];
    static constexpr sf::Color BG_COLOR = sf::Color(10, 10, 10);
    static constexpr sf::Color COLOR[] {
        sf::Color(BG_COLOR.r,   BG_COLOR.g,   BG_COLOR.b),
        sf::Color(255,          255,          255     ),
        sf::Color(255,          BG_COLOR.g,   BG_COLOR.b),
        sf::Color(255,          128,          BG_COLOR.b),
        sf::Color(255,          255,          BG_COLOR.b),
        sf::Color(BG_COLOR.r,   255,          BG_COLOR.b),
        sf::Color(BG_COLOR.r,   BG_COLOR.g,   255     ),
        sf::Color(128,          BG_COLOR.g,   255     ),
        sf::Color(128,          BG_COLOR.g,   BG_COLOR.b),
        sf::Color(128,          64,           BG_COLOR.b),
        sf::Color(128,          128,          BG_COLOR.b),
        sf::Color(BG_COLOR.r,   128,          BG_COLOR.b),
        sf::Color(BG_COLOR.r,   BG_COLOR.g,   128     ),
        sf::Color(64,           BG_COLOR.g,   128     ),
        sf::Color(128,          128,          128     ),
        sf::Color(64,           64,           64      ),
    };

    const unsigned int width;
    const unsigned int height;
    sf::Image image;
    sf::Texture texture;
    sf::Sprite sprite;
    MemoryDevicePointer _memory;

    void draw_char(unsigned int x, unsigned int y, const uint8_t* bitmap, const sf::Color& foreground, const sf::Color& background);
    void draw_screen();
    void window_handler(unsigned int scale, unsigned int framerate);

public:
    Screen() = delete;
    Screen(const Screen&) = delete;
    Screen(Screen&&) = delete;

    Screen(
        unsigned int width,
        unsigned int height,
        unsigned int offset_x,
        unsigned int offset_y,
        float scale = 1.0f
    );

    MemoryDevice& memory() const;
    void debug_print(unsigned int x, unsigned int y, const std::string& str);

    void draw(sf::RenderWindow& window);
};

