#pragma once
#include<SFML/Graphics.hpp>

//TODO Класс для управления противниками. Должен отвечать за спавн, обновление и удаление убитых.

class EnemyManager{
private:

public:
    EnemyManager();
    virtual ~EnemyManager();
};