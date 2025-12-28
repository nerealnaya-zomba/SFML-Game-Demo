#pragma once
#include <SFML/Graphics.hpp>
#include <Mounting.h>
#include<GameData.h>

const sf::Vector2f PORTAL_OPENED_SCALE = {3.f,3.f};
const sf::Vector2f PORTAL_START_SCALE = {0.f,0.f};
const unsigned int PORTAL_EXIST_TIME = 1000;

/////////////////////////////////////////////////////
// Представляет собой портал, который появляется на 2 секунды:
// 0-1 секунда - портал увеличивается в размере до стандартного
// 1-2 секунда - портал уменьшается в размере до нуля. 
/////////////////////////////////////////////////////
class enemyPortal{
private:
    /////////////////////////////////////////////////////
    // Некоторые указатели на внешние объекты
    /////////////////////////////////////////////////////
    GameData* data;
    /////////////////////////////////////////////////////

    bool isSizingUp = true;
    bool isExist = true;
    bool isHalfPassed = false;
    bool isEnemyWalkedOut = false;      // Вышел ли противник из портала

    sf::Clock portalClock;              // Считает время сколько портал существует
    
    sf::Sprite* portalSprite;
    texturesIterHelper portalHelper;

    void sizeUp();                      
    void sizeDown();                    
public:
    enemyPortal(GameData& data, sf::Vector2f position);
    ~enemyPortal();
    
    /////////////////////////////////////////////////////
    // Изменяет размер портала в зависимтости от прошедшего времени: sizeUp или sizeDown
    /////////////////////////////////////////////////////
    void update();                          
    /////////////////////////////////////////////////////
    // Изменяет текстуру портала на следующую
    /////////////////////////////////////////////////////
    void updateTextures();                  
    /////////////////////////////////////////////////////
    // Отрисовывает портал на текущем окне
    /////////////////////////////////////////////////////
    void draw(sf::RenderWindow& window);    


    //Getters
    bool getIsExist();
    bool getIsHalfPassed();
    bool getIsEnemyWalkedOut();

    //Setters
    void setIsEnemyWalkedOut(bool value);
    void setPosition(sf::Vector2f position);
};