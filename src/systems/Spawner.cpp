#include "spawner.h"

/////////////////////////////////////////////////////////////////////
// &m  - Ссылка на менеджер противников
// ea  - Количество противников
// eps - Сколько появляется противников за раз
// sa  - Площадь в которой будут спавниться
/////////////////////////////////////////////////////////////////////
template <typename T>
Spawner<T>::Spawner(EnemyManager<T> &m, unsigned int ea, unsigned int eps, sf::Vector2f sa)
    : manager(&m), enemyAmount(ea), enemyPerSecond(eps), spawnArea(sa) 
{
    runSpawnCooldownClockIfNotRunning();
}

template <typename T>
Spawner<T>::~Spawner() {
    // Деструктор
}

template <typename T>
void Spawner<T>::update() {
    //Спавним enemyPerSecond противников, если прошел кулдаун
    if(checkInterval(spawnCooldownTimer, spawnCooldown)) {
        for (unsigned int i = 0; i < enemyPerSecond; i++) 
        {
            manager->add(T);
        }
    }
}

/////////////////////////////////////////////////////////////////////
// Utils
/////////////////////////////////////////////////////////////////////
template <typename T>
void Spawner<T>::runSpawnCooldownClockIfNotRunning() {
    if(!spawnCooldownTimer.isRunning()) {
        spawnCooldownTimer.restart();
    }
}