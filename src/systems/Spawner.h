#pragma once
#include <EnemyManager.h>
#include <SFML/Graphics.hpp>
#include <Mounting.h>
#include <GameData.h>
#include <Platform.h>
#include <Ground.h>
#include <Player.h>

class EnemyManager;

class Spawner {
private:
    /////////////////////////////////////////////////////////////////////
    // Общее
    /////////////////////////////////////////////////////////////////////
    EnemyManager* manager;  
    GameData* data;
    Platform* platform;
    Ground* ground;
    Player* player;
    sf::RenderWindow* window;
    
    const std::string enemyName;
    sf::Vector2f spawnArea[2];      // [0] - X позиция мин и макс. [1] - Y позиция мин и макс.
    sf::Clock spawnCooldownTimer;
    unsigned int enemyAmount;
    unsigned int spawnCooldown;

    /////////////////////////////////////////////////////////////////////
    // Utils
    /////////////////////////////////////////////////////////////////////
    void runSpawnCooldownClockIfNotRunning();
    void spawnCountOfEnemies();
public:
    /////////////////////////////////////////////////////////////////////
    // &m  - Ссылка на менеджер противников
    // n   - Имя противника: Skeleton, ...
    // ea  - Количество противников
    // eps - Сколько появляется противников за раз
    // minX1, maxX2, minY1, maxY2  - Координаты площади где будут спавниться скелеты
    /////////////////////////////////////////////////////////////////////
    Spawner(EnemyManager &m, std::string n, unsigned int ea, unsigned int sc, float minX1, float maxX2, float minY1, float maxY2,
    GameData& d, Platform& p, Ground& g, Player& pl, sf::RenderWindow& w);
    Spawner(EnemyManager &m, std::string n, unsigned int ea, unsigned int sc, sf::Vector2f sa[2],
    GameData& d, Platform& p, Ground& g, Player& pl, sf::RenderWindow& w);
    
    ~Spawner();

    void update();
    
    
};