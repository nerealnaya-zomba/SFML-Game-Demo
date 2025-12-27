#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<algorithm>
#include<Skeleton.h>

template<typename T>
//TODO Класс для управления противниками. Должен отвечать за спавн, обновление и удаление убитых.
/////////////////////////////////////////////////////
// Перед появлением скелета, должен появляться портал на 2 с. Который будет увеличиваться первую секунду, а после увеличения спавнить скелета и уменьшаться обратно.
// Для этого нужно создать enemyPortal.h
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
    std::vector<T*> enemies;

    void removeIfNotAlive();
public:
    /*
        Add enemy
    */
    void add(T* enemy);

    /*
        Update
    */
    void updateAI_all();

    void updateControls_all();
    
    void updatePhysics_all();

    void updateTextures_all();

    void draw_all();


    EnemyManager();
    virtual ~EnemyManager();
};