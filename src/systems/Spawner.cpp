#include "spawner.h"
#include<EnemyManager.h>

Spawner::Spawner(EnemyManager &m, std::string n, unsigned int ea, unsigned int sc, float minX1, float maxX2, float minY1, float maxY2,
    GameData& d, Platform& p, Ground& g, Player& pl, sf::RenderWindow& w) 
    : manager(&m),enemyName(n), enemyAmount(ea), spawnCooldown(sc), spawnArea{{minX1,maxX2},{minY1,maxY2}}, data(&d), platform(&p), ground(&g), player(&pl),window(&w)
{
    runSpawnCooldownClockIfNotRunning();
}

Spawner::Spawner(EnemyManager &m, std::string n, unsigned int ea, unsigned int sc, sf::Vector2f sa[2],
    GameData& d, Platform& p, Ground& g, Player& pl, sf::RenderWindow& w)
    : manager(&m),enemyName(n), enemyAmount(ea), spawnCooldown(sc), spawnArea{sa[0],sa[1]}, data(&d), platform(&p), ground(&g), player(&pl),window(&w)
{
    runSpawnCooldownClockIfNotRunning();
}

Spawner::~Spawner()
{
    // Деструктор
}

void Spawner::update() {
    //Спавним enemyPerSecond противников, если прошел кулдаун
    if(checkInterval(spawnCooldownTimer, spawnCooldown)) {
        spawnCountOfEnemies();
    }
}

/////////////////////////////////////////////////////////////////////
// Utils
/////////////////////////////////////////////////////////////////////
void Spawner::runSpawnCooldownClockIfNotRunning() {
    if(!spawnCooldownTimer.isRunning()) {
        spawnCooldownTimer.restart();
    }
}

void Spawner::spawnCountOfEnemies()
{
    for (unsigned int i = 0; i < enemyAmount; i++) 
    {
        
        sf::Vector2f randomPos = {random(spawnArea[0].x,spawnArea[0].y),random(spawnArea[1].x,spawnArea[1].y)};
        if(enemyName == "SkeletonWhite")
        {
            manager->addSkeleton(*data,*window,*ground,*platform,*player,"white",randomPos);
        }
        else if(enemyName == "SkeletonYellow")
        {
            manager->addSkeleton(*data,*window,*ground,*platform,*player,"yellow",randomPos);
        }
    }
}
