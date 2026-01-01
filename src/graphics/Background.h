#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<iostream>
#include<Defines.h>

enum Type{RepeatedBackground, SingleBackground};
////////////////////////////////////////////////// IMPLEMENTME
// Представляет собой фон уровня. 
// На фон уровня можно выбирать как картинку, так и видео (FFMPEG в помощь).
//
// Есть несколько типов работы фона:
// — Repeated: Фон повторяется на протяжении всего уроня по ширине и высоте
// — Single: Только один фон в одной позиции
//////////////////////////////////////////////////
class Background
{  
    public:
    Background(sf::Vector2f pos, std::string bgName, Type type);
    ~Background();

    //////////////////////////////////////////////////
    //  Отрисовка
    //////////////////////////////////////////////////
    void draw(sf::RenderWindow& window);
    //////////////////////////////////////////////////
    private:
    Type type;                  // Тип фона
    sf::Vector2f position;      // Позиция
    std::string name;           // Название фона
    ////////////////////////////////////////////////// УСТАРЕВШИЕ
    sf::Sprite* sky;
    sf::Sprite* mansion;
    sf::Texture* skyTexture;
    sf::Texture* mansionTexture;
    ////////////////////////////////////////////////// УСТАРЕВШИЕ

};