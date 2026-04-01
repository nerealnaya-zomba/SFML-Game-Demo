#pragma once

#include<SFML/Graphics.hpp>
#include<Player.h>

#include<memory>
#include<vector>

// Cooldown
const sf::Vector2f BASE_UI_COOLDOWN_RECTS_SIZE              = {58.f,58.f};
const sf::Vector2f BASE_UI_COOLDOWN_INNER_SIZE              = {48.f,48.f};
const sf::Vector2f BASE_UI_COOLDOWN_STACK_OFFSET            = {26.f,0.f};
const float BASE_UI_COOLDOWN_STACK_GAP                      = 14.f;

// HP bar
const sf::Vector2f BASE_HP_BAR_OFFSET                       = {18.f,18.f};
const sf::Vector2f BASE_RESOURCE_BAR_SIZE                   = {600.f,42.f};
const float BASE_RESOURCE_BAR_GAP                           = 14.f;
const float BASE_RESOURCE_LABEL_WIDTH                       = 124.f;

// Inventory panel
const sf::Vector2f BASE_INVENTORY_PANEL_OFFSET              = {24.f,20.f};
const sf::Vector2f BASE_INVENTORY_PANEL_SIZE                = {320.f,0.f};
const sf::Vector2f BASE_INVENTORY_SLOT_SIZE                 = {48.f,48.f};
const sf::Vector2f BASE_INVENTORY_SLOT_GAP                  = {10.f,10.f};
const unsigned int BASE_INVENTORY_COLUMNS                   = 4;
const sf::Vector2f BASE_STATS_PANEL_OFFSET                  = {18.f,152.f};
const sf::Vector2f BASE_STATS_PANEL_SIZE                    = {264.f,0.f};
const float BASE_STATS_LINE_HEIGHT                          = 19.f;
const float BASE_STATS_LINE_GAP                             = 5.f;
const float BASE_STATS_PANEL_PADDING                        = 12.f;

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
        sf::RectangleShape shadow;
        sf::RectangleShape frame;
        sf::RectangleShape back;
        sf::RectangleShape front;
        sf::RectangleShape accent;
        sf::CircleShape readyGlow;
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
    sf::RectangleShape hpShadow;
    sf::RectangleShape hpFrame;
    sf::RectangleShape hpBack;
    sf::RectangleShape hpFront;
    sf::RectangleShape hpHighlight;
    sf::RectangleShape hpLabelPlate;
    sf::Text hpTextInfo;
    sf::Text hpText;

    void updateHP();
    void updateHpInterpolation();
    void updateHpText();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ЭНЕРГИЯ
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    sf::RectangleShape energyShadow;
    sf::RectangleShape energyFrame;
    sf::RectangleShape energyBack;
    sf::RectangleShape energyFront;
    sf::RectangleShape energyHighlight;
    sf::RectangleShape energyLabelPlate;

    sf::Text energyTextInfo;
    sf::Text energyText;

    void updateEnergy();
    void updateEnergyInterpolation();
    void updateEnergyText();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ХАРАКТЕРИСТИКИ
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct StatLineVisual
    {
        sf::RectangleShape plate;
        sf::RectangleShape accent;
        sf::Text label;
        sf::Text value;

        explicit StatLineVisual(sf::Font& font)
            : label(font)
            , value(font)
        {
        }
    };

    sf::RectangleShape statsPanelShadow;
    sf::RectangleShape statsPanelBack;
    sf::RectangleShape statsHeaderAccent;
    sf::RectangleShape statsSideSigil;
    sf::RectangleShape statsDivider;
    sf::Text statsTitleText;
    sf::Text statsWeaponText;
    std::vector<StatLineVisual> statLines;

    void updateStatsPanel();
    void rebuildStatLines();

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
    sf::RectangleShape weaponChip;
    sf::CircleShape goldCoinGlow;
    sf::CircleShape goldCoinOuter;
    sf::CircleShape goldCoinInner;
    sf::RectangleShape goldCoinShine;

    sf::Text inventoryTitleText;
    sf::Text inventoryGoldText;
    sf::Text inventoryWeaponText;
    sf::Text inventoryWeaponHintText;
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
