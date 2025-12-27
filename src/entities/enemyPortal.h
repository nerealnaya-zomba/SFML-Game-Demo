#pragma once
#include <SFML/Graphics.hpp>
#include <Mounting.h>
#include<GameData.h>

const sf::Vector2f PORTAL_BASE_SCALE = {1.f,1.f};
const unsigned int PORTAL_EXIST_TIME = 2000;

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
    bool isSizingDown = false;
    bool isExist = true;
    bool isHalfPassed = false;

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
    void updateTextures();                  // IMPLEMENTME
    /////////////////////////////////////////////////////
    // Отрисовывает портал на текущем окне
    /////////////////////////////////////////////////////
    void draw(sf::RenderWindow& window);    // IMPLEMENTME


    //Getters
    bool getIsExist();
};