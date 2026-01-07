#include "EnemyManager.h"
#include<Spawner.h>
#include<Skeleton.h>
#include<enemyPortal.h>

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

void EnemyManager::addSkeleton(GameData& data,sf::RenderWindow& window,Ground& ground,Platform& platform,Player& player,std::string type,sf::Vector2f pos)
{
    skeletons.push_back(std::make_shared<Skeleton>(data,window,ground,platform,player,type,pos)); 
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
}

void EnemyManager::addSpawner(std::string enemyName)
{
}

EnemyManager::EnemyManager(const nlohmann::json& d, GameData& gd, Platform& p, Ground& g, Player& pl, sf::RenderWindow& w)
    : data(&d), gameData(&gd), platform(&p), ground(&g), player(&pl), window(&w)
{
    loadSpawnerData();
}

EnemyManager::~EnemyManager()
{
}