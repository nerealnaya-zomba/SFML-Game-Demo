#include "EnemyManager.h"
#include<Spawner.h>
#include<Skeleton.h>
#include<enemyPortal.h>
#include<GameLevel.h>

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

void EnemyManager::dropGold(const sf::Vector2f &position, const std::string& enemyType)
{
    const bool isYellow = enemyType == "yellow";
    const int coinCount = isYellow ? random(4, 6) : random(3, 5);
    const int minValue = isYellow ? 6 : 4;
    const int maxValue = isYellow ? 10 : 7;

    for (int index = 0; index < coinCount; ++index)
    {
        coins.emplace_back(
            sf::Vector2f{position.x, position.y - random(4.f, 16.f)},
            random(minValue, maxValue),
            sf::Vector2f{random(-2.3f, 2.3f), random(-4.8f, -2.6f)}
        );
    }
}

void EnemyManager::updateAI_all()
{
    for (auto &&enemy : skeletons) {
        enemy->updateAI();
    }
}

void EnemyManager::updateControls_all()
{
    for (auto &&enemy : skeletons) {
        enemy->updateControl();
    }
}

void EnemyManager::updatePhysics_all()
{
    for (auto &&enemy : skeletons) {
        enemy->updatePhysics();
    }

    updateCoins();
}

void EnemyManager::updateTextures_all()
{
    for (auto &&enemy : skeletons) {
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

    for (auto&& coin : coins)
    {
        coin.draw(*window);
    }
}

void EnemyManager::clearEnemies()
{
    skeletons.clear();
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
    
}
