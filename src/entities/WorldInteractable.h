#pragma once

#include <Interactive.h>

#include <GameCamera.h>
#include <GameData.h>
#include <Player.h>

#include <string>

class GameLevelManager;

class WorldInteractable : public InteractiveObject
{
public:
    enum class Type
    {
        RestShrine,
        EchoTablet,
        GoldCache
    };

    struct Config
    {
        Type type = Type::EchoTablet;
        std::string textureName;
        sf::Vector2f position{0.f, 0.f};
        sf::Vector2f scale{1.f, 1.f};
        sf::Color color = sf::Color::White;
        sf::Color accentColor = sf::Color(220, 184, 122, 255);
        float interactRadius = 120.f;
        int goldReward = 0;
        bool singleUse = true;
        bool grantsCheckpoint = false;
        bool restoreVitality = false;
        bool hasCustomSpawnOffset = false;
        sf::Vector2f spawnOffset{0.f, 0.f};
        std::string prompt;
        std::string title;
        std::string body;
    };

private:
    GameData* data = nullptr;
    GameCamera* camera = nullptr;
    GameLevelManager* manager = nullptr;
    Player* player = nullptr;

    Type type_ = Type::EchoTablet;
    sf::Vector2f anchorPosition_{0.f, 0.f};
    sf::Vector2f baseScale_{1.f, 1.f};
    sf::Color baseColor_ = sf::Color::White;
    sf::Color accentColor_ = sf::Color(220, 184, 122, 255);
    int goldReward_ = 0;
    bool singleUse_ = true;
    bool grantsCheckpoint_ = false;
    bool restoreVitality_ = false;
    bool hasCustomSpawnOffset_ = false;
    sf::Vector2f spawnOffset_{0.f, 0.f};
    bool activated_ = false;
    bool panelOpen_ = false;

    sf::Clock animationClock_;
    float animationPhase_ = 0.f;
    sf::CircleShape shadow_;
    sf::CircleShape halo_;
    sf::CircleShape innerHalo_;
    sf::RectangleShape promptShadow_;
    sf::RectangleShape promptPlate_;
    sf::Text promptText_;

    sf::RectangleShape panelShadow_;
    sf::RectangleShape panelBack_;
    sf::RectangleShape panelAccent_;
    sf::Text titleText_;
    sf::Text bodyText_;
    sf::Text hintText_;

    const sf::Texture& resolveTexture(const std::string& textureName) const;
    void updateInteractionState();
    void updateAmbientMotion();
    void updatePromptLayout();
    void updatePanelLayout();
    void openPanel();
    void closePanel();
    void performActivation();
    sf::Vector2f calculateSpawnPoint() const;
    static std::string wrapText(const std::string& text, std::size_t maxLineLength);

public:
    WorldInteractable(GameData& data, GameCamera& camera, GameLevelManager& manager, Player& player, const Config& config);
    ~WorldInteractable() override;

    void draw(sf::RenderWindow& window) override;
    void update() override;
    bool handleEvent(const sf::Event& event) override;
    bool blocksPlayerInput() const override;
};
