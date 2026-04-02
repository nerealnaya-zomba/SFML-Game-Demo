#include "Defines.h"
#include "Mounting.h"
#include "nlohmann/json_fwd.hpp"
#include <GameLevel.h>
#include <MiniLocationEntrance.h>
#include <WorldInteractable.h>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <TGUI/AllWidgets.hpp>
#include <TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <random>
#include <stdexcept>
#include <string>

#include <Player.h>

namespace
{
struct MiniLocationCandidate
{
    float centerX = 0.f;
    float surfaceY = 0.f;
    float leftX = 0.f;
    float rightX = 0.f;
};

struct MiniLocationTheme
{
    std::string title;
    std::string entranceTexture;
    std::string exitTexture;
    std::string backdropDecoration;
    std::string hillDecoration;
    std::string hangingDecoration;
    std::string ambientDecoration;
    std::string rewardTypeName;
    std::string rewardTexture;
    sf::Vector2f rewardScale{1.f, 1.f};
    sf::Color entranceColor = sf::Color::White;
    sf::Color exitColor = sf::Color::White;
    sf::Color accentColor = sf::Color::White;
    int goldReward = 0;
    bool grantsCheckpoint = false;
    bool restoreVitality = false;
    std::string entrancePrompt;
    std::string exitPrompt;
    std::string rewardPrompt;
    std::string rewardTitle;
    std::string rewardBody;
};

std::mt19937& miniLocationRng()
{
    static std::mt19937 rng(std::random_device{}());
    return rng;
}

float randomFloat(float minValue, float maxValue)
{
    std::uniform_real_distribution<float> dist(minValue, maxValue);
    return dist(miniLocationRng());
}

template <typename T>
const T& pickRandom(const std::vector<T>& values)
{
    std::uniform_int_distribution<std::size_t> dist(0u, values.size() - 1u);
    return values[dist(miniLocationRng())];
}

std::uint8_t clampToByte(float value)
{
    return static_cast<std::uint8_t>(std::clamp(value, 0.f, 255.f));
}

sf::Color withAlpha(const sf::Color& color, float alpha)
{
    return sf::Color(color.r, color.g, color.b, clampToByte(alpha));
}

sf::Color brighten(const sf::Color& color, float factor)
{
    const float clampedFactor = std::clamp(factor, 0.f, 1.f);
    const auto brightenChannel = [&](std::uint8_t channel) {
        return clampToByte(static_cast<float>(channel) + (255.f - static_cast<float>(channel)) * clampedFactor);
    };

    return sf::Color(
        brightenChannel(color.r),
        brightenChannel(color.g),
        brightenChannel(color.b),
        color.a
    );
}

float fract(float value)
{
    return value - std::floor(value);
}

float seededNoise(sf::Vector2f position, int saltA, float saltB)
{
    const float value = std::sin(
        position.x * 0.01373f
        + position.y * 0.00919f
        + static_cast<float>(saltA) * 0.07131f
        + saltB * 17.0f
    ) * 43758.5453f;

    return fract(value);
}

const std::vector<MiniLocationTheme>& getMiniLocationThemes()
{
    static const std::vector<MiniLocationTheme> themes{
        {
            "Sunken Grotto",
            "MossyDecorationHazard_25.png",
            "MossyDecorationHazard_24.png",
            "MossyBackgroundDecoration_09.png",
            "MossyHills_06.png",
            "MossyHangingPlants_06.png",
            "blueFlower1",
            "GoldCache",
            "Item_29.png",
            {3.8f, 3.8f},
            sf::Color(214, 246, 232, 255),
            sf::Color(212, 232, 255, 255),
            sf::Color(102, 202, 178, 255),
            115,
            false,
            false,
            "Enter to descend into grotto",
            "Enter to climb back",
            "Enter to claim the flooded cache",
            "Flooded Reliquary",
            "Coins and moon-bright tokens survived where the roots kept the chamber sealed. Something down here waited to be remembered."
        },
        {
            "Root Burrow",
            "MossyDecorationHazard_23.png",
            "MossyDecorationHazard_25.png",
            "MossyBackgroundDecoration_07.png",
            "MossyHills_05.png",
            "MossyHangingPlants_03.png",
            "windPlant1",
            "EchoTablet",
            "MossyDecorationHazard_11.png",
            {0.28f, 0.28f},
            sf::Color(222, 246, 218, 255),
            sf::Color(236, 250, 224, 255),
            sf::Color(136, 210, 112, 255),
            65,
            false,
            false,
            "Enter the root burrow",
            "Enter to return outside",
            "Enter to read the root-etched tablet",
            "Root-Etched Tablet",
            "The tunnel walls whisper of hunters who marked side-paths with buried stone. They hid warnings where only the curious would find them."
        },
        {
            "Forgotten Reliquary",
            "MossyDecorationHazard_21.png",
            "MossyDecorationHazard_24.png",
            "MossyBackgroundDecoration_08.png",
            "MossyHills_04.png",
            "MossyHangingPlants_07.png",
            "blueFlower2",
            "RestShrine",
            "MossyDecorationHazard_21.png",
            {0.29f, 0.29f},
            sf::Color(248, 236, 220, 255),
            sf::Color(255, 228, 196, 255),
            sf::Color(224, 176, 108, 255),
            0,
            true,
            true,
            "Enter the sealed reliquary",
            "Enter to leave the reliquary",
            "Enter to attune reliquary flame",
            "Reliquary Flame",
            "A hidden ember still keeps watch beneath the stone. It can hold your return here and steady the breath you carry deeper underground."
        }
    };

    return themes;
}

bool hasEntranceClearance(
    const MiniLocationCandidate& candidate,
    const std::vector<std::shared_ptr<sf::RectangleShape>>& platformRects
)
{
    constexpr float kRequiredHalfWidth = 96.f;
    constexpr float kMaxCeilingGap = 210.f;

    for (const auto& platformRect : platformRects)
    {
        const sf::FloatRect bounds = platformRect->getGlobalBounds();
        const float platformBottom = bounds.position.y + bounds.size.y;
        if (platformBottom >= candidate.surfaceY - 4.f)
        {
            continue;
        }

        const float verticalGap = candidate.surfaceY - platformBottom;
        if (verticalGap > kMaxCeilingGap)
        {
            continue;
        }

        if (candidate.centerX + kRequiredHalfWidth < bounds.position.x
            || candidate.centerX - kRequiredHalfWidth > bounds.position.x + bounds.size.x)
        {
            continue;
        }

        return false;
    }

    return true;
}

void addMiniLocationSeal(Platform& platforms, const float x, const float topY, const float bottomY)
{
    const float startY = std::min(topY, bottomY);
    const float endY = std::max(topY, bottomY);

    for (float currentY = startY; currentY <= endY; currentY += 170.f)
    {
        platforms.addPlatform({x, currentY}, "Invisible-wall");
    }
}
}

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

    if (player && !player->isLevelUnlocked(*levelName))
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
        player->notifyLevelEntered(levelIt->first);
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
        player->notifyLevelEntered(levelIt->first);
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

