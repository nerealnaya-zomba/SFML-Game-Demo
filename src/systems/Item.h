#pragma once
#include<SFML/Graphics.hpp>
#include<GameData.h>

const sf::Vector2f BASE_ITEM_SCALE = {1.f,1.f};

////////////////////////////////////////////////////////////////////////////////////////
// Класс реализующий вещь со своими характеристиками и ценой
// 
// Позиционирование иконки с левого-верхнего угла
////////////////////////////////////////////////////////////////////////////////////////
class Item
{
private:
    sf::IntRect rect;
    std::unique_ptr<sf::Sprite> sprite; // Иконка

public:
    struct Stats {
        int bulletSpeed = 0;
        int bulletDistance = 0;
        int shootSpeedCooldownReduction = 0;
        int initialSpeed = 0;
        int maxSpeed = 0;
        float health = 0;
        float damage = 0;
    };
    
    const Stats stats;

    enum Quality {COMMON, RARE, MYTH, LEGENDARY};
    const Quality quality;

    const int price;
    const std::string displayName;  // Отображаемое название

    Item(GameData& data, sf::Vector2i iconSize, sf::Vector2i position, std::string name, std::string displayItemName, Quality q, int p, Stats itemStats);
    ~Item() = default;

    void draw(sf::RenderWindow& window);

    // Setters
    void setPosition(sf::Vector2i& pos);
};