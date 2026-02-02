#include "Spawner.h"
#include<EnemyManager.h>

Spawner::Spawner(EnemyManager &m, std::string n, int ea, int sc, int eps, float minX1, float maxX2, float minY1, float maxY2,
    GameData& d, Platform& p, Ground& g, Player& pl, sf::RenderWindow& w) 
    : manager(&m),enemyName(n), enemyAmount(ea), spawnCooldown(sc), enemyPerSpawn(eps), spawnArea{{minX1,maxX2},{minY1,maxY2}}, 
    data(&d), platform(&p), ground(&g), player(&pl),window(&w)
{
    runSpawnCooldownClockIfNotRunning();
}

Spawner::Spawner(EnemyManager &m, std::string n, int ea, int sc, int eps, sf::Vector2f sa[2],
    GameData& d, Platform& p, Ground& g, Player& pl, sf::RenderWindow& w)
    : manager(&m),enemyName(n), enemyAmount(ea), spawnCooldown(sc), enemyPerSpawn(eps), spawnArea{sa[0],sa[1]}, 
    data(&d), platform(&p), ground(&g), player(&pl),window(&w)
{
    runSpawnCooldownClockIfNotRunning();
}

Spawner::~Spawner()
{
    // Деструктор
}

void Spawner::update() {
    if(isEmpty) return;

    //Спавним enemyPerSecond противников, если прошел кулдаун
    if(checkInterval(spawnCooldownTimer, spawnCooldown)) {
        spawnCountOfEnemies();
    }
}

void Spawner::attachPlayer(Player &p)
{
    this->player = &p;
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
    if(isEmpty) return;

    // Спавним пачку и отнимаем значение пачки от общего кол-ва оставшихся противников
    if((enemyAmount-enemyPerSpawn)>=0){
        for (unsigned int i = 0; i < enemyPerSpawn; i++) 
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
        // Уменьшаем кол-во противников после спавна
        enemyAmount-=enemyPerSpawn;
    } 
    // Если противников мало для спавна пачки enemyPerSpawn, то спавним оставшееся количество и сбрасываем enemyAmount до 0
    else if((enemyAmount-enemyPerSpawn)<0 && enemyAmount!= 0){
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
        // Уменьшаем кол-во противников до нуля после спавна
        enemyAmount = 0;
    }
    // Если противники закончились, помечаем спавнер как "опустошенный"
    else{
        isEmpty = true;
    }
}
