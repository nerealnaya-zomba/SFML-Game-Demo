#pragma once
#include<SFML/Graphics.hpp>
#include<iostream>
#include<Defines.h>
#include<Mounting.h>
#include<GameData.h>

const float BASE_GROUND_OFFSET = 8.f;

class GameLevel; // NOTE Нужен для избежания ошибки зависимостей

//////////////////////////////////////////////////
// Пол должен иметь точку начала и точку конца.
// 
// Должна указываться высота, на которой будет стоять пол.
// 
// Если существо презается в пол сбоку, то он не должен проходить насквозь. Должна работать коллизия по бокам.
// 
// Должна иметься возможность выбирать текстуру пола.
////////////////////////////////////////////////// REMINDER СУПЕР ВАЖНО! После того как закончишь с менеджером уровней поменяй WINDOW_WIDTH WINDOW_HEIGHT на levelSize
class Ground
{
private:
    sf::RectangleShape* ground1Rect_m;
    sf::Texture* ground1Texture_m;
    sf::Sprite* ground1Sprite_m;

    unsigned int point_begin, point_end;                // Точка начала и конца пола.
    float height;                                       // Высота пола
    unsigned int yPos;                                  // Позиция пола по Y коорд. 
    float offset;                                       // Коллизия по Y координате + offset. Чем больше offset - тем ниже проваливается игрок, прежде чем сработает коллизия
    
    sf::Vector2u tilesetsize;                           // Размер текстуры в пикселях
    float offSet;
public:
    //////////////////////////////////////////////////
    // groundFileName - Имя файла из images\Ground\TileSetGreen
    //////////////////////////////////////////////////
    Ground(GameData& gameTextures, GameLevel& level, std::string groundFileName, unsigned int point_begin, unsigned int point_end, unsigned int yPos = 0u, float offSet = BASE_GROUND_OFFSET);
    ~Ground();

    void draw(sf::RenderWindow& window);
    
    void clearGround();

    // Getters 
    sf::RectangleShape& getRect();

    // Setters
    void setOffset(float offset);

};
