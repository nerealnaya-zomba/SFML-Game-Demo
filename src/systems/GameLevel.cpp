#include "Defines.h"
#include "Mounting.h"
#include "nlohmann/json_fwd.hpp"
#include <GameLevel.h>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <TGUI/AllWidgets.hpp>
#include <TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>

void GameLevelManager::initializeLevels(const std::string& levelsFolder)
{
    const std::filesystem::path levelsPath(levelsFolder);

    for (const auto& dirEntry : std::filesystem::directory_iterator{levelsPath})
    {
        const std::string onlyFileName = dirEntry.path().filename().u8string();

        levels.emplace(
            onlyFileName,
            std::make_shared<GameLevel>(
                *data,
                *camera,
                *this,
                *window,
                onlyFileName
            )
        );
    }
}

GameLevelManager::GameLevelManager(GameData& d, GameCamera& c, sf::RenderWindow& w, const std::string& lF)
    : data(&d)
    , camera(&c)
    , window(&w)
    , levelsFolder(lF)
{
    initializeLevels(levelsFolder);

    levelIt = levels.find("level1.json");
    if (levelIt == levels.end())
    {
        levelIt = levels.begin();
    }
}

GameLevelManager::~GameLevelManager() = default;

void GameLevelManager::setPlayerPositionToBase()
{
    if (!player || levels.empty() || levelIt == levels.end())
    {
        return;
    }

    player->setPosition(levelIt->second->getPlayerSpawnPos());
}

bool GameLevelManager::goToLevel(std::optional<std::string> levelName)
{
    if (levels.empty() || !levelName.has_value())
    {
        return false;
    }

    auto nextLevelIt = levels.find(*levelName);
    if (nextLevelIt == levels.end())
    {
        return false;
    }

    if (levelIt != levels.end())
    {
        levelIt->second->saveLevelData();
        levelIt->second->clearLevel();
    }

    levelIt = nextLevelIt;
    levelIt->second->loadLevelData(*levelName);

    if (player)
    {
        const sf::Vector2f spawnPos = levelIt->second->getPlayerSpawnPos();
        player->setPosition(spawnPos);
        camera->setCenterPosition(spawnPos);
    }

    return true;
}

bool GameLevelManager::restartCurrentLevel()
{
    if (levels.empty() || levelIt == levels.end())
    {
        return false;
    }

    const std::string currentLevelName = levelIt->first;
    levelIt->second->clearLevel();
    levelIt->second->loadLevelData(currentLevelName);

    if (player)
    {
        const sf::Vector2f spawnPos = levelIt->second->getPlayerSpawnPos();
        player->respawnAt(spawnPos);
        camera->setCenterPosition(spawnPos);
    }

    return true;
}

bool GameLevelManager::respawnPlayerAtCurrentSpawn()
{
    if (levels.empty() || levelIt == levels.end() || !player)
    {
        return false;
    }

    const sf::Vector2f spawnPos = levelIt->second->getPlayerSpawnPos();
    player->respawnAt(spawnPos);
    camera->setCenterPosition(spawnPos);
    return true;
}

void GameLevelManager::update()
{
    if (levels.empty() || levelIt == levels.end())
    {
        return;
    }

    levelIt->second->update();
}

void GameLevelManager::updatePlatforms()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updatePlatforms();
    }
}

void GameLevelManager::updateDecorations()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updateDecorations();
    }
}

void GameLevelManager::updateBackgrounds()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updateBackgrounds();
    }
}

void GameLevelManager::updateGrounds()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updateGrounds();
    }
}

void GameLevelManager::updateEnemyManager()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updateEnemyManager();
    }

    updateDeathRecoveries();
}

void GameLevelManager::draw()
{
    if (levels.empty() || levelIt == levels.end())
    {
        return;
    }

    levelIt->second->draw();
}

void GameLevelManager::drawPlatforms()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawPlatforms();
    }
}

void GameLevelManager::drawDecorations()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawDecorations();
    }
}

void GameLevelManager::drawBackgrounds()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawBackgrounds();
    }
}

