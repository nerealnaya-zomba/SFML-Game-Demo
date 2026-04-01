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
class Player;

const std::string LEVELS_FOLDER = "data/levelData/";

class GameLevel
{
private:
    sf::Vector2i size{};
    std::shared_ptr<Platform> platforms;
    std::shared_ptr<Decoration> decorations;
    std::shared_ptr<Ground> ground;
    std::vector<std::shared_ptr<Background>> background;

    Player* player = nullptr;
    GameData* data = nullptr;
    GameCamera* camera = nullptr;
    GameLevelManager* levelManager = nullptr;
    std::unique_ptr<EnemyManager> enemyManager;
    sf::RenderWindow* window = nullptr;

    sf::Vector2f playerSpawnPos{};
    nlohmann::json loadedLevelData{};

    bool doResetToBase = true;
    bool isConstant = true;

    void initializePlatforms(const nlohmann::json& data);
    void initializeDecorations(const nlohmann::json& data);
    void initializeBackground(const nlohmann::json& data);
    void initializeGround(const nlohmann::json& data);
    void initializeEnemyManager(const nlohmann::json& data);
    void tryInitializeEnemyManager();

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

    void draw();
    void drawPlatforms();
    void drawDecorations();
    void drawBackgrounds();
    void drawGrounds();
    void drawEnemyManager();

    void loadLevelData(const std::string& fileName);
    void clearLevel();
    void saveLevelData();
    void resetTobase();

    void runErrorScreen(std::string errorString);

    sf::Vector2i getLevelSize() const;
    std::vector<std::shared_ptr<sf::RectangleShape>>& getPlatformRects();
    sf::RectangleShape& getGroundRect();
    sf::Vector2f getPlayerSpawnPos();
    sf::Sprite& getLevelBackgroundSprite();

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

    void draw();
    void drawPlatforms();
    void drawDecorations();
    void drawBackgrounds();
    void drawGrounds();
    void drawEnemyManager();

    sf::Vector2i getCurrentLevelSize() const;
    std::string getCurrentLevelName() const;
    std::vector<std::string> getLevelNames() const;
    std::vector<std::shared_ptr<sf::RectangleShape>>& getPlatformRects();
    sf::RectangleShape& getGroundRect();
    const std::map<std::string, std::shared_ptr<GameLevel>>& getLevelsMap() const;
    std::map<std::string, std::shared_ptr<GameLevel>>::iterator& getIteratorReference();

    void attachPlayer(Player& p);
    void registerDeathRecovery(const sf::Vector2f& position, int goldAmount);
    void handleEvent(const sf::Event& event);
};