void GameLevelManager::updateInteractives()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updateInteractives();
    }
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

void GameLevelManager::drawInteractives()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawInteractives();
    }
}

sf::Vector2i GameLevelManager::getCurrentLevelSize() const
{
    return levelIt != levels.end() ? levelIt->second->getLevelSize() : sf::Vector2i{};
}

sf::FloatRect GameLevelManager::getCurrentCameraBoundsForPosition(const sf::Vector2f& position) const
{
    if (levelIt == levels.end() || !levelIt->second)
    {
        return sf::FloatRect({0.f, 0.f}, sf::Vector2f{0.f, 0.f});
    }

    return levelIt->second->getCameraBoundsForPosition(position);
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

bool GameLevelManager::hasBlockingInteractiveModal() const
{
    return levelIt != levels.end() && levelIt->second && levelIt->second->hasBlockingInteractiveModal();
}

void GameLevelManager::setCurrentLevelSpawn(const sf::Vector2f& pos)
{
    if (levelIt == levels.end() || !levelIt->second)
    {
        return;
    }

    levelIt->second->setPlayerSpawnPos(pos);
}

void GameLevelManager::attachPlayer(Player& p)
{
    player = &p;

    for (auto& [_, level] : levels)
    {
        level->attachPlayer(p);
    }

    if (levelIt != levels.end())
    {
        player->notifyLevelEntered(levelIt->first);
    }
}

void GameLevelManager::clearDeathRecoveries()
{
    deathRecoveries.clear();
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

bool GameLevelManager::handleEvent(const sf::Event& event)
{
    if (!player || levelIt == levels.end())
    {
        return false;
    }

    for (auto& recovery : deathRecoveries)
    {
        if (recovery->belongsToLevel(levelIt->first) && recovery->handleEvent(event, *player))
        {
            return true;
        }
    }

    if (levelIt->second->handleEvent(event))
    {
        return true;
    }

    deathRecoveries.erase(
        std::remove_if(deathRecoveries.begin(), deathRecoveries.end(),
            [](const std::unique_ptr<DeathRecovery>& recovery) {
                return recovery->isRecovered();
            }),
        deathRecoveries.end()
    );

    return false;
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

void GameLevel::updateInteractives()
{
    for (auto& interactive : interactives)
    {
        interactive->update();
    }
}

void GameLevel::update()
{
    updatePlatforms();
    updateDecorations();
    updateBackgrounds();
    updateGrounds();
    updateInteractives();
    updateMiniLocationHazards();
}

void GameLevel::drawPlatforms()
{
    if (platforms)
    {
        platforms->draw(*window);
    }

    drawMiniLocationBarriers();
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

    drawMiniLocationHazards();
}

void GameLevel::drawEnemyManager()
{
    if (enemyManager)
    {
        enemyManager->draw_all();
    }
}

void GameLevel::drawInteractives()
{
    for (auto& interactive : interactives)
    {
        interactive->draw(*window);
    }
}

void GameLevel::updateMiniLocationHazards()
{
    if (!player || !player->isAlive || player->isPlayingDieAnimation)
    {
        return;
    }

    const sf::Vector2f feet = player->getFeetPosition();

    for (const auto& generatedLocation : generatedMiniLocations)
    {
        if (feet.x < generatedLocation.activeLeftX || feet.x > generatedLocation.activeRightX)
        {
            continue;
        }

        if (feet.y >= generatedLocation.deathY)
        {
            player->forceKill();
            return;
        }
    }
}

void GameLevel::drawMiniLocationBarriers()
{
    if (!window)
    {
        return;
    }

    const float time = miniLocationEffectsClock.getElapsedTime().asSeconds();

    for (const auto& barrier : generatedMiniBarriers)
    {
        const float height = barrier.bottomY - barrier.topY;
        if (height <= 0.f)
        {
            continue;
        }

        const float pulse = 0.5f + 0.5f * std::sin(time * 3.1f + barrier.phase);

        sf::RectangleShape outerGlow({barrier.width * 4.8f, height});
        outerGlow.setOrigin({outerGlow.getSize().x / 2.f, 0.f});
        outerGlow.setPosition({barrier.x, barrier.topY});
        outerGlow.setFillColor(withAlpha(barrier.glowColor, 34.f + 34.f * pulse));
        window->draw(outerGlow);

        sf::RectangleShape innerGlow({barrier.width * 2.4f, height});
        innerGlow.setOrigin({innerGlow.getSize().x / 2.f, 0.f});
        innerGlow.setPosition({barrier.x, barrier.topY});
        innerGlow.setFillColor(withAlpha(brighten(barrier.coreColor, 0.25f), 82.f + 48.f * pulse));
        window->draw(innerGlow);

        sf::RectangleShape core({barrier.width, height});
        core.setOrigin({core.getSize().x / 2.f, 0.f});
        core.setPosition({barrier.x, barrier.topY});
        core.setFillColor(withAlpha(brighten(barrier.coreColor, 0.42f), 176.f + 48.f * pulse));
        window->draw(core);

        for (int segmentIndex = 0; segmentIndex < 10; ++segmentIndex)
        {
            const float ratio = static_cast<float>(segmentIndex) / 9.f;
            const float y = barrier.topY
                + ratio * height
                + std::sin(time * 4.6f + barrier.phase + ratio * 9.f) * 9.f;
            const float segmentWidth = barrier.width * (
                1.18f + 0.32f * std::sin(time * 6.4f + barrier.phase * 0.7f + static_cast<float>(segmentIndex))
            );

            sf::RectangleShape segment({segmentWidth, 4.f});
            segment.setOrigin({segmentWidth / 2.f, 2.f});
            segment.setPosition({
                barrier.x + std::sin(time * 2.7f + barrier.phase + ratio * 11.f) * 7.f,
                y
            });
            segment.setFillColor(withAlpha(brighten(barrier.coreColor, 0.55f), 210.f));
            window->draw(segment);
        }

        for (const float side : std::array<float, 2>{-1.f, 1.f})
        {
            for (int sparkIndex = 0; sparkIndex < 6; ++sparkIndex)
            {
                const float ratio = static_cast<float>(sparkIndex) / 5.f;
                const float y = barrier.topY
                    + ratio * height
                    + std::sin(time * 2.2f + barrier.phase + side * 0.7f + ratio * 10.f) * 13.f;
                const float radius = 2.6f + 1.7f * (0.5f + 0.5f * std::sin(time * 5.8f + barrier.phase + sparkIndex));

                sf::CircleShape spark(radius);
                spark.setOrigin({radius, radius});
                spark.setPosition({
                    barrier.x + side * (barrier.width * 0.9f + 8.f + 5.f * std::sin(time * 3.4f + ratio * 9.f)),
                    y
                });
                spark.setFillColor(withAlpha(brighten(barrier.glowColor, 0.38f), 180.f));
                window->draw(spark);
            }
        }
    }
}

void GameLevel::drawMiniLocationHazards()
{
    if (!window)
    {
        return;
    }

    const float time = miniLocationEffectsClock.getElapsedTime().asSeconds();

    for (const auto& hazard : generatedMiniHazards)
    {
        const float width = hazard.rightX - hazard.leftX;
        const float height = hazard.bottomY - hazard.topY;
        if (width <= 0.f || height <= 0.f)
        {
            continue;
        }

        const float pulse = 0.5f + 0.5f * std::sin(time * 2.6f + hazard.phase);

        sf::RectangleShape glow({width + 96.f, height + 40.f});
        glow.setPosition({hazard.leftX - 48.f, hazard.topY - 14.f});
        glow.setFillColor(withAlpha(hazard.glowColor, 30.f + 24.f * pulse));
        window->draw(glow);

        sf::RectangleShape abyss({width, height});
        abyss.setPosition({hazard.leftX, hazard.topY});
        abyss.setFillColor(sf::Color(34, 7, 5, 220));
        window->draw(abyss);

        sf::RectangleShape moltenBand({width, std::min(44.f, height * 0.32f)});
        moltenBand.setPosition({hazard.leftX, hazard.topY});
        moltenBand.setFillColor(withAlpha(hazard.coreColor, 132.f + 38.f * pulse));
        window->draw(moltenBand);

        for (int bubbleIndex = 0; bubbleIndex < 16; ++bubbleIndex)
        {
            const float ratio = static_cast<float>(bubbleIndex) / 15.f;
            const float x = hazard.leftX
                + ratio * width
                + std::sin(time * 2.0f + hazard.phase + ratio * 8.4f) * 13.f;
            const float y = hazard.topY + 6.f + std::sin(time * 5.1f + hazard.phase + ratio * 10.2f) * 5.f;
            const float radius = 8.f + 5.f * (0.5f + 0.5f * std::sin(time * 6.0f + hazard.phase + bubbleIndex));

            sf::CircleShape bubble(radius);
            bubble.setOrigin({radius, radius});
            bubble.setPosition({x, y});
            bubble.setFillColor(withAlpha(brighten(hazard.coreColor, 0.35f), 112.f + 62.f * pulse));
            window->draw(bubble);
        }

        for (int emberIndex = 0; emberIndex < 12; ++emberIndex)
        {
            const float laneNoise = seededNoise(
                {hazard.leftX + static_cast<float>(emberIndex) * 17.f, hazard.topY},
                emberIndex,
                hazard.phase + 0.31f
            );
            const float horizontalDrift = std::sin(time * (1.3f + emberIndex * 0.08f) + hazard.phase + emberIndex) * 18.f;
            const float riseDistance = std::fmod(
                time * (76.f + emberIndex * 7.f) + hazard.phase * 48.f + static_cast<float>(emberIndex * 31),
                height + 88.f
            );
            const float x = hazard.leftX + laneNoise * width + horizontalDrift;
            const float y = hazard.bottomY - riseDistance;
            const float radius = 2.2f + 3.6f * seededNoise(
                {hazard.rightX, hazard.bottomY + static_cast<float>(emberIndex) * 9.f},
                emberIndex + 7,
                hazard.phase + 0.63f
            );

            sf::CircleShape ember(radius);
            ember.setOrigin({radius, radius});
            ember.setPosition({x, y});
            ember.setFillColor(withAlpha(hazard.emberColor, 96.f + 84.f * pulse));
            window->draw(ember);
        }
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
        const std::string backgroundTheme = data.contains("Presets")
            ? data["Presets"].value("BackgroundTheme", std::string{})
            : std::string{};
        const auto& backgroundLayers = data["Background"];
        const std::size_t layerCount = backgroundLayers.size();

        std::size_t layerIndex = 0;
        for (const auto& backgroundData : backgroundLayers)
        {
            const sf::Vector2f position = {backgroundData["Position"][0], backgroundData["Position"][1]};
            const sf::Vector2f parallaxFactor = {
                backgroundData["ParallaxFactor"][0],
                backgroundData["ParallaxFactor"][1]
            };
            const std::string name = backgroundData["BgName"];
            const Type type = backgroundData["Type"];
            BackgroundSceneConfig sceneConfig;
            sceneConfig.themeName = backgroundData.value("Theme", backgroundTheme);
            sceneConfig.layerIndex = layerIndex;
            sceneConfig.layerCount = layerCount;

            background.push_back(
                std::make_shared<Background>(
                    *this->data,
                    *this->camera,
                    *this,
                    position,
                    name,
                    parallaxFactor,
                    type,
                    sceneConfig
                )
            );

            ++layerIndex;
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
        const float offset = groundData.contains("Offset")
            ? groundData["Offset"].get<float>()
            : BASE_GROUND_OFFSET;
        const unsigned int depthRows = groundData.contains("DepthRows")
            ? groundData["DepthRows"].get<unsigned int>()
            : 0u;
        const std::string groundStyle = groundData.value("GroundStyle", std::string{});

        ground = std::make_shared<Ground>(
            *this->data,
            *this,
            groundName,
            position.x,
            position.y,
            yPos,
            offset
        );

        ground->setVisualDepthRows(depthRows);
        if (!groundStyle.empty())
        {
            ground->setStyle(groundStyle);
        }
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

void GameLevel::initializeInteractives(const nlohmann::json& data)
{
    interactives.clear();

    if (!player)
    {
        return;
    }

    const auto parseColor = [](const nlohmann::json& object, const char* key, const sf::Color& fallback) {
        if (!object.contains(key))
        {
            return fallback;
        }

        const auto& color = object[key];
        return sf::Color{
            color[0].get<std::uint8_t>(),
            color[1].get<std::uint8_t>(),
            color[2].get<std::uint8_t>(),
            color[3].get<std::uint8_t>()
        };
    };

    if (data.contains("Interactives"))
    {
        for (const auto& interactiveData : data["Interactives"])
        {
            WorldInteractable::Config config;

            const std::string typeName = interactiveData.value("Type", "EchoTablet");
            if (typeName == "RestShrine")
            {
                config.type = WorldInteractable::Type::RestShrine;
            }
            else if (typeName == "GoldCache")
            {
                config.type = WorldInteractable::Type::GoldCache;
            }

            config.textureName = interactiveData.value("Texture", std::string{});
            config.position = {
                interactiveData["Position"][0].get<float>(),
                interactiveData["Position"][1].get<float>()
            };
            config.scale = interactiveData.contains("Scale")
                ? sf::Vector2f{
                    interactiveData["Scale"][0].get<float>(),
                    interactiveData["Scale"][1].get<float>()
                }
                : sf::Vector2f{1.f, 1.f};
            config.color = parseColor(interactiveData, "Color", sf::Color::White);
            config.accentColor = parseColor(interactiveData, "AccentColor", sf::Color(220, 184, 122, 255));
            config.interactRadius = interactiveData.value("InteractRadius", 120.f);
            config.goldReward = interactiveData.value("RewardGold", 0);
            config.singleUse = interactiveData.value("SingleUse", true);
            config.grantsCheckpoint = interactiveData.value("GrantsCheckpoint", config.type == WorldInteractable::Type::RestShrine);
            config.restoreVitality = interactiveData.value("RestoreVitality", config.type == WorldInteractable::Type::RestShrine);
            config.prompt = interactiveData.value("Prompt", std::string{"Enter to interact"});
            config.title = interactiveData.value("Title", std::string{"Forgotten Relic"});
            config.body = interactiveData.value("Body", std::string{"The dead left a trace here."});

            if (interactiveData.contains("SpawnOffset"))
            {
                config.hasCustomSpawnOffset = true;
                config.spawnOffset = {
                    interactiveData["SpawnOffset"][0].get<float>(),
                    interactiveData["SpawnOffset"][1].get<float>()
                };
            }

            interactives.push_back(std::make_unique<WorldInteractable>(
                *this->data,
                *this->camera,
                *this->levelManager,
                *player,
                config
            ));
        }
    }

    for (const auto& generatedLocation : generatedMiniLocations)
    {
        MiniLocationEntrance::Config entranceConfig;
        entranceConfig.textureName = generatedLocation.entranceTexture;
        entranceConfig.position = generatedLocation.entrancePosition;
        entranceConfig.scale = {0.32f, 0.32f};
        entranceConfig.destinationSupportPoint = generatedLocation.entranceDestinationSupport;
        entranceConfig.color = generatedLocation.entranceColor;
        entranceConfig.accentColor = generatedLocation.accentColor;
        entranceConfig.interactRadius = generatedLocation.interactRadius;
        entranceConfig.prompt = generatedLocation.entrancePrompt;
        entranceConfig.subtitle = generatedLocation.title;

        interactives.push_back(std::make_unique<MiniLocationEntrance>(
            *this->data,
            *this->camera,
            *player,
            entranceConfig
        ));

        MiniLocationEntrance::Config exitConfig;
        exitConfig.textureName = generatedLocation.exitTexture;
        exitConfig.position = generatedLocation.exitPosition;
        exitConfig.scale = {0.26f, 0.26f};
        exitConfig.destinationSupportPoint = generatedLocation.exitDestinationSupport;
        exitConfig.color = generatedLocation.exitColor;
        exitConfig.accentColor = generatedLocation.accentColor;
        exitConfig.interactRadius = generatedLocation.interactRadius;
        exitConfig.prompt = generatedLocation.exitPrompt;
        exitConfig.subtitle = "Return to the trail";

        interactives.push_back(std::make_unique<MiniLocationEntrance>(
            *this->data,
            *this->camera,
            *player,
            exitConfig
        ));
    }

    for (const auto& generatedReward : generatedMiniRewards)
    {
        WorldInteractable::Config config;

        if (generatedReward.typeName == "RestShrine")
        {
            config.type = WorldInteractable::Type::RestShrine;
        }
        else if (generatedReward.typeName == "GoldCache")
        {
            config.type = WorldInteractable::Type::GoldCache;
        }
        else
        {
            config.type = WorldInteractable::Type::EchoTablet;
        }

        config.textureName = generatedReward.textureName;
        config.position = generatedReward.position;
        config.scale = generatedReward.scale;
        config.color = generatedReward.color;
        config.accentColor = generatedReward.accentColor;
        config.interactRadius = generatedReward.interactRadius;
        config.goldReward = generatedReward.goldReward;
        config.singleUse = generatedReward.singleUse;
        config.grantsCheckpoint = generatedReward.grantsCheckpoint;
        config.restoreVitality = generatedReward.restoreVitality;
        config.hasCustomSpawnOffset = generatedReward.hasCustomSpawnOffset;
        config.spawnOffset = generatedReward.spawnOffset;
        config.prompt = generatedReward.prompt;
        config.title = generatedReward.title;
        config.body = generatedReward.body;

        interactives.push_back(std::make_unique<WorldInteractable>(
            *this->data,
            *this->camera,
            *this->levelManager,
            *player,
            config
        ));
    }
}

void GameLevel::generateMiniLocations()
{
    generatedMiniLocations.clear();
    generatedMiniRewards.clear();
    generatedMiniBarriers.clear();
    generatedMiniHazards.clear();

    if (!platforms || !decorations || !ground || loadedLevelData.is_null())
    {
        return;
    }

    const float baseWidth = static_cast<float>(size.x);
    const float groundY = ground->getRect().getPosition().y;
    const sf::Vector2f spawnPos = {
        loadedLevelData["Presets"]["PlayerSpawn"][0].get<float>(),
        loadedLevelData["Presets"]["PlayerSpawn"][1].get<float>()
    };

    std::vector<sf::Vector2f> reservedPositions;
    reservedPositions.push_back(spawnPos);

    if (loadedLevelData.contains("Interactives"))
    {
        for (const auto& interactiveData : loadedLevelData["Interactives"])
        {
            reservedPositions.push_back({
                interactiveData["Position"][0].get<float>(),
                interactiveData["Position"][1].get<float>()
            });
        }
    }

    std::vector<MiniLocationCandidate> candidates;
    const auto& platformRects = platforms->getRects();

    const auto tryAddCandidate = [&](MiniLocationCandidate candidate) {
        candidate.centerX = std::clamp(candidate.centerX, 220.f, baseWidth - 220.f);
        candidate.leftX = std::clamp(candidate.leftX, 0.f, baseWidth);
        candidate.rightX = std::clamp(candidate.rightX, 0.f, baseWidth);

        if (candidate.rightX - candidate.leftX < 120.f)
        {
            return;
        }

        if (std::abs(candidate.centerX - spawnPos.x) < 440.f)
        {
            return;
        }

        for (const sf::Vector2f& reserved : reservedPositions)
        {
            if (std::abs(candidate.centerX - reserved.x) < 340.f
                && std::abs(candidate.surfaceY - reserved.y) < 240.f)
            {
                return;
            }
        }

        if (!hasEntranceClearance(candidate, platformRects))
        {
            return;
        }

        candidates.push_back(candidate);
    };

    for (const auto& platformRect : platformRects)
    {
        const sf::FloatRect bounds = platformRect->getGlobalBounds();
        if (bounds.size.x < 145.f || bounds.size.y > 95.f || bounds.size.x < bounds.size.y)
        {
            continue;
        }

        tryAddCandidate({
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y,
            bounds.position.x + 20.f,
            bounds.position.x + bounds.size.x - 20.f
        });
    }

    constexpr std::array<float, 5> kGroundFractions{0.18f, 0.36f, 0.54f, 0.72f, 0.84f};
    for (const float fraction : kGroundFractions)
    {
        const float center = std::clamp(baseWidth * fraction + randomFloat(-170.f, 170.f), 220.f, baseWidth - 220.f);
        tryAddCandidate({center, groundY, center - 128.f, center + 128.f});
    }

    std::shuffle(candidates.begin(), candidates.end(), miniLocationRng());

    const int desiredCount = std::clamp(static_cast<int>(std::round(baseWidth / 3600.f)), 1, 3);
    const float minSpacing = std::max(860.f, baseWidth * 0.13f);

    std::vector<MiniLocationCandidate> selectedCandidates;
    for (const auto& candidate : candidates)
    {
        const bool overlapsSelected = std::any_of(selectedCandidates.begin(), selectedCandidates.end(),
            [&](const MiniLocationCandidate& selected) {
                return std::abs(candidate.centerX - selected.centerX) < minSpacing;
            });
        if (overlapsSelected)
        {
            continue;
        }

        selectedCandidates.push_back(candidate);
        if (static_cast<int>(selectedCandidates.size()) >= desiredCount)
        {
            break;
        }
    }

    if (selectedCandidates.empty())
    {
        const float fallbackCenter = std::clamp(baseWidth * 0.62f, 240.f, baseWidth - 240.f);
        selectedCandidates.push_back({fallbackCenter, groundY, fallbackCenter - 128.f, fallbackCenter + 128.f});
    }

    std::sort(selectedCandidates.begin(), selectedCandidates.end(), [](const MiniLocationCandidate& lhs, const MiniLocationCandidate& rhs) {
        return lhs.centerX < rhs.centerX;
    });

    constexpr float kPocketWorldGap = 1860.f;
    constexpr float kPocketRoomSpacing = 1040.f;
    constexpr float kPocketSealOverflow = 180.f;
    constexpr float kPocketWorldSealInset = 92.f;
    const float worldSealX = std::max(0.f, baseWidth - kPocketWorldSealInset);
    const float worldSealTop = 0.f;
    const float worldSealBottom = static_cast<float>(size.y) + kPocketSealOverflow;

    addMiniLocationSeal(
        *platforms,
        worldSealX,
        worldSealTop,
        worldSealBottom
    );
    generatedMiniBarriers.push_back({
        worldSealX + 36.f,
        worldSealTop,
        worldSealBottom,
        24.f,
        randomFloat(0.f, 6.28318f),
        sf::Color(136, 232, 220, 255),
        sf::Color(102, 198, 192, 255)
    });

    float extensionCursor = baseWidth + kPocketWorldGap;
    const auto& themes = getMiniLocationThemes();

    for (std::size_t index = 0; index < selectedCandidates.size(); ++index)
    {
        const MiniLocationCandidate& candidate = selectedCandidates[index];
        const MiniLocationTheme& theme = pickRandom(themes);

        const float roomWidth = randomFloat(960.f, 1180.f);
        const float roomFloorY = std::clamp(groundY - randomFloat(86.f, 124.f), 840.f, 942.f);
        const float roomHeight = randomFloat(250.f, 320.f);
        const float roomX = extensionCursor;
        const float roomCenterX = roomX + roomWidth * 0.5f;
        const float roomCeilingY = roomFloorY - roomHeight;
        const float shelfY = roomFloorY - randomFloat(150.f, 174.f);
        const float roomLeftSealX = roomX - 102.f;
        const float roomRightSealX = roomX + roomWidth + 32.f;
        const float roomSealTop = 0.f;
        const float roomSealBottom = static_cast<float>(size.y) + kPocketSealOverflow;
        const float deathY = std::min(static_cast<float>(size.y) - 24.f, roomFloorY + randomFloat(152.f, 184.f));
        const float hazardTopY = std::max(roomFloorY + 88.f, deathY - 20.f);
        const float hazardBottomY = static_cast<float>(size.y) + 56.f;

        extensionCursor += roomWidth + kPocketRoomSpacing + randomFloat(80.f, 180.f);
        size.x = std::max(size.x, static_cast<int>(std::ceil(roomX + roomWidth + 420.f)));

        addMiniLocationSeal(
            *platforms,
            roomLeftSealX,
            roomSealTop,
            roomSealBottom
        );
        addMiniLocationSeal(
            *platforms,
            roomRightSealX,
            roomSealTop,
            roomSealBottom
        );
        generatedMiniBarriers.push_back({
            roomLeftSealX + 36.f,
            roomSealTop,
            roomSealBottom,
            22.f,
            randomFloat(0.f, 6.28318f),
            theme.accentColor,
            brighten(theme.entranceColor, 0.18f)
        });
        generatedMiniBarriers.push_back({
            roomRightSealX + 36.f,
            roomSealTop,
            roomSealBottom,
            22.f,
            randomFloat(0.f, 6.28318f),
            theme.exitColor,
            brighten(theme.accentColor, 0.24f)
        });
        generatedMiniHazards.push_back({
            roomX - 8.f,
            roomX + roomWidth + 56.f,
            hazardTopY,
            hazardBottomY,
            randomFloat(0.f, 6.28318f),
            sf::Color(242, 104, 56, 255),
            sf::Color(255, 182, 96, 255),
            sf::Color(255, 236, 188, 255)
        });

        for (float floorX = roomX + 28.f; floorX <= roomX + roomWidth - 330.f; floorX += 320.f)
        {
            platforms->addPlatform({floorX, roomFloorY}, "Quadruple");
        }
        platforms->addPlatform({roomX + roomWidth - 308.f, roomFloorY}, "Quadruple");
        platforms->addPlatform({roomX + 18.f, roomFloorY - 142.f}, "Crypt-pillar");
        platforms->addPlatform({roomX + roomWidth - 78.f, roomFloorY - 142.f}, "Crypt-pillar");
        platforms->addPlatform({roomCenterX - 166.f, roomCeilingY + 34.f}, "Fallen-arcade");
        platforms->addPlatform({roomCenterX - 135.f, shelfY}, "Triple");
        if (index % 2 == 0)
        {
            platforms->addPlatform({roomX + 136.f, roomFloorY - 96.f}, "Double-horizontal-1");
        }
        else
        {
            platforms->addPlatform({roomX + roomWidth - 252.f, roomFloorY - 98.f}, "Double-horizontal-2");
        }

        decorations->addDecoration(
            theme.backdropDecoration,
            {roomCenterX, roomFloorY - 118.f},
            {1.04f, 1.04f},
            {0.12f, 0.10f},
            -4,
            sf::Color(255, 255, 255, 96)
        );
        decorations->addDecoration(
            theme.hillDecoration,
            {roomCenterX, roomFloorY + 34.f},
            {0.84f, 0.84f},
            {0.18f, 0.14f},
            -3,
            sf::Color(176, 198, 204, 162)
        );
        decorations->addDecoration(
            theme.hangingDecoration,
            {roomCenterX, roomCeilingY + 18.f},
            {0.58f, 0.58f},
            {0.32f, 0.28f},
            -1,
            sf::Color(170, 214, 188, 204)
        );
        decorations->addDecoration(
            theme.ambientDecoration,
            {roomCenterX - 132.f, roomFloorY + 62.f},
            {0.34f, 0.34f},
            {0.68f, 0.68f},
            2,
            sf::Color(236, 255, 244, 236)
        );
        decorations->addDecoration(
            theme.ambientDecoration,
            {roomCenterX + 146.f, roomFloorY + 60.f},
            {0.28f, 0.28f},
            {0.70f, 0.70f},
            2,
            sf::Color(236, 255, 244, 220)
        );
        decorations->addDecoration(
            "MossyDecorationHazard_03.png",
            {roomX + 86.f, roomFloorY - 10.f},
            {0.22f, 0.22f},
            {0.54f, 0.50f},
            1,
            sf::Color(255, 255, 255, 220)
        );
        decorations->addDecoration(
            "MossyDecorationHazard_19.png",
            {roomX + roomWidth - 80.f, roomFloorY - 6.f},
            {0.20f, 0.20f},
            {0.54f, 0.50f},
            1,
            sf::Color(255, 255, 255, 216)
        );

        const float returnDirection = candidate.centerX < baseWidth * 0.72f ? 1.f : -1.f;
        const float returnSupportX = std::clamp(
            candidate.centerX + returnDirection * 88.f,
            candidate.leftX + 28.f,
            candidate.rightX - 28.f
        );

        generatedMiniLocations.push_back({
            theme.title,
            theme.entranceTexture,
            theme.exitTexture,
            {candidate.centerX, candidate.surfaceY + 18.f},
            {roomX + 198.f, roomFloorY},
            {roomX + 110.f, roomFloorY + 18.f},
            {returnSupportX, candidate.surfaceY},
            theme.entranceColor,
            theme.exitColor,
            theme.accentColor,
            126.f,
            theme.entrancePrompt,
            theme.exitPrompt,
            roomX - 8.f,
            roomX + roomWidth + 56.f,
            roomLeftSealX - 28.f,
            roomRightSealX + 28.f,
            roomCeilingY,
            roomFloorY,
            deathY
        });

        GeneratedMiniReward reward;
        reward.typeName = theme.rewardTypeName;
        reward.textureName = theme.rewardTexture;
        reward.position = {roomCenterX + randomFloat(-12.f, 12.f), shelfY - 8.f};
        reward.scale = theme.rewardScale;
        reward.color = sf::Color::White;
        reward.accentColor = theme.accentColor;
        reward.interactRadius = 118.f;
        reward.goldReward = theme.goldReward;
        reward.singleUse = true;
        reward.grantsCheckpoint = theme.grantsCheckpoint;
        reward.restoreVitality = theme.restoreVitality;
        reward.prompt = theme.rewardPrompt;
        reward.title = theme.rewardTitle;
        reward.body = theme.rewardBody;
        if (theme.grantsCheckpoint)
        {
            reward.hasCustomSpawnOffset = true;
            reward.spawnOffset = {72.f, -126.f};
        }
        generatedMiniRewards.push_back(reward);
    }
}

void GameLevel::tryInitializeEnemyManager()
{
    if (player && !loadedLevelData.is_null())
    {
        initializeEnemyManager(loadedLevelData);
    }
}

void GameLevel::tryInitializeInteractives()
{
    if (player && !loadedLevelData.is_null())
    {
        initializeInteractives(loadedLevelData);
    }
}

void GameLevel::draw()
{
    drawBackgrounds();
    drawDecorations();
    drawGrounds();
    drawInteractives();
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
    primaryWorldWidth = size.x;
    primaryWorldCameraRightEdge = static_cast<float>(primaryWorldWidth);
    levelName = fileNamePath;

    initializePlatforms(loadedLevelData);
    initializeDecorations(loadedLevelData);
    initializeBackground(loadedLevelData);
    initializeGround(loadedLevelData);
    if (ground)
    {
        primaryWorldCameraRightEdge = ground->getCameraClampRight();
    }
    generateMiniLocations();

    enemyManager.reset();
    tryInitializeEnemyManager();
    tryInitializeInteractives();

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
    interactives.clear();
    generatedMiniLocations.clear();
    generatedMiniRewards.clear();
    generatedMiniBarriers.clear();
    generatedMiniHazards.clear();

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

sf::FloatRect GameLevel::getCameraBoundsForPosition(const sf::Vector2f& position) const
{
    for (const auto& generatedLocation : generatedMiniLocations)
    {
        if (position.x >= generatedLocation.activeLeftX && position.x <= generatedLocation.activeRightX)
        {
            return sf::FloatRect(
                {generatedLocation.activeLeftX, 0.f},
                {generatedLocation.activeRightX - generatedLocation.activeLeftX, static_cast<float>(size.y)}
            );
        }
    }

    const float mainWorldRight = std::max(
        primaryWorldCameraRightEdge > 0.f ? primaryWorldCameraRightEdge : static_cast<float>(primaryWorldWidth),
        0.f
    );
    return sf::FloatRect({0.f, 0.f}, {mainWorldRight, static_cast<float>(size.y)});
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

bool GameLevel::handleEvent(const sf::Event& event)
{
    for (auto& interactive : interactives)
    {
        if (interactive->blocksPlayerInput())
        {
            interactive->handleEvent(event);
            return true;
        }
    }

    for (auto& interactive : interactives)
    {
        if (interactive->handleEvent(event))
        {
            return true;
        }
    }

    return false;
}

bool GameLevel::hasBlockingInteractiveModal() const
{
    return std::any_of(interactives.begin(), interactives.end(), [](const std::unique_ptr<InteractiveObject>& interactive) {
        return interactive && interactive->blocksPlayerInput();
    });
}

void GameLevel::setPlayerSpawnPos(const sf::Vector2f& pos)
{
    playerSpawnPos = pos;
}

void GameLevel::attachPlayer(Player& p)
{
    player = &p;

    if (enemyManager)
    {
        enemyManager->attachPlayer(p);
    }
    else
    {
        tryInitializeEnemyManager();
    }

    tryInitializeInteractives();
}