void GameLevelManager::drawGrounds()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawGrounds();
    }
}

void GameLevelManager::drawEnemyManager()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawEnemyManager();
    }

    drawDeathRecoveries();
}

sf::Vector2i GameLevelManager::getCurrentLevelSize() const
{
    return levelIt != levels.end() ? levelIt->second->getLevelSize() : sf::Vector2i{};
}

std::string GameLevelManager::getCurrentLevelName() const
{
    return levelIt != levels.end() ? levelIt->second->levelName : std::string{};
}

std::vector<std::string> GameLevelManager::getLevelNames() const
{
    std::vector<std::string> levelNames;
    levelNames.reserve(levels.size());

    for (const auto& [levelName, _] : levels)
    {
        levelNames.push_back(levelName);
    }

    return levelNames;
}

std::vector<std::shared_ptr<sf::RectangleShape>>& GameLevelManager::getPlatformRects()
{
    if (levelIt == levels.end() || !levelIt->second)
    {
        throw std::runtime_error("Current level does not exist");
    }

    return levelIt->second->getPlatformRects();
}

sf::RectangleShape& GameLevelManager::getGroundRect()
{
    return levelIt->second->getGroundRect();
}

const std::map<std::string, std::shared_ptr<GameLevel>>& GameLevelManager::getLevelsMap() const
{
    return levels;
}

std::map<std::string, std::shared_ptr<GameLevel>>::iterator& GameLevelManager::getIteratorReference()
{
    return levelIt;
}

void GameLevelManager::attachPlayer(Player& p)
{
    player = &p;

    for (auto& [_, level] : levels)
    {
        level->attachPlayer(p);
    }
}

void GameLevelManager::registerDeathRecovery(const sf::Vector2f& position, int goldAmount)
{
    if (!data || goldAmount <= 0 || levelIt == levels.end())
    {
        return;
    }

    deathRecoveries.push_back(std::make_unique<DeathRecovery>(
        *data,
        levelIt->first,
        findDeathRecoveryAnchor(position),
        goldAmount
    ));
}

void GameLevelManager::handleEvent(const sf::Event& event)
{
    if (!player || levelIt == levels.end())
    {
        return;
    }

    for (auto& recovery : deathRecoveries)
    {
        if (recovery->belongsToLevel(levelIt->first) && recovery->handleEvent(event, *player))
        {
            break;
        }
    }

    deathRecoveries.erase(
        std::remove_if(deathRecoveries.begin(), deathRecoveries.end(),
            [](const std::unique_ptr<DeathRecovery>& recovery) {
                return recovery->isRecovered();
            }),
        deathRecoveries.end()
    );
}

void GameLevelManager::updateDeathRecoveries()
{
    if (!player || levelIt == levels.end())
    {
        return;
    }

    for (auto& recovery : deathRecoveries)
    {
        if (recovery->belongsToLevel(levelIt->first))
        {
            recovery->update(*player);
        }
    }

    deathRecoveries.erase(
        std::remove_if(deathRecoveries.begin(), deathRecoveries.end(),
            [](const std::unique_ptr<DeathRecovery>& recovery) {
                return recovery->isRecovered();
            }),
        deathRecoveries.end()
    );
}

void GameLevelManager::drawDeathRecoveries()
{
    if (levelIt == levels.end())
    {
        return;
    }

    for (auto& recovery : deathRecoveries)
    {
        if (recovery->belongsToLevel(levelIt->first))
        {
            recovery->draw(*window);
        }
    }
}

sf::Vector2f GameLevelManager::findDeathRecoveryAnchor(const sf::Vector2f& position)
{
    const float levelWidth = static_cast<float>(getCurrentLevelSize().x);
    const float clampedX = std::clamp(position.x, 18.f, std::max(18.f, levelWidth - 18.f));

    float supportY = getGroundRect().getPosition().y;
    for (const auto& platformRect : getPlatformRects())
    {
        const sf::FloatRect bounds = platformRect->getGlobalBounds();
        if (clampedX < bounds.position.x - 6.f || clampedX > bounds.position.x + bounds.size.x + 6.f)
        {
            continue;
        }

        if (bounds.position.y + 8.f < position.y)
        {
            continue;
        }

        supportY = std::min(supportY, bounds.position.y);
    }

    return {clampedX, supportY - 6.f};
}

