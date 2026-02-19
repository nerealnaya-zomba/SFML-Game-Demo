#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<iostream>
#include<Defines.h>
#include<GameData.h>
#include<GameCamera.h>
#include<Player.h>
/////////////////////////////////////
// BASE_PARALLAX_FACTOR == 1.f - Фон следует за игроком без малейших изменений в позиции
// BASE_PARALLAX_FACTOR == 0.f - Фон остается на месте своего создания
// BASE_PARALLAX_FACTOR < 0.f - Фон двигается за игроком, но смещается в противоположную от игрока сторону (Фон)
// BASE_PARALLAX_FACTOR > 0.f - Фон двигается за игроком, но смещается в сторону игрока (Перед)
/////////////////////////////////////
const float BASE_PARALLAX_FACTOR = 1.f;

class GameCamera;
class GameLevelManager;
class GameLevel;
class Player;

//////////////////////////////////////////
// То, что Z=-100, на него паралакс не воздействует, и он абсолютно всегда находится в позиции камеры Z_FOREGROUND_POWER = 1.f
// По мере приблежения Z к 0, скорость объектов увеличивается
// А после преодоления предела в 0, становится быстрее
//////////////////////////////////////////
enum Type{RepeatedBackground, SingleBackground};
////////////////////////////////////////////////// IMPLEMENTME
// Представляет собой фон уровня. 
// На фон уровня можно выбирать как картинку, так и видео (FFMPEG в помощь).
// 
// У фона есть задние объекты z<0 и передние z>0.
// На задние и передние объекты работает парралакс эффект
//
// Есть несколько типов работы фона:
// — Repeated: Фон повторяется на протяжении всего уроня по ширине и высоте
// — Single: Только один фон в одной позиции
//////////////////////////////////////////////////
class Background
{  
public:
    Background(GameData& d, GameCamera& c, GameLevel& l, sf::Vector2f pos, std::string bgName, sf::Vector2f parallaxFact, Type t);
    ~Background();

    //////////////////////////////////////////////////
    //  Обновление
    //////////////////////////////////////////////////
    void update();
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    //  Отрисовка
    //////////////////////////////////////////////////
    void draw(sf::RenderWindow& window);
    //////////////////////////////////////////////////

    // Setters
    void setParallaxFactor(sf::Vector2f f);

    // Getters
    sf::Sprite &getSprite();

private:
    // Указатели на внешние данные
    const GameCamera* camera;
    const GameData* gamedata;
    const GameLevel* level;
    const Player* player;

    const Type type;                        // Тип фона
    const sf::Vector2f position;            // Позиция при создании.
    const std::string name;                 // Название фона
    sf::Vector2u bgTextureSize;

    // Parallax
    int zDepth;                             // Сила с которой растет offset
    sf::Vector2f parallaxFactor;            // Растет с удалением от центра спрайта

    void applyParallax(); // Вычисляет offset, основываясь на zDepth и удалении от сентра спрайта

    // Основной спрайт
    sf::Sprite* bgFront;
};