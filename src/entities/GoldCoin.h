#pragma once

#include <SFML/Graphics.hpp>

class Ground;
class Platform;
class Player;

class GoldCoin
{
private:
    sf::Vector2f position_{};
    sf::Vector2f velocity_{};
    float radius_ = 8.f;
    int value_ = 0;
    bool collected_ = false;
    bool grounded_ = false;
    float phaseOffset_ = 0.f;

    sf::CircleShape glow_;
    sf::CircleShape outerCoin_;
    sf::CircleShape innerCoin_;
    sf::RectangleShape shine_;
    sf::Clock animationClock_;

    sf::FloatRect getBoundsAt(const sf::Vector2f& position) const;
    void updateVisuals();
    void resolveGroundCollision(Ground& ground);
    void resolvePlatformCollision(Platform& platform);

public:
    GoldCoin(sf::Vector2f position, int value, sf::Vector2f initialVelocity);

    void update(Ground& ground, Platform& platform, Player& player, const sf::FloatRect& levelBounds);
    void draw(sf::RenderWindow& window);

    bool isCollected() const;
};