GameLevel::GameLevel(GameData& d, GameCamera& c, GameLevelManager& m, sf::RenderWindow& w, const std::string& fileNamePath)
    : data(&d)
    , camera(&c)
    , levelManager(&m)
    , window(&w)
{
    loadLevelData(fileNamePath);
}

GameLevel::~GameLevel() = default;

void GameLevel::updatePlatforms()
{
}

void GameLevel::updateDecorations()
{
    if (decorations)
    {
        decorations->updateTextures();
    }
}

void GameLevel::updateBackgrounds()
{
    for (auto&& i : background)
    {
        i->update();
    }
}

void GameLevel::updateGrounds()
{
}

void GameLevel::updateEnemyManager()
{
    if (!enemyManager)
    {
        return;
    }

    enemyManager->updateAI_all();
    enemyManager->updateControls_all();
    enemyManager->updatePhysics_all();
    enemyManager->updateSpawners_all();
    enemyManager->updateTextures_all();
}

void GameLevel::update()
{
    updatePlatforms();
    updateDecorations();
    updateBackgrounds();
    updateGrounds();
}

void GameLevel::drawPlatforms()
{
    if (platforms)
    {
        platforms->draw(*window);
    }
}

void GameLevel::drawDecorations()
{
    if (decorations)
    {
        decorations->draw(*window);
    }
}

void GameLevel::drawBackgrounds()
{
    for (auto&& i : background)
    {
        i->draw(*window);
    }
}

void GameLevel::drawGrounds()
{
    if (ground)
    {
        ground->draw(*window);
    }
}

void GameLevel::drawEnemyManager()
{
    if (enemyManager)
    {
        enemyManager->draw_all();
    }
}

void GameLevel::initializePlatforms(const nlohmann::json& data)
{
    platforms = std::make_shared<Platform>();

    for (const auto& platform : data["Platforms"])
    {
        const sf::Vector2f position = {
            platform["Position"][0],
            platform["Position"][1]
        };

        const std::string type = platform["Type"];
        platforms->addPlatform(position, type);
    }
}

void GameLevel::initializeDecorations(const nlohmann::json& data)
{
    decorations = std::make_shared<Decoration>(*this->data, *this->camera);

    for (const auto& decoration : data["Decorations"])
    {
        const std::string name = decoration["Name"];
        const sf::Vector2f position = {decoration["Position"][0], decoration["Position"][1]};
        const sf::Vector2f scale = {decoration["Scale"][0], decoration["Scale"][1]};
        const sf::Color color = sf::Color{
            decoration["Color"][0],
            decoration["Color"][1],
            decoration["Color"][2],
            decoration["Color"][3]
        };
        const sf::Vector2f parallaxFactor = {decoration["ParallaxFactor"][0], decoration["ParallaxFactor"][1]};
        const int zDepth = decoration["Z"];

        decorations->addDecoration(name, position, scale, parallaxFactor, zDepth, color);
    }
}

void GameLevel::initializeBackground(const nlohmann::json& data)
{
    try
    {
        for (const auto& backgroundData : data["Background"])
        {
            const sf::Vector2f position = {backgroundData["Position"][0], backgroundData["Position"][1]};
            const sf::Vector2f parallaxFactor = {
                backgroundData["ParallaxFactor"][0],
                backgroundData["ParallaxFactor"][1]
            };
            const std::string name = backgroundData["BgName"];
            const Type type = backgroundData["Type"];

            background.push_back(
                std::make_shared<Background>(
                    *this->data,
                    *this->camera,
                    *this,
                    position,
                    name,
                    parallaxFactor,
                    type
                )
            );
        }
    }
    catch (const std::exception&)
    {
        const std::string errorMsg = "Error loading level data for: " + levelName;
        std::cout << std::endl << errorMsg << std::endl;
        runErrorScreen(errorMsg);
    }
}

