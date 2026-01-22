#pragma once
#include<SFML/Graphics.hpp>
#include<Player.h>

const sf::Vector2f BASE_UI_COOLDOWN_RECTS_SIZE              = {50.f,50.f};
const sf::Color BASE_UI_COOLDOWN_RECT_BACK_COLOR_ACTIVE     = sf::Color::Green;
const sf::Color BASE_UI_COOLDOWN_RECT_BACK_COLOR_INACTIVE   = sf::Color::Red;
const sf::Color BASE_UI_COOLDOWN_RECT_FRONT_COLOR           = sf::Color::White;

///////////////////////////////////
// Реализует интерфейс игрока:
// *  Здоровье
// *  Энергия
// 
// *  Кулдаун телепорта
// *  Кулдаун выстрела
// *  Кулдаун деша
// *  
// 
///////////////////////////////////
class PlayerUI
{
private:
    // Внешние объекты
    GameCamera* camera;
    Player* player;

    
    // Переменные для линейной интерполяции перезарядок
    const sf::Vector2f minSize;
    const sf::Vector2f maxSize;

    // Представляет собой иконку отображающую перезарядку
    struct CooldownRect{
        int* cooldown;
        sf::RectangleShape back;
        sf::RectangleShape front;
        std::unique_ptr<sf::Sprite> icon;
    };

    std::vector<CooldownRect> cooldownRects;
    
    // Обновляет все cooldownRects: текущий cooldown, back(красный если не готов, зеленый если готов), front(линейная интерполяция размера по Y в зависимости от кулдауна)
    void updateCooldownRects();
    void addCooldownRect(int& cd, sf::Texture& iconTexture);

public:
    PlayerUI(Player& p, GameCamera& c, const sf::Vector2f mnS, const sf::Vector2f mxS);
    ~PlayerUI() = default;

    void draw(sf::RenderWindow& window);
};
