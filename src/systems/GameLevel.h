#pragma once

#include <Background.h>
#include <DeathRecovery.h>
#include <Decoration.h>
#include <EnemyManager.h>
#include <Ground.h>
#include <LevelRegistry.h>
#include <Platform.h>
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

#include <NotificationFeed.h>

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
        std::string id;
        sf::FloatRect cameraBounds{};
        std::string entranceTexture;
        std::string exitTexture;
        sf::Vector2f entrancePosition{0.f, 0.f};
        sf::Vector2f entranceDestinationSupport{0.f, 0.f};
        sf::Vector2f entranceScale{0.22f, 0.33f};
        sf::Vector2f exitPosition{0.f, 0.f};
        sf::Vector2f exitDestinationSupport{0.f, 0.f};
        sf::Vector2f exitScale{0.22f, 0.33f};
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

    struct LevelEventZone
    {
        std::string title;
        std::string body;
        sf::FloatRect bounds{};
        NotificationTone tone = NotificationTone::Info;
        bool fireOnce = true;
        bool requireAlive = true;
        bool triggered = false;
    };

    struct SharedDrawEntry
    {
        enum class Kind
        {
            Background,
            Decoration,
            Ground,
            Actor,
            Interactive,
            Platform,
        };

        Kind kind = Kind::Decoration;
        std::size_t index = 0u;
        int order = 0;
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
    std::vector<LevelEventZone> levelEventZones;
    std::vector<SharedDrawEntry> sharedWorldDrawOrder_;
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
    std::optional<std::string> activeMiniLocationId_{};
    std::optional<sf::Vector2f> activeMiniLocationReturnSupport_{};

    bool doResetToBase = true;
    bool isConstant = true;
    bool useSharedWorldDrawOrder_ = false;
    bool introNotificationPending_ = false;
    std::string weatherThemeId_{};
    std::string weatherThemeTitle_{};

    void initializePlatforms(const nlohmann::json& data);
    void initializeDecorations(const nlohmann::json& data);
    void initializeBackground(const nlohmann::json& data);
    void initializeGround(const nlohmann::json& data);
    void initializeEnemyManager(const nlohmann::json& data);
    void initializeInteractives(const nlohmann::json& data);
    void initializePortals(const nlohmann::json& data);
    void initializeExplicitMiniLocations(const nlohmann::json& data);
    void generateMiniLocations();
    void tryInitializeEnemyManager();
    void tryInitializeInteractives();
    void initializeEventZones(const nlohmann::json& data);
    void initializeSharedDrawOrder(const nlohmann::json& data);
    void updateMiniLocationHazards();
    void updateLevelEvents();
    void drawMiniLocationBarriers();
    void drawMiniLocationHazards();
    void drawActors();
    void drawSharedWorldEntry(const SharedDrawEntry& entry);
    void queueNotification(std::string title, std::string body, NotificationTone tone) const;

public:
    GameLevel(GameData& d, GameCamera& c, GameLevelManager& m, sf::RenderWindow& w, const LevelDescriptor& descriptor);
    ~GameLevel();

    std::string levelName;
    std::string levelTitle;
    std::string sourceFileName;
    std::filesystem::path sourceFilePath{};

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
    void drawInteractiveOverlays();

    void loadLevelData(const LevelDescriptor& descriptor);
    void clearLevel();
    void saveLevelData();
    void resetTobase();

    void runErrorScreen(std::string errorString);

    sf::Vector2i getLevelSize() const;
    sf::FloatRect getCameraBoundsForPosition(const sf::Vector2f& position) const;
    bool enterMiniLocation(const std::string& id, std::optional<sf::Vector2f> returnSupportPoint = std::nullopt);
    void exitMiniLocation();
    sf::Vector2f exitMiniLocation(const sf::Vector2f& fallbackReturnSupportPoint);
    std::vector<std::shared_ptr<sf::RectangleShape>>& getPlatformRects();
    Platform& getPlatformSystem();
    sf::RectangleShape& getGroundRect();
    sf::Vector2f getPlayerSpawnPos();
    sf::Vector2f getTraderPosition() const;
    sf::Sprite& getLevelBackgroundSprite();
    bool handleEvent(const sf::Event& event);
    bool hasBlockingInteractiveModal() const;
    void setPlayerSpawnPos(const sf::Vector2f& pos);
    LevelDescriptor getLevelDescriptor() const;
    void onPlayerEnteredLevel();

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
    LevelRegistry levelRegistry_{};

    std::map<std::string, std::shared_ptr<GameLevel>> levels;
    std::map<std::string, std::shared_ptr<GameLevel>>::iterator levelIt;
    std::vector<std::unique_ptr<DeathRecovery>> deathRecoveries;
    std::function<void(std::string, std::string, NotificationTone)> notificationSink_ =
        [](std::string, std::string, NotificationTone) {};

    void initializeLevels(const std::string& levelsFolder);
    void updateDeathRecoveries();
    void drawDeathRecoveries();
    sf::Vector2f findDeathRecoveryAnchor(const sf::Vector2f& position);

public:
    GameLevelManager(GameData& d, GameCamera& c, sf::RenderWindow& w, const std::string& lF);
    ~GameLevelManager();

    void setPlayerPositionToBase();

    bool goToLevel(std::optional<std::string> levelName, bool ignoreUnlocks = false);
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
    void drawInteractiveOverlays();

    sf::Vector2i getCurrentLevelSize() const;
    sf::FloatRect getCurrentCameraBoundsForPosition(const sf::Vector2f& position) const;
    bool enterCurrentMiniLocation(const std::string& id, std::optional<sf::Vector2f> returnSupportPoint = std::nullopt);
    void exitCurrentMiniLocation();
    sf::Vector2f exitCurrentMiniLocation(const sf::Vector2f& fallbackReturnSupportPoint);
    std::string getCurrentLevelName() const;
    std::string getCurrentLevelTitle() const;
    std::vector<std::string> getLevelNames() const;
    std::map<std::string, std::string> getLevelDisplayNames() const;
    std::vector<std::shared_ptr<sf::RectangleShape>>& getPlatformRects();
    Platform& getCurrentPlatformSystem();
    sf::RectangleShape& getGroundRect();
    const std::map<std::string, std::shared_ptr<GameLevel>>& getLevelsMap() const;
    std::map<std::string, std::shared_ptr<GameLevel>>::iterator& getIteratorReference();
    const LevelRegistry& getLevelRegistry() const;
    std::optional<LevelDescriptor> resolveLevelIdentifier(const std::string& levelIdentifier) const;
    std::string getLevelDisplayName(const std::string& levelIdentifier) const;
    bool hasBlockingInteractiveModal() const;
    void setCurrentLevelSpawn(const sf::Vector2f& pos);
    sf::Vector2f getCurrentTraderPosition() const;
    bool teleportPlayerToCurrentLevelPosition(const sf::Vector2f& pos);
    bool teleportPlayerToCurrentMiniLocationPosition(
        const std::string& miniLocationId,
        const sf::Vector2f& pos,
        std::optional<sf::Vector2f> returnSupportPoint = std::nullopt
    );
    bool teleportPlayerToLevelPosition(const std::string& levelName, const sf::Vector2f& pos);

    void attachPlayer(Player& p);
    void setNotificationSink(std::function<void(std::string, std::string, NotificationTone)> sink);
    void pushNotification(std::string title, std::string body, NotificationTone tone) const;
    void clearDeathRecoveries();
    void registerDeathRecovery(const sf::Vector2f& position, int goldAmount);
    bool handleEvent(const sf::Event& event);
};
