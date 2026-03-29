#pragma once

#include<SFML/Graphics.hpp>
#include<Player.h>

#include<memory>
#include<vector>

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

// Inventory panel
const sf::Vector2f BASE_INVENTORY_PANEL_OFFSET              = {24.f,20.f};
const sf::Vector2f BASE_INVENTORY_PANEL_SIZE                = {320.f,0.f};
const sf::Vector2f BASE_INVENTORY_SLOT_SIZE                 = {48.f,48.f};
const sf::Vector2f BASE_INVENTORY_SLOT_GAP                  = {10.f,10.f};
const unsigned int BASE_INVENTORY_COLUMNS                   = 4;

class GameData;
class GameCamera;

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
    struct CooldownRect{
        int* targetCooldown;
        sf::Clock* currentCooldown;
        sf::RectangleShape back;
        sf::RectangleShape front;
        std::unique_ptr<sf::Sprite> icon;
    };

    std::vector<CooldownRect> cooldownRects;
    
    void updateCooldownRects();
    void updateCooldownRectsPos();
    void updateIterpolation();
    void updateCooldownRectsColor();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ЗДОРОВЬЕ
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    sf::RectangleShape hpBack;
    sf::RectangleShape hpFront;
    sf::Text hpTextInfo;
    sf::Text hpText;

    void updateHP();
    void updateHpInterpolation();
    void updateHpText();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ЭНЕРГИЯ
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    sf::RectangleShape energyBack;
    sf::RectangleShape energyFront;

    sf::Text energyTextInfo;
    sf::Text energyText;

    void updateEnergy();
    void updateEnergyInterpolation();
    void updateEnergyText();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ИНВЕНТАРЬ И ЗОЛОТО
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct InventorySlotVisual {
        sf::RectangleShape shadow;
        sf::RectangleShape background;
        sf::RectangleShape accent;
        std::unique_ptr<sf::Sprite> icon;
    };

    sf::RectangleShape inventoryPanelShadow;
    sf::RectangleShape inventoryPanelBack;
    sf::RectangleShape inventoryHeaderAccent;
    sf::RectangleShape inventoryDivider;
    sf::RectangleShape goldChip;
    sf::CircleShape goldCoinGlow;
    sf::CircleShape goldCoinOuter;
    sf::CircleShape goldCoinInner;
    sf::RectangleShape goldCoinShine;

    sf::Text inventoryTitleText;
    sf::Text inventoryGoldText;
    sf::Text inventoryEmptyText;

    std::vector<InventorySlotVisual> inventorySlots;
    sf::Clock uiAnimationClock;

    void updateInventoryPanel();
    void syncInventoryIcons();

public:
    PlayerUI(Player &p, GameCamera &c, GameData &d);
    ~PlayerUI() = default;

    void draw(sf::RenderWindow& window);
    void update();

    void addCooldownRect(sf::Clock& currentCD, int& targetCD, sf::Texture& iconTexture);
};
