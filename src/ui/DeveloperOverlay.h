#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include <array>
#include <functional>
#include <string>

class Player;
class GameCamera;
class GameLevelManager;
class GameData;

struct DeveloperOverlayActions
{
    std::function<void()> onRestoreVitals = []() {};
    std::function<void(int)> onGrantGold = [](int) {};
    std::function<void()> onRestartLevel = []() {};
    std::function<void()> onRespawnAtSpawn = []() {};
    std::function<void()> onReturnToBase = []() {};
    std::function<bool(const std::string&)> onGoToLevel = [](const std::string&) { return false; };
    std::function<void(bool)> onSetVsync = [](bool) {};
    std::function<void(int)> onSetMenuParticleCount = [](int) {};
};

class DeveloperOverlay
{
public:
    explicit DeveloperOverlay(sf::RenderWindow& window);
    ~DeveloperOverlay();

    void setActions(DeveloperOverlayActions actions);
    bool handleEvent(const sf::Event& event);
    void beginFrame(sf::Time deltaTime);
    void draw(
        Player& player,
        const GameData& gameData,
        const GameCamera& camera,
        const GameLevelManager& levelManager,
        bool hasActiveRun,
        bool deathSequenceActive
    );

    bool isOpen() const;

private:
    sf::RenderWindow* window_ = nullptr;
    DeveloperOverlayActions actions_{};
    bool isOpen_ = false;
    bool showImGuiDemo_ = false;
    int grantGoldAmount_ = 50;
    float smoothedFrameMs_ = 0.f;
    std::array<float, 180> frameHistory_{};
    std::size_t frameHistoryCount_ = 0;
    std::size_t frameHistoryIndex_ = 0;

    void applyStyle() const;
    void tryLoadGameFont() const;
    void recordFrameTime(sf::Time deltaTime);
    bool isKeyboardOrMouseEvent(const sf::Event& event) const;
};
