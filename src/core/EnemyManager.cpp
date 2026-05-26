#include "EnemyManager.h"

#include <Spawner.h>
#include <Skeleton.h>
#include <BestiaryEnemy.h>
#include <Bullet.h>
#include <enemyPortal.h>
#include <GameLevel.h>

#include <cmath>

namespace
{
void applySpawnerArchetype(
    const std::string& archetype,
    int& enemyAmount,
    int& spawnCooldown,
    int& enemyPerSpawn)
{
    if (archetype == "Ambush")
    {
        enemyPerSpawn = std::max(enemyPerSpawn + 1, 2);
        spawnCooldown = std::max(1600, spawnCooldown - 1800);
    }
    else if (archetype == "Siege")
    {
        enemyAmount += 2;
        spawnCooldown = std::max(2200, spawnCooldown - 900);
    }
    else if (archetype == "Swarm")
    {
        enemyAmount += 3;
        enemyPerSpawn = std::max(enemyPerSpawn + 2, 3);
        spawnCooldown = std::max(1400, spawnCooldown - 2400);
    }
    else if (archetype == "Duel")
    {
        enemyPerSpawn = 1;
        spawnCooldown = std::max(2800, spawnCooldown + 600);
    }

    enemyAmount = std::max(enemyAmount, 0);
    enemyPerSpawn = std::max(enemyPerSpawn, 1);
    spawnCooldown = std::max(spawnCooldown, 900);
}

sf::FloatRect readRect(const nlohmann::json& value, const sf::FloatRect fallback = sf::FloatRect({0.f, 0.f}, {0.f, 0.f}))
{
    if (!value.is_array() || value.size() < 4)
    {
        return fallback;
    }
    return {{value[0].get<float>(), value[1].get<float>()}, {value[2].get<float>(), value[3].get<float>()}};
}
}

void EnemyManager::updateSpawner()
{
    for (Spawner& spawner : spawners)
    {
        spawner.update();
    }
}

void EnemyManager::removeIfNotAlive()
{
    skeletons.erase(
        std::remove_if(skeletons.begin(), skeletons.end(),
            [](auto& enemy) {
                return !enemy->isAlive;
            }),
        skeletons.end()
    );

    bestiaryEnemies.erase(
        std::remove_if(bestiaryEnemies.begin(), bestiaryEnemies.end(),
            [](auto& enemy) {
                return !enemy->isAlive;
            }),
        bestiaryEnemies.end()
    );
}

void EnemyManager::loadSpawnerData()
{
    for (const auto& spawner : (*data)["Spawners"])
    {
        int enemyAmount = spawner["EnemyAmount"];
        int spawnCooldown = spawner["SpawnCooldown"];
        int enemyPerSpawn = spawner["EnemyPerSpawn"];
        applySpawnerArchetype(
            spawner.value("Archetype", std::string{}),
            enemyAmount,
            spawnCooldown,
            enemyPerSpawn
        );

        if (enemyAmount <= 0)
        {
            continue;
        }

        sf::Vector2f spawnArea[2] = {
            {spawner["SpawnArea"][0][0], spawner["SpawnArea"][0][1]},
            {spawner["SpawnArea"][1][0], spawner["SpawnArea"][1][1]}
        };

        spawners.emplace_back(
            *this,
            *this->gameLevel,
            spawner["EnemyName"],
            enemyAmount,
            spawnCooldown,
            enemyPerSpawn,
            spawnArea,
            *this->gameData,
            *this->platform,
            *this->ground,
            *this->player,
            *this->window,
            Spawner::EncounterConfig{
                spawner.value("ActivationMode", std::string{}) == "OnEnter",
                spawner.value("FirstSpawnDelayMs", 0),
                spawner.value("ActivationPadding", 120.f),
                spawner.value("EnemyHP", 0),
                spawner.value("EnemyDamage", 0),
                spawner.value("GoldReward", 0),
                spawner.contains("ActivationArea") && spawner["ActivationArea"].is_array(),
                readRect(spawner.value("ActivationArea", nlohmann::json::array()))
            }
        );
    }
}

