#pragma once
#include<SFML/Graphics.hpp>
#include<SFML/System.hpp>
#include<math.h>
#include<map>

/*Make sprite pulse once, with certain color
    target - target to pulse
    pulseColor - color to pulse with
    speed - speed of pulse
*/
static void pulseSprite(sf::Sprite& sprite, const sf::Color& pulseColor, float speed, sf::Time duration) {
    static std::map<sf::Sprite*, sf::Color> originalColors;
    static std::map<sf::Sprite*, sf::Clock> clocks;
    
    // Сохраняем оригинальный цвет при первом вызове
    if (originalColors.find(&sprite) == originalColors.end()) {
        originalColors[&sprite] = sprite.getColor();
        clocks[&sprite].restart();
    }
    
    float elapsed = clocks[&sprite].getElapsedTime().asSeconds();
    float progress = elapsed / duration.asSeconds();
    
    if (progress >= 1.0f) {
        // Возвращаем оригинальный цвет и очищаем данные
        sprite.setColor(originalColors[&sprite]);
        originalColors.erase(&sprite);
        clocks.erase(&sprite);
        return;
    }
    
    // Пульсация через синус
    float alpha = (std::sin(elapsed * speed * 2 * 3.14159f) + 1.0f) / 2.0f;
    
    const sf::Color& original = originalColors[&sprite];
    sf::Color current;
    current.r = static_cast<uint8_t>(original.r + (pulseColor.r - original.r) * alpha);
    current.g = static_cast<uint8_t>(original.g + (pulseColor.g - original.g) * alpha);
    current.b = static_cast<uint8_t>(original.b + (pulseColor.b - original.b) * alpha);
    current.a = sprite.getColor().a;
    
    sprite.setColor(current);
}