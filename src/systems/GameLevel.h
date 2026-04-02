#pragma once

#include <Background.h>
#include <DeathRecovery.h>
#include <Decoration.h>
#include <EnemyManager.h>
#include <Ground.h>
#include <Platform.h>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

class Background;
class Decoration;
class EnemyManager;
class GameLevelManager;
class InteractiveObject;
class Player;

const std::string LEVELS_FOLDER = "data/levelData/";

class GameLevel
{
private:
    struct GeneratedMiniLocation
    {
        std::string title;
        std::string entranceTexture;
        std::string exitTexture;
        sf::Vector2f entrancePosition{0.f, 0.f};
        sf::Vector2f entranceDestinationSupport{0.f, 0.f};
        sf::Vector2f exitPosition{0.f, 0.f};
        sf::Vector2f exitDestinationSupport{0.f, 0.f};
        sf::Color entranceColor = sf::Color::White;
        sf::Color exitColor = sf::Color::White;
        sf::Color accentColor = sf::Color::White;
        float interactRadius = 120.f;
        std::string entrancePrompt;
        std::string exitPrompt;
        float roomLeftX = 0.f;
        float roomRightX = 0.f;
        float activeLeftX = 0.f;
        float activeRightX = 0.f;
        float roomCeilingY = 0.f;
        float roomFloorY = 0.f;
        float deathY = 0.f;
    };

    struct GeneratedMiniReward
    {
        std::string typeName;
        std::string textureName;
        sf::Vector2f position{0.f, 0.f};
        sf::Vector2f scale{1.f, 1.f};
        sf::Color color = sf::Color::White;
        sf::Color accentColor = sf::Color::White;
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

    struct GeneratedMiniBarrier
    {
        float x = 0.f;
        float topY = 0.f;
        float bottomY = 0.f;
        float width = 22.f;
        float phase = 0.f;
        sf::Color coreColor = sf::Color::White;
        sf::Color glowColor = sf::Color::White;
    };

    struct GeneratedMiniHazard
    {
        float leftX = 0.f;
        float rightX = 0.f;
        float topY = 0.f;
        float bottomY = 0.f;
        float phase = 0.f;
        sf::Color coreColor = sf::Color(255, 126, 72, 255);
        sf::Color glowColor = sf::Color(255, 208, 124, 255);
        sf::Color emberColor = sf::Color(255, 238, 190, 255);
    };

    sf::Vector2i size{};
    std::shared_ptr<Platform> platforms;
    std::shared_ptr<Decoration> decorations;
    std::shared_ptr<Ground> ground;
    std::vector<std::shared_ptr<Background>> background;
    std::vector<std::unique_ptr<InteractiveObject>> interactives;
    std::vector<GeneratedMiniLocation> generatedMiniLocations;
    std::vector<GeneratedMiniReward> generatedMiniRewards;
    std::vector<GeneratedMiniBarrier> generatedMiniBarriers;
    std::vector<GeneratedMiniHazard> generatedMiniHazards;
    sf::Clock miniLocationEffectsClock;

    Player* player = nullptr;
    GameData* data = nullptr;
    GameCamera* camera = nullptr;
    GameLevelManager* levelManager = nullptr;
    std::unique_ptr<EnemyManager> enemyManager;
    sf::RenderWindow* window = nullptr;

    sf::Vector2f playerSpawnPos{};
    nlohmann::json loadedLevelData{};
    int primaryWorldWidth = 0;
    float primaryWorldCameraRightEdge = 0.f;

    bool doResetToBase = true;
    bool isConstant = true;

    void initializePlatforms(const nlohmann::json& data);
    void initializeDecorations(const nlohmann::json& data);
    void initializeBackground(const nlohmann::json& data);
    void initializeGround(const nlohmann::json& data);
    void initializeEnemyManager(const nlohmann::json& data);
    void initializeInteractives(const nlohmann::json& data);
    void generateMiniLocations();
    void tryInitializeEnemyManager();
    void tryInitializeInteractives();
    void updateMiniLocationHazards();
    void drawMiniLocationBarriers();
    void drawMiniLocationHazards();

public:
    GameLevel(GameData& d, GameCamera& c, GameLevelManager& m, sf::RenderWindow& w, const std::string& fileNamePath);
    ~GameLevel();

    std::string levelName;

    void update();
    void updatePlatforms();
    void updateDecorations();
    void updateBackgrounds();
    void updateGrounds();
    void updateEnemyManager();
    void updateInteractives();

    void draw();
    void drawPlatforms();
    void drawDecorations();
    void drawBackgrounds();
    void drawGrounds();
    void drawEnemyManager();
    void drawInteractives();

    void loadLevelData(const std::string& fileName);
    void clearLevel();
    void saveLevelData();
    void resetTobase();

    void runErrorScreen(std::string errorString);

    sf::Vector2i getLevelSize() const;
    sf::FloatRect getCameraBoundsForPosition(const sf::Vector2f& position) const;
    std::vector<std::shared_ptr<sf::RectangleShape>>& getPlatformRects();
    sf::RectangleShape& getGroundRect();
    sf::Vector2f getPlayerSpawnPos();
    sf::Sprite& getLevelBackgroundSprite();
    bool handleEvent(const sf::Event& event);
    bool hasBlockingInteractiveModal() const;
    void setPlayerSpawnPos(const sf::Vector2f& pos);

    void attachPlayer(Player& p);
};

class GameLevelManager
{
private:
    Player* player = nullptr;
    GameData* data = nullptr;
    GameCamera* camera = nullptr;
    sf::RenderWindow* window = nullptr;

    const std::string levelsFolder;

    std::map<std::string, std::shared_ptr<GameLevel>> levels;
    std::map<std::string, std::shared_ptr<GameLevel>>::iterator levelIt;
    std::vector<std::unique_ptr<DeathRecovery>> deathRecoveries;

    void initializeLevels(const std::string& levelsFolder);
    void updateDeathRecoveries();
    void drawDeathRecoveries();
    sf::Vector2f findDeathRecoveryAnchor(const sf::Vector2f& position);

public:
    GameLevelManager(GameData& d, GameCamera& c, sf::RenderWindow& w, const std::string& lF);
    ~GameLevelManager();

    void setPlayerPositionToBase();

    bool goToLevel(std::optional<std::string> levelName);
    bool restartCurrentLevel();
    bool respawnPlayerAtCurrentSpawn();

    void update();
    void updatePlatforms();
    void updateDecorations();
    void updateBackgrounds();
    void updateGrounds();
    void updateEnemyManager();
    void updateInteractives();

    void draw();
    void drawPlatforms();
    void drawDecorations();
    void drawBackgrounds();
    void drawGrounds();
    void drawEnemyManager();
    void drawInteractives();

    sf::Vector2i getCurrentLevelSize() const;
    sf::FloatRect getCurrentCameraBoundsForPosition(const sf::Vector2f& position) const;
    std::string getCurrentLevelName() const;
    std::vector<std::string> getLevelNames() const;
    std::vector<std::shared_ptr<sf::RectangleShape>>& getPlatformRects();
    sf::RectangleShape& getGroundRect();
    const std::map<std::string, std::shared_ptr<GameLevel>>& getLevelsMap() const;
    std::map<std::string, std::shared_ptr<GameLevel>>::iterator& getIteratorReference();
    bool hasBlockingInteractiveModal() const;
    void setCurrentLevelSpawn(const sf::Vector2f& pos);

    void attachPlayer(Player& p);
    void clearDeathRecoveries();
    void registerDeathRecovery(const sf::Vector2f& position, int goldAmount);
    bool handleEvent(const sf::Event& event);
};
