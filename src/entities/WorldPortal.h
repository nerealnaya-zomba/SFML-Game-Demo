#pragma once

#include <Interactive.h>

#include <GameCamera.h>
#include <GameData.h>
#include <GameLevel.h>
#include <Player.h>

#include <optional>
#include <string>
#include <vector>

class WorldPortal : public InteractiveObject
{
public:
    enum class TargetType
    {
        Level,
        MiniLocation,
        Position
    };

    struct Target
    {
        TargetType type = TargetType::Position;
        std::string levelId;
        std::string miniLocationId;
        sf::Vector2f position{0.f, 0.f};
        std::optional<sf::Vector2f> spawnPosition;
    };

    struct Config
    {
        std::string id;
        sf::Vector2f position{0.f, 0.f};
        sf::Vector2f scale{0.36f, 0.36f};
        sf::Color color = sf::Color(212, 236, 255, 245);
        sf::Color accentColor = sf::Color(112, 208, 255, 255);
        float interactRadius = 130.f;
        std::string portalTexture = "portalGreen";
        std::string prompt = "Enter portal";
        std::string title = "World Portal";
        Target target{};
    };

    WorldPortal(GameData& data, GameCamera& camera, GameLevelManager& manager, Player& player, const Config& config);
    ~WorldPortal() override = default;

    void draw(sf::RenderWindow& window) override;
    void update() override;
    bool handleEvent(const sf::Event& event) override;
    bool blocksPlayerInput() const override;

private:
    GameCamera* camera_ = nullptr;
    GameLevelManager* manager_ = nullptr;
    Player* player_ = nullptr;
    std::vector<sf::Texture>* portalTextures_ = nullptr;
    texturesIterHelper portalHelper_{};

    Config config_{};
    sf::Clock animationClock_{};
    sf::CircleShape outerGlow_{};
    sf::CircleShape innerGlow_{};
    sf::RectangleShape promptBack_{};
    sf::Text promptText_;
    bool transitioning_ = false;

    void updateInteractionState();
    void updateVisuals();
    void updatePromptLayout();
    bool activate();
};
