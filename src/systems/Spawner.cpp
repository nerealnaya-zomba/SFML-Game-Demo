#include "spawner.h"

/////////////////////////////////////////////////////////////////////
// &m  - Ссылка на менеджер противников
// ea  - Количество противников
// eps - Сколько появляется противников за раз
// sa  - Площадь в которой будут спавниться
/////////////////////////////////////////////////////////////////////

Spawner::Spawner(EnemyManager &m, std::string n, unsigned int ea, unsigned int eps, float minX1, float maxX2, float minY1, float maxY2)
    : manager(&m),enemyName(n), enemyAmount(ea), enemyPerSecond(eps), spawnArea{{minX1,maxX2},{minY1,maxY2}}
{
    
    runSpawnCooldownClockIfNotRunning();
}

Spawner::Spawner(EnemyManager &m, std::string n, unsigned int ea, unsigned int eps, float a[3])
{
}

Spawner::~Spawner()
{
    // Деструктор
}

void Spawner::update() {
    //Спавним enemyPerSecond противников, если прошел кулдаун
    if(checkInterval(spawnCooldownTimer, spawnCooldown)) {
        
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

void Spawner::spawnCountOfSkeletons()
{
    for (unsigned int i = 0; i < enemyPerSecond; i++) 
    {
        
        sf::Vector2f randomPos = {random(spawnArea[0].x,spawnArea[0].y),random(spawnArea[1].x,spawnArea[1].y)};
        
        manager->addSkeleton(*data,*window,*ground,*platform,*player,type,randomPos);
        
    }
}
