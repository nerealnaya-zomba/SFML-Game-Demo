#include "EnemyManager.h"
#include<Spawner.h>
#include<Skeleton.h>
#include<BestiaryEnemy.h>
#include<Bullet.h>
#include<enemyPortal.h>
#include<GameLevel.h>
#include<cmath>

void EnemyManager::updateSpawner()
{
    for (Spawner &spawner : spawners)
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

    for (const auto &spawner : (*data)["Spawners"])
    {   
        // Координаты спавна
        sf::Vector2f spawnArea[2] = {
            {spawner["SpawnArea"][0][0],spawner["SpawnArea"][0][1]},
            {spawner["SpawnArea"][1][0],spawner["SpawnArea"][1][1]}
        };
        // Вставляем спавнер
        spawners.emplace_back(
            *this,
            *this->gameLevel,
            spawner["EnemyName"],
            spawner["EnemyAmount"],
            spawner["SpawnCooldown"],
            spawner["EnemyPerSpawn"],
            spawnArea,
            *this->gameData,
            *this->platform,
            *this->ground,
            *this->player,
            *this->window);
    }
}

void EnemyManager::updateCoins()
{
    if(!ground || !platform || !player || !gameLevel)
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

void EnemyManager::addSkeleton(GameData& data,sf::RenderWindow& window,Ground& ground,Platform& platform,Player& player,std::string type,sf::Vector2f pos)
{
    skeletons.push_back(std::make_shared<Skeleton>(data,*this,*this->gameLevel,window,ground,platform,player,type,pos)); 
}   

void EnemyManager::addBestiaryEnemy(GameData& data, sf::RenderWindow& window, Ground& ground, Platform& platform, Player& player, std::string type, sf::Vector2f pos)
{
    bestiaryEnemies.push_back(std::make_shared<BestiaryEnemy>(data, *this, *this->gameLevel, window, ground, platform, player, type, pos));
}

void EnemyManager::dropGold(const sf::Vector2f &position, const std::string& enemyType)
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

void EnemyManager::updateAI_all()
{
    for (auto &&enemy : skeletons) {
        enemy->updateAI();
    }

    for (auto&& enemy : bestiaryEnemies) {
        enemy->updateAI();
    }
}

void EnemyManager::updateControls_all()
{
    for (auto &&enemy : skeletons) {
        enemy->updateControl();
    }

    for (auto&& enemy : bestiaryEnemies) {
        enemy->updateControl();
    }
}

void EnemyManager::updatePhysics_all()
{
    for (auto &&enemy : skeletons) {
        enemy->updatePhysics();
    }

    for (auto&& enemy : bestiaryEnemies) {
        enemy->updatePhysics();
    }

    updateCoins();
}

void EnemyManager::updateTextures_all()
{
    for (auto &&enemy : skeletons) {
        enemy->updateTextures();
    }

    for (auto&& enemy : bestiaryEnemies) {
        enemy->updateTextures();
    }

    this->removeIfNotAlive();
}

void EnemyManager::updateSpawners_all()
{
    for (auto &&i : spawners)
    {
        i.update();
    }
    
}

void EnemyManager::draw_all()
{
    for (auto &&enemy : skeletons) {
        enemy->draw();
    }

    for (auto&& enemy : bestiaryEnemies) {
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

EnemyManager::EnemyManager(const nlohmann::json& d, GameData& gd, GameLevel& gl, Platform& p, Ground& g, Player& pl, sf::RenderWindow& w)
    : data(&d), gameData(&gd), platform(&p), ground(&g), player(&pl), window(&w)
{
    // Линковка внешних ссылок к указателям
    this->gameLevel = &gl;

    loadSpawnerData();
}

EnemyManager::~EnemyManager()
{
}

void EnemyManager::attachPlayer(Player &p)
{
    this->player = &p;

    for (auto &&spawner : spawners)
    {
        spawner.attachPlayer(p);
    }
    

    for (auto &&skeleton : skeletons)
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
