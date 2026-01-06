#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<algorithm>
#include<Skeleton.h>
#include<enemyPortal.h>
#include<deque>
#include<map>
#include<Spawner.h>

class Spawner;

//NOTE Класс для управления противниками. Должен отвечать за спавн, обновление и удаление убитых.
//NOTE Если хочешь добавить еще один класс противника - в конце .cpp файла добавь template class EnemyManager<Твой противник>. Также и со Spawner.
/////////////////////////////////////////////////////
// Перед появлением скелета, появляется портал на 2 с. Который будет увеличиваться первую секунду, а после увеличения спавнить скелета и уменьшаться обратно.
/////////////////////////////////////////////////////
/* У передаваемого объекта должны присутствовать такие методы и поля как:
 — updateAI()
 — updatePhysics()
 — updateTextures();
 — draw();
Booleans: 
 — isAlive
*/
class EnemyManager{
private:

    std::vector<std::shared_ptr<Skeleton>> skeletons;
    std::vector<Spawner> spawners;
    ////////////////////////
    //Spawners updating
    ////////////////////////
    void updateSpawner();
    ////////////////////////
    // Removing/adding enemy
    ////////////////////////
    /*
        Remove enemy
    */
    void removeIfNotAlive();

    

    void addFlyingEnemy();   // IMPLEMENTME Добавь че нить

public:
    

    /*
        Update
    */
    void updateAI_all();

    void updateControls_all();
    
    void updatePhysics_all();

    void updateTextures_all();

    void draw_all();

    /*
        Add enemy
    */
    void addSkeleton(GameData& data,sf::RenderWindow& window,Ground& ground,Platform& platform,Player& player,std::string type,sf::Vector2f pos);

    ////////////////////////
    // Добавить спавнер противника
    ////////////////////////
    void addSpawner(std::string enemyName);

    ////////////////////////
    // Constructor & destructor
    ////////////////////////
    EnemyManager();
    virtual ~EnemyManager();
};