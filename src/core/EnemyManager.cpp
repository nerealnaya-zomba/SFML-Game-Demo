#include "EnemyManager.h"

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

void EnemyManager::addSkeleton(GameData& data,sf::RenderWindow& window,Ground& ground,Platform& platform,Player& player,std::string type,sf::Vector2f pos)
{
    skeletons.push_back(std::make_shared<Skeleton>(data,window,ground,platform,player,type,pos)); 
}   

void EnemyManager::updateAI_all()
{
    for (auto enemy : skeletons) {
        enemy->updateAI();
    }
}

void EnemyManager::updateControls_all()
{
    for (auto enemy : skeletons) {
        enemy->updateControl();
    }
}

void EnemyManager::updatePhysics_all()
{
    for (auto enemy : skeletons) {
        enemy->updatePhysics();
    }
}

void EnemyManager::updateTextures_all()
{
    for (auto enemy : skeletons) {
        enemy->updateTextures();
    }
    this->removeIfNotAlive();
}

void EnemyManager::draw_all()
{
    for (auto enemy : skeletons) {
        enemy->draw();
    }
}

void EnemyManager::addSpawner(std::string enemyName)
{
}

EnemyManager::EnemyManager()
{
}

EnemyManager::~EnemyManager()
{
}