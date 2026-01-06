#pragma once
#include <EnemyManager.h>
#include <SFML/Graphics.hpp>
#include <Mounting.h>

class EnemyManager;

class Spawner {
private:
    EnemyManager* manager;  
    sf::Vector2f spawnArea;
    sf::Clock spawnCooldownTimer;
    float spawnCooldown;
    unsigned int enemyAmount;
    unsigned int enemyPerSecond;

public:
    /////////////////////////////////////////////////////////////////////
    // &m  - Ссылка на менеджер противников
    // ea  - Количество противников
    // eps - Сколько появляется противников за раз
    // sa  - Площадь в которой будут спавниться
    /////////////////////////////////////////////////////////////////////
    Spawner(EnemyManager &m, unsigned int ea, unsigned int eps, sf::Vector2f sa);
    
    ~Spawner();

    void update();
    
    /////////////////////////////////////////////////////////////////////
    // Utils
    /////////////////////////////////////////////////////////////////////
    void runSpawnCooldownClockIfNotRunning();
};