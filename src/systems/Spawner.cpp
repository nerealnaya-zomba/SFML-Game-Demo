#include "spawner.h"

/////////////////////////////////////////////////////////////////////
// &m  - Ссылка на менеджер противников
// ea  - Количество противников
// eps - Сколько появляется противников за раз
// sa  - Площадь в которой будут спавниться
/////////////////////////////////////////////////////////////////////

Spawner::Spawner(EnemyManager &m, unsigned int ea, unsigned int eps, sf::Vector2f sa)
    : manager(&m), enemyAmount(ea), enemyPerSecond(eps), spawnArea(sa) 
{
    runSpawnCooldownClockIfNotRunning();
}


Spawner::~Spawner() {
    // Деструктор
}

void Spawner::update() {
    //Спавним enemyPerSecond противников, если прошел кулдаун
    if(checkInterval(spawnCooldownTimer, spawnCooldown)) {
        for (unsigned int i = 0; i < enemyPerSecond; i++) 
        {
            
        }
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