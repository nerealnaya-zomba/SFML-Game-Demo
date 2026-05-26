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

    sf::Vector2f baseScale;
    bool purchased = false;
public:
    enum class Category { Upgrade, Weapon };
    enum class WeaponKind { AshenBolt, Gravepiercer, PyreOrb, StormNeedler, DreadPrism, NightfallBeam };

    struct Stats {
        int bulletSpeed = 0;
        int bulletDistance = 0;
        int shootSpeedCooldownReduction = 0;
        int initialSpeed = 0;
        int maxSpeed = 0;
        int health = 0;
        int maxEnergy = 0;
        int damage = 0;
        int dashForce = 0;
        int dashCooldownReduction = 0;
        int extraJumpCount = 0;
        int jumpPower = 0;
        int slowFallPercent = 0;
    };

    struct WeaponStats {
        WeaponKind kind = WeaponKind::AshenBolt;
        int damageBonus = 0;
        int cooldownMs = 0;
        int energyCost = 0;
        int projectileSpeed = 0;
        int projectileRange = 0;
        int projectileCount = 1;
        int pierceCount = 0;
        int splashRadius = 0;
        int spread = 0;
    };

    const Stats stats;
    const WeaponStats weaponStats;

    enum Quality {COMMON, RARE, MYTH, LEGENDARY};
    const Quality quality;
    const Category category;

    const std::string iconName;
    const int price;
    const std::string displayName;  // Отображаемое название
    const std::string description;

    Item(
        GameData& data,
        sf::Vector2i iconSize,
        sf::Vector2i position,
        std::string name,
        std::string displayItemName,
        Quality q,
        Category c,
        int p,
        Stats itemStats,
        WeaponStats weaponStats,
        std::string itemDescription
    );
    ~Item() = default;

    void draw(sf::RenderWindow& window);

    // Getters
        // For mouse
    bool isIntersects(sf::Vector2i pos);
    sf::Vector2f getBaseScale();
    sf::Vector2u getTextureSize();
    sf::Vector2f getCenterPosition();
    sf::FloatRect getBounds() const;
    const sf::Texture& getTexture() const;
    bool isPurchased() const;

    // Setters
    void setPosition(const sf::Vector2i& pos);
    void setScale(sf::Vector2f scale);
    void setColor(const sf::Color& color);
    void markPurchased(bool value = true);
};