void EnemyManager::updateCoins()
{
    if (!ground || !platform || !player || !gameLevel)
    {
        return;
    }

    const sf::FloatRect levelBounds = {
        {0.f, 0.f},
        static_cast<sf::Vector2f>(gameLevel->getLevelSize())
    };

    for (auto&& coin : coins)
    {
        coin.update(*ground, *platform, *player, levelBounds);
    }

    coins.erase(
        std::remove_if(coins.begin(), coins.end(),
            [](const GoldCoin& coin) {
                return coin.isCollected();
            }),
        coins.end()
    );
}

std::shared_ptr<Skeleton> EnemyManager::addSkeleton(
    GameData& data,
    sf::RenderWindow& window,
    Ground& ground,
    Platform& platform,
    Player& player,
    std::string type,
    sf::Vector2f pos)
{
    auto enemy = std::make_shared<Skeleton>(data, *this, *this->gameLevel, window, ground, platform, player, type, pos);
    skeletons.push_back(enemy);
    return enemy;
}

std::shared_ptr<BestiaryEnemy> EnemyManager::addBestiaryEnemy(
    GameData& data,
    sf::RenderWindow& window,
    Ground& ground,
    Platform& platform,
    Player& player,
    std::string type,
    sf::Vector2f pos)
{
    auto enemy = std::make_shared<BestiaryEnemy>(data, *this, *this->gameLevel, window, ground, platform, player, type, pos);
    bestiaryEnemies.push_back(enemy);
    return enemy;
}

void EnemyManager::dropGold(const sf::Vector2f& position, const std::string& enemyType)
{
    int coinCount = random(3, 5);
    int minValue = 4;
    int maxValue = 7;

    if (enemyType == "yellow")
    {
        coinCount = random(4, 6);
        minValue = 6;
        maxValue = 10;
    }
    else if (enemyType == "wraith-bat")
    {
        coinCount = random(4, 5);
        minValue = 6;
        maxValue = 9;
    }
    else if (enemyType == "void-slime")
    {
        coinCount = random(4, 6);
        minValue = 7;
        maxValue = 11;
    }
    else if (enemyType == "dread-scorpion")
    {
        coinCount = random(5, 7);
        minValue = 8;
        maxValue = 12;
    }

    for (int index = 0; index < coinCount; ++index)
    {
        coins.emplace_back(
            sf::Vector2f{position.x, position.y - random(4.f, 16.f)},
            random(minValue, maxValue),
            sf::Vector2f{random(-2.3f, 2.3f), random(-4.8f, -2.6f)}
        );
    }
}

void EnemyManager::dropGoldAmount(const sf::Vector2f& position, int totalGold)
{
    totalGold = std::max(totalGold, 0);
    if (totalGold <= 0)
    {
        return;
    }

    const int coinCount = std::clamp(totalGold / 6 + 1, 1, 8);
    int remaining = totalGold;
    for (int index = 0; index < coinCount; ++index)
    {
        const int slotsLeft = coinCount - index;
        const int value = index == coinCount - 1
            ? remaining
            : std::max(1, remaining / slotsLeft);
        remaining -= value;
        coins.emplace_back(
            sf::Vector2f{position.x, position.y - random(4.f, 16.f)},
            value,
            sf::Vector2f{random(-2.3f, 2.3f), random(-4.8f, -2.6f)}
        );
    }
}

void EnemyManager::applySplashDamage(const sf::Vector2f& impactCenter, float splashRadius, const Bullet& bullet, const void* ignoredTarget)
{
    for (const auto& otherSkeleton : skeletons)
    {
        if (!otherSkeleton || !otherSkeleton->isAlive || otherSkeleton.get() == ignoredTarget)
        {
            continue;
        }

        const sf::Vector2f delta = otherSkeleton->getRect().getGlobalBounds().getCenter() - impactCenter;
        const float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (distance <= splashRadius)
        {
            otherSkeleton->receiveBulletHit(bullet, true);
        }
    }

    for (const auto& bestiaryEnemy : bestiaryEnemies)
    {
        if (!bestiaryEnemy || !bestiaryEnemy->isAlive || bestiaryEnemy.get() == ignoredTarget)
        {
            continue;
        }

        const sf::Vector2f delta = bestiaryEnemy->getRect().getGlobalBounds().getCenter() - impactCenter;
        const float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (distance <= splashRadius)
        {
            bestiaryEnemy->receiveBulletHit(bullet, true);
        }
    }
}