void GameLevel::initializeGround(const nlohmann::json& data)
{
    for (const auto& groundData : data["Ground"])
    {
        const std::string groundName = groundData["GroundName"];
        const sf::Vector2u position = {groundData["Points"][0], groundData["Points"][1]};
        const unsigned int yPos = groundData["YPos"];

        ground = std::make_shared<Ground>(
            *this->data,
            *this,
            groundName,
            position.x,
            position.y,
            yPos
        );
    }
}

void GameLevel::initializeEnemyManager(const nlohmann::json& data)
{
    if (!player || !platforms || !ground)
    {
        return;
    }

    enemyManager = std::make_unique<EnemyManager>(
        data,
        *this->data,
        *this,
        *platforms,
        *ground,
        *player,
        *window
    );
}

void GameLevel::tryInitializeEnemyManager()
{
    if (player && !loadedLevelData.is_null())
    {
        initializeEnemyManager(loadedLevelData);
    }
}

void GameLevel::draw()
{
    drawBackgrounds();
    drawDecorations();
    drawGrounds();
    drawPlatforms();
}

void GameLevel::loadLevelData(const std::string& fileNamePath)
{
    std::ifstream dataFile(LEVELS_FOLDER + fileNamePath);
    if (!dataFile.good())
    {
        std::cerr << "Error reading level's json data:\n\t" << LEVELS_FOLDER + fileNamePath << " not found!\n";
        std::exit(EXIT_FAILURE);
    }

    loadedLevelData = nlohmann::json::parse(dataFile);

    size = sf::Vector2i(loadedLevelData["Presets"]["Size"][0], loadedLevelData["Presets"]["Size"][1]);
    levelName = fileNamePath;

    initializePlatforms(loadedLevelData);
    initializeDecorations(loadedLevelData);
    initializeBackground(loadedLevelData);
    initializeGround(loadedLevelData);

    enemyManager.reset();
    tryInitializeEnemyManager();

    playerSpawnPos = {
        loadedLevelData["Presets"]["PlayerSpawn"][0],
        loadedLevelData["Presets"]["PlayerSpawn"][1]
    };
}

void GameLevel::clearLevel()
{
    if (platforms)
    {
        platforms->clearPlatforms();
    }
    if (decorations)
    {
        decorations->clearDecorations();
    }
    if (enemyManager)
    {
        enemyManager->clearEnemies();
        enemyManager.reset();
    }

    background.clear();
    ground.reset();
    platforms.reset();
    decorations.reset();
}

void GameLevel::saveLevelData()
{
}

void GameLevel::resetTobase()
{
}

void GameLevel::runErrorScreen(std::string errorString)
{
    sf::Text text(*data->gameFont);
    text.setCharacterSize(50u);
    text.setString(errorString + "\n Press 'Q' to exit");
    setTextOriginToMiddle(text);
    text.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2});

    while (true)
    {
        while (const auto ev = this->window->pollEvent())
        {
            if (const auto* keyPressed = ev->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Q)
                {
                    std::exit(0);
                }
            }
        }

        window->clear(sf::Color::Red);
        window->draw(text);
        window->display();
    }
}

sf::Vector2i GameLevel::getLevelSize() const
{
    return size;
}

std::vector<std::shared_ptr<sf::RectangleShape>>& GameLevel::getPlatformRects()
{
    return platforms->getRects();
}

sf::RectangleShape& GameLevel::getGroundRect()
{
    return ground->getRect();
}

sf::Vector2f GameLevel::getPlayerSpawnPos()
{
    return playerSpawnPos;
}

sf::Sprite& GameLevel::getLevelBackgroundSprite()
{
    return background.begin()->get()->getSprite();
}

void GameLevel::attachPlayer(Player& p)
{
    player = &p;

    if (enemyManager)
    {
        enemyManager->attachPlayer(p);
        return;
    }

    tryInitializeEnemyManager();
}
