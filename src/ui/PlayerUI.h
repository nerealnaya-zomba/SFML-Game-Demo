#pragma once
#include<SFML/Graphics.hpp>
#include<Player.h>

// Cooldown
const sf::Vector2f BASE_UI_COOLDOWN_RECTS_SIZE              = {50.f,50.f};
const sf::Color BASE_UI_COOLDOWN_RECT_BACK_COLOR_ACTIVE     = sf::Color(180,180,180,255);
const sf::Color BASE_UI_COOLDOWN_RECT_BACK_COLOR_INACTIVE   = sf::Color::Black;
const sf::Color BASE_UI_COOLDOWN_RECT_FRONT_COLOR           = sf::Color::White;
const uint8_t BASE_UI_COOLDOWNT_RECT_BACK_ALPHA_            = 180;
const uint8_t BASE_UI_COOLDOWNT_RECT_FRONT_ALPHA_INACTIVE   = 180;
const uint8_t BASE_UI_COOLDOWNT_RECT_FRONT_ALPHA_ACTIVE     = 0;

// HP bar
const sf::Vector2f BASE_HP_BAR_OFFSET                       = {10.f,10.f};

class GameData;

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
    GameData* data;
    Player* player;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ПЕРЕЗАРЯДКА
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Представляет собой иконку отображающую перезарядку
    struct CooldownRect{
        int* targetCooldown;
        sf::Clock* currentCooldown;
        sf::RectangleShape back;
        sf::RectangleShape front;
        std::unique_ptr<sf::Sprite> icon;
    };

    std::vector<CooldownRect> cooldownRects;
    
    ///////////////////////////////////
    // Обновляет все cooldownRects: текущий cooldown, back(красный если не готов, зеленый если готов), front(линейная интерполяция размера по Y в зависимости от кулдауна)
    ///////////////////////////////////
    void updateCooldownRects();
        // Содержит:
        void updateCooldownRectsPos();
        void updateIterpolation();
        void updateCooldownRectsColor();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ЗДОРОВЬЕ
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    sf::RectangleShape hpBack;
    sf::RectangleShape hpFront;

    sf::Text hpText;

    void updateHP();
        void updateHpInterpolation();
        void updateHpText();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ЭНЕРГИЯ
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    sf::RectangleShape energyBack;
    sf::RectangleShape energyFront;

    sf::Text energyText;

    void updateEnergy();
        void updateEnergyInterpolation();
        void updateEnergyText();

public:
    PlayerUI(Player &p, GameCamera &c, GameData &d);
    ~PlayerUI() = default;

    void draw(sf::RenderWindow& window);
    void update();

    ///////////////////////////////////
    // Добавить отображение кулдауна
    ///////////////////////////////////
    void addCooldownRect(sf::Clock& currentCD, int& targetCD, sf::Texture& iconTexture);

};