void EnemyManager::killEnemiesInRect(const sf::FloatRect& hazardRect)
{
    for (const auto& skeleton : skeletons)
    {
        if (skeleton && skeleton->isAlive && skeleton->getRect().getGlobalBounds().findIntersection(hazardRect))
        {
            skeleton->isAlive = false;
        }
    }

    for (const auto& bestiaryEnemy : bestiaryEnemies)
    {
        if (bestiaryEnemy && bestiaryEnemy->isAlive && bestiaryEnemy->getRect().getGlobalBounds().findIntersection(hazardRect))
        {
            bestiaryEnemy->isAlive = false;
        }
    }
}

void EnemyManager::updateAI_all()
{
    for (auto&& enemy : skeletons)
    {
        enemy->updateAI();
    }

    for (auto&& enemy : bestiaryEnemies)
    {
        enemy->updateAI();
    }
}

void EnemyManager::updateControls_all()
{
    for (auto&& enemy : skeletons)
    {
        enemy->updateControl();
    }

    for (auto&& enemy : bestiaryEnemies)
    {
        enemy->updateControl();
    }
}

void EnemyManager::updatePhysics_all()
{
    for (auto&& enemy : skeletons)
    {
        enemy->updatePhysics();
    }

    for (auto&& enemy : bestiaryEnemies)
    {
        enemy->updatePhysics();
    }

    updateCoins();
}

void EnemyManager::updateTextures_all()
{
    for (auto&& enemy : skeletons)
    {
        enemy->updateTextures();
    }

    for (auto&& enemy : bestiaryEnemies)
    {
        enemy->updateTextures();
    }

    removeIfNotAlive();
}

void EnemyManager::updateSpawners_all()
{
    for (auto&& spawner : spawners)
    {
        spawner.update();
    }
}

void EnemyManager::draw_all()
{
    for (auto&& enemy : skeletons)
    {
        enemy->draw();
    }

    for (auto&& enemy : bestiaryEnemies)
    {
        enemy->draw();
    }

    for (auto&& coin : coins)
    {
        coin.draw(*window);
    }
}

void EnemyManager::clearEnemies()
{
    skeletons.clear();
    bestiaryEnemies.clear();
    spawners.clear();
    coins.clear();
}

void EnemyManager::addSpawner(std::string enemyName)
{
}

EnemyManager::EnemyManager(
    const nlohmann::json& d,
    GameData& gd,
    GameLevel& gl,
    Platform& p,
    Ground& g,
    Player& pl,
    sf::RenderWindow& w)
    : data(&d)
    , gameData(&gd)
    , platform(&p)
    , ground(&g)
    , player(&pl)
    , window(&w)
{
    this->gameLevel = &gl;
    loadSpawnerData();
}

EnemyManager::~EnemyManager()
{
}

void EnemyManager::attachPlayer(Player& p)
{
    this->player = &p;

    for (auto&& spawner : spawners)
    {
        spawner.attachPlayer(p);
    }

    for (auto&& skeleton : skeletons)
    {
        skeleton->attachPlayer(p);
    }

    for (auto&& enemy : bestiaryEnemies)
    {
        enemy->attachPlayer(p);
    }
}

const std::vector<std::shared_ptr<Skeleton>>& EnemyManager::getSkeletons() const
{
    return skeletons;
}

const std::vector<std::shared_ptr<BestiaryEnemy>>& EnemyManager::getBestiaryEnemies() const
{
    return bestiaryEnemies;
}
