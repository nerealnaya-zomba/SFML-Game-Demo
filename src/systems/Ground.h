#pragma once
#include<SFML/Graphics.hpp>
#include<iostream>
#include<Defines.h>
#include<Mounting.h>
#include<GameData.h>
#include<windows.h>

class GameLevelManager;
//////////////////////////////////////////////////
// Пол должен иметь точку начала и точку конца.
// 
// Должна указываться высота, на которой будет стоять пол.
// 
// Если существо презается в пол сбоку, то он не должен проходить насквозь. Должна работать коллизия по бокам.
// 
// Должна иметься возможность выбирать текстуру пола.
////////////////////////////////////////////////// TODO Доделать этот класс
class Ground
{
private:
    sf::RectangleShape* ground1Rect_m;
    sf::Texture* ground1Texture_m;
    sf::Sprite* ground1Sprite_m;

    sf::Vector2f point_begin, point_end;                // Точка начала и конца пола.
    float height;                                       // Высота пола
    float yPos;                                         // Позиция пола по Y коорд.
    float offset;                                       // Коллизия по Y координате + offset. Чем больше offset - тем ниже проваливается игрок, прежде чем сработает коллизия
    
    
public:
    //////////////////////////////////////////////////
    // groundFileName - Имя файла из images\Ground\TileSetGreen
    //////////////////////////////////////////////////
    Ground(GameData& gameTextures, GameLevelManager& levelManager, std::string groundFileName); //'GameLevelManager' has not been declared
    ~Ground();


    void draw(sf::RenderWindow& window, float yPos);
    
    // Getters 
    sf::RectangleShape& getRect();

    // Setters
    void setOffset(float offset);

};