#pragma once

#include <Interactive.h>

#include <GameCamera.h>
#include <GameData.h>
#include <Player.h>

#include <string>

class GameLevelManager;

class MiniLocationEntrance : public InteractiveObject
{
public:
    struct Config
    {
        std::string textureName;
        sf::Vector2f position{0.f, 0.f};
        sf::Vector2f scale{1.f, 1.f};
        sf::Vector2f destinationSupportPoint{0.f, 0.f};
        sf::Color color = sf::Color::White;
        sf::Color accentColor = sf::Color(130, 214, 184, 255);
        float interactRadius = 120.f;
        std::string prompt;
        std::string subtitle;
        bool exitsMiniLocation = false;
        std::string miniLocationId;
    };

private:
    GameCamera* camera = nullptr;
    GameLevelManager* levelManager = nullptr;
    Player* player = nullptr;
    std::vector<sf::Texture>* portalTextures_ = nullptr;
    texturesIterHelper portalTextureHelper_{};

    sf::Vector2f anchorPosition_{0.f, 0.f};
    sf::Vector2f destinationSupportPoint_{0.f, 0.f};
    sf::Vector2f baseScale_{1.f, 1.f};
    sf::Color baseColor_ = sf::Color::White;
    sf::Color accentColor_ = sf::Color(130, 214, 184, 255);
    bool exitsMiniLocation_ = false;
    std::string miniLocationId_{};

    sf::Clock animationClock_;
    sf::CircleShape shadowMouth_;
    sf::CircleShape outerGlow_;
    sf::CircleShape innerGlow_;
    sf::RectangleShape promptShadow_;
    sf::RectangleShape promptPlate_;
    sf::Text promptText_;
    sf::Text subtitleText_;

    void updateInteractionState();
    void updateAmbientMotion();
    void updatePromptLayout();

public:
    MiniLocationEntrance(GameData& data, GameCamera& camera, GameLevelManager& manager, Player& player, const Config& config);
    ~MiniLocationEntrance() override = default;

    void draw(sf::RenderWindow& window) override;
    void update() override;
    bool handleEvent(const sf::Event& event) override;
    bool blocksPlayerInput() const override;
};
