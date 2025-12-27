#pragma once
#include<SFML/Graphics.hpp>

class HealthBar {
private:
    sf::RenderWindow* window;
    sf::Vector2f healthBar_size;
    sf::RectangleShape backRect;
    sf::RectangleShape frontRect;
    sf::Vector2f offset;
    sf::RectangleShape* targetRect;
    int maxHP;
    
public:
    HealthBar(sf::RectangleShape* targetRect, sf::RenderWindow& window, 
              sf::Color backRect_color, sf::Color frontRect_color, 
              sf::Vector2f healthBar_size, int currentHP, 
              sf::Vector2f offset = {0.f, -20.f})
        : window(&window), healthBar_size(healthBar_size), offset(offset), 
          targetRect(targetRect), maxHP(currentHP)
    {
        // Инициализация прямоугольников
        backRect.setFillColor(backRect_color);
        frontRect.setFillColor(frontRect_color);
        backRect.setSize(healthBar_size);
        frontRect.setSize(healthBar_size);
        
        // Устанавливаем origin в центр для точного позиционирования
        backRect.setOrigin({healthBar_size.x / 2, healthBar_size.y / 2});
        frontRect.setOrigin({healthBar_size.x / 2, healthBar_size.y / 2});
    }

    void update(int currentHP) {
        if (!targetRect) return;
        
        // Позиция над целью
        sf::Vector2f position = targetRect->getGlobalBounds().getCenter();
        position += offset;
        
        backRect.setPosition(position);
        frontRect.setPosition(position);
        
        // Обновление размера health bar в зависимости от HP
        if (currentHP > 0 && maxHP > 0) {
            float healthRatio = static_cast<float>(currentHP) / maxHP;
            float currentWidth = healthBar_size.x * healthRatio;
            frontRect.setSize({currentWidth, healthBar_size.y});
            
            // Сдвигаем origin чтобы health bar уменьшался с центра
            frontRect.setOrigin({currentWidth / 2, healthBar_size.y / 2});
        } else {
            frontRect.setSize({0.f, healthBar_size.y});
        }
    }

    void draw(bool isAlive) {
        if (window && isAlive) {
            window->draw(backRect);
            window->draw(frontRect);
        }
    }
    
    // Геттеры/сеттеры для настройки
    void setOffset(const sf::Vector2f& newOffset) { offset = newOffset; }
    void setColors(sf::Color backColor, sf::Color frontColor) {
        backRect.setFillColor(backColor);
        frontRect.setFillColor(frontColor);
    }
    void setMaxHP(int newMaxHP) { maxHP = newMaxHP; }
    int getMaxHP() const { return maxHP; }
    
    ~HealthBar() = default;
};

// Пример использования:
// HealthBar healthBar(&skeletonRect, window, sf::Color::Red, sf::Color::Green, {50.f, 5.f}, 100);
// В цикле: healthBar.update(currentHP); healthBar.draw();