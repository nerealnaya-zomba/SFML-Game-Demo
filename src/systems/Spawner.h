#pragma once
#include <EnemyManager.h>
#include <SFML/Graphics.hpp>
#include <Mounting.h>

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
    float spawnCooldown;
    unsigned int enemyAmount;
    unsigned int enemyPerSecond;
    
    /////////////////////////////////////////////////////////////////////
    // Скелет
    /////////////////////////////////////////////////////////////////////
    std::string type;   // Золотой/обычный

    /////////////////////////////////////////////////////////////////////
    // Utils
    /////////////////////////////////////////////////////////////////////
    void runSpawnCooldownClockIfNotRunning();
    void spawnCountOfSkeletons();
public:
    /////////////////////////////////////////////////////////////////////
    // &m  - Ссылка на менеджер противников
    // n   - Имя противника
    // ea  - Количество противников
    // eps - Сколько появляется противников за раз
    // sa  - Площадь в которой будут спавниться
    /////////////////////////////////////////////////////////////////////
    Spawner(EnemyManager &m, std::string n, unsigned int ea, unsigned int eps, float X1, float X2, float X3, float X4);
    Spawner(EnemyManager &m, std::string n, unsigned int ea, unsigned int eps, float a[3]);
    
    ~Spawner();

    void update();
    
    
};