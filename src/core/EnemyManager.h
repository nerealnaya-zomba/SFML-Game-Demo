#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<algorithm>
#include<fstream>
#include<deque>
#include<map>
#include <GameData.h>
#include <Platform.h>
#include <Ground.h>
#include <Player.h>


class Spawner;
class Skeleton;

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

    const nlohmann::json* data;
    GameData* gameData;
    Platform* platform;
    Ground* ground;
    Player* player;
    sf::RenderWindow* window;

    ////////////////////////
    // Arrays
    ////////////////////////
    std::vector<std::shared_ptr<Skeleton>> skeletons;
    std::vector<Spawner> spawners;

    ////////////////////////
    //Spawners updating
    ////////////////////////
    void updateSpawner();

    ////////////////////////
    // Removing enemy
    ////////////////////////
    /*
        Remove enemy
    */
    void removeIfNotAlive();

    
    ////////////////////////
    // Загружает спавнеры levelData
    ////////////////////////
    void loadSpawnerData();

public:
    

    /*
        Update
    */
    void updateAI_all();

    void updateControls_all();
    
    void updatePhysics_all();

    void updateTextures_all();

    void updateSpawners_all();

    void draw_all();

    /*
        Add enemy
    */
    void addSkeleton(GameData& data,sf::RenderWindow& window,Ground& ground,Platform& platform,Player& player,std::string type,sf::Vector2f pos);
    void addFlyingEnemy();   // IMPLEMENTME Добавь че нить

    ////////////////////////
    // Добавить спавнер противника
    ////////////////////////
    void addSpawner(std::string enemyName);

    ////////////////////////
    // ln - Название файла уровня
    ////////////////////////
    EnemyManager(const nlohmann::json& d, GameData& gd, Platform& p, Ground& g, Player& pl, sf::RenderWindow& w);
    ~EnemyManager();

    //Setters
    void attachPlayer(Player& p);
};