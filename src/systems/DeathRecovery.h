#pragma once

#include <SFML/Graphics.hpp>

#include <array>
#include <string>

class GameData;
class Player;

constexpr sf::Keyboard::Scancode BASE_DEATH_RECOVERY_KEY = sf::Keyboard::Scancode::F;

class DeathRecovery
{
private:
    static constexpr std::size_t ORBIT_COIN_COUNT = 6;

    std::string levelName_;
    sf::Vector2f anchorPosition_{};
    int goldAmount_ = 0;
    bool recovered_ = false;
    bool playerInRange_ = false;

    sf::Clock animationClock_;

    sf::RectangleShape beam_;
    sf::CircleShape beamGlow_;
    sf::CircleShape coreGlow_;
    sf::CircleShape groundRing_;
    sf::RectangleShape promptBackground_;
    sf::Text amountText_;
    sf::Text promptText_;
    std::array<sf::CircleShape, ORBIT_COIN_COUNT> orbitCoins_;

    void updateVisuals();

public:
    DeathRecovery(GameData& data, std::string levelName, sf::Vector2f anchorPosition, int goldAmount);

    void update(Player& player);
    void draw(sf::RenderWindow& window);
    bool handleEvent(const sf::Event& event, Player& player);

    bool belongsToLevel(const std::string& levelName) const;
    bool isRecovered() const;
};
