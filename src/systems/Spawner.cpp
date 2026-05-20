#include "Spawner.h"

#include <EnemyManager.h>
#include <GameLevel.h>

namespace
{
sf::FloatRect makeActivationBounds(const sf::Vector2f spawnArea[2])
{
    const float left = std::min(spawnArea[0].x, spawnArea[0].y);
    const float right = std::max(spawnArea[0].x, spawnArea[0].y);
    const float top = std::min(spawnArea[1].x, spawnArea[1].y);
    const float bottom = std::max(spawnArea[1].x, spawnArea[1].y);

    return sf::FloatRect(
        {left - 120.f, top - 120.f},
        {std::max(0.f, right - left) + 240.f, std::max(0.f, bottom - top) + 240.f}
    );
}
}

Spawner::Spawner(
    EnemyManager& m,
    GameLevel& gl,
    std::string n,
    int ea,
    int sc,
    int eps,
    float minX1,
    float maxX2,
    float minY1,
    float maxY2,
    GameData& d,
    Platform& p,
    Ground& g,
    Player& pl,
    sf::RenderWindow& w,
    EncounterConfig encounterConfig)
    : manager(&m)
    , data(&d)
    , gameLevel(&gl)
    , platform(&p)
    , ground(&g)
    , player(&pl)
    , window(&w)
    , enemyName(std::move(n))
    , spawnArea{{minX1, maxX2}, {minY1, maxY2}}
    , enemyAmount(ea)
    , enemyPerSpawn(eps)
    , spawnCooldown(sc)
    , activateOnPlayerEnter_(encounterConfig.activateOnPlayerEnter)
    , activated_(!encounterConfig.activateOnPlayerEnter)
    , firstSpawnPending_(encounterConfig.initialSpawnDelayMs > 0)
    , initialSpawnDelayMs_(std::max(encounterConfig.initialSpawnDelayMs, 0))
    , activationBounds_(makeActivationBounds(spawnArea))
{
    if (activated_)
    {
        runSpawnCooldownClockIfNotRunning();
    }
}

Spawner::Spawner(
    EnemyManager& m,
    GameLevel& gl,
    std::string n,
    int ea,
    int sc,
    int eps,
    sf::Vector2f sa[2],
    GameData& d,
    Platform& p,
    Ground& g,
    Player& pl,
    sf::RenderWindow& w,
    EncounterConfig encounterConfig)
    : manager(&m)
    , data(&d)
    , gameLevel(&gl)
    , platform(&p)
    , ground(&g)
    , player(&pl)
    , window(&w)
    , enemyName(std::move(n))
    , spawnArea{sa[0], sa[1]}
    , enemyAmount(ea)
    , enemyPerSpawn(eps)
    , spawnCooldown(sc)
    , activateOnPlayerEnter_(encounterConfig.activateOnPlayerEnter)
    , activated_(!encounterConfig.activateOnPlayerEnter)
    , firstSpawnPending_(encounterConfig.initialSpawnDelayMs > 0)
    , initialSpawnDelayMs_(std::max(encounterConfig.initialSpawnDelayMs, 0))
    , activationBounds_(makeActivationBounds(spawnArea))
{
    if (activated_)
    {
        runSpawnCooldownClockIfNotRunning();
    }
}

Spawner::~Spawner() = default;

void Spawner::update()
{
    if (isEmpty)
    {
        return;
    }

    if (!activated_)
    {
        if (activateOnPlayerEnter_ && isPlayerInsideActivationBounds())
        {
            activated_ = true;
            spawnCooldownTimer.restart();
            if (initialSpawnDelayMs_ <= 0)
            {
                spawnCountOfEnemies();
                firstSpawnPending_ = false;
            }
        }
        else
        {
            return;
        }
    }

    const int currentInterval = firstSpawnPending_ ? initialSpawnDelayMs_ : spawnCooldown;
    if (currentInterval <= 0)
    {
        spawnCountOfEnemies();
        firstSpawnPending_ = false;
        return;
    }

    if (checkInterval(spawnCooldownTimer, currentInterval))
    {
        spawnCountOfEnemies();
        firstSpawnPending_ = false;
    }
}

void Spawner::attachPlayer(Player& p)
{
    this->player = &p;
}

void Spawner::runSpawnCooldownClockIfNotRunning()
{
    if (!spawnCooldownTimer.isRunning())
    {
        spawnCooldownTimer.restart();
    }
}

bool Spawner::isPlayerInsideActivationBounds() const
{
    return player != nullptr && activationBounds_.contains(player->getCenterPosition());
}

void Spawner::spawnCountOfEnemies()
{
    if (isEmpty)
    {
        return;
    }

    const auto spawnEnemyAt = [&](const sf::Vector2f& randomPos) {
        if (enemyName == "SkeletonWhite")
        {
            manager->addSkeleton(*data, *window, *ground, *platform, *player, "white", randomPos);
        }
        else if (enemyName == "SkeletonYellow")
        {
            manager->addSkeleton(*data, *window, *ground, *platform, *player, "yellow", randomPos);
        }
        else if (enemyName == "WraithBat")
        {
            manager->addBestiaryEnemy(*data, *window, *ground, *platform, *player, "wraith-bat", randomPos);
        }
        else if (enemyName == "VoidSlime")
        {
            manager->addBestiaryEnemy(*data, *window, *ground, *platform, *player, "void-slime", randomPos);
        }
        else if (enemyName == "DreadScorpion")
        {
            manager->addBestiaryEnemy(*data, *window, *ground, *platform, *player, "dread-scorpion", randomPos);
        }
    };

    if ((enemyAmount - enemyPerSpawn) >= 0)
    {
        for (int i = 0; i < enemyPerSpawn; ++i)
        {
            const sf::Vector2f randomPos = {
                random(spawnArea[0].x, spawnArea[0].y),
                random(spawnArea[1].x, spawnArea[1].y)
            };
            spawnEnemyAt(randomPos);
        }
        enemyAmount -= enemyPerSpawn;
    }
    else if ((enemyAmount - enemyPerSpawn) < 0 && enemyAmount != 0)
    {
        for (int i = 0; i < enemyAmount; ++i)
        {
            const sf::Vector2f randomPos = {
                random(spawnArea[0].x, spawnArea[0].y),
                random(spawnArea[1].x, spawnArea[1].y)
            };
            spawnEnemyAt(randomPos);
        }
        enemyAmount = 0;
    }
    else
    {
        isEmpty = true;
    }
}
