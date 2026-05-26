#pragma once

#include <SFML/Graphics.hpp>

#include <GameCamera.h>
#include <GameData.h>
#include <Defines.h>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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
class BackgroundAtmosphere;

//////////////////////////////////////////
// То, что Z=-100, на него паралакс не воздействует, и он абсолютно всегда находится в позиции камеры Z_FOREGROUND_POWER = 1.f
// По мере приблежения Z к 0, скорость объектов увеличивается
// А после преодоления предела в 0, становится быстрее
//////////////////////////////////////////
enum Type
{
    RepeatedBackgroundXY,
    RepeatedBackgroundX,
    RepeatedBackgroundY,
    SingleBackground
};

struct BackgroundSceneConfig
{
    std::string themeName{};
    std::size_t layerIndex = 0;
    std::size_t layerCount = 1;
    float tileOffsetY = 0.f;
};

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
    Background(
        GameData& d,
        GameCamera& c,
        GameLevel& l,
        sf::Vector2f pos,
        std::string bgName,
        sf::Vector2f parallaxFact,
        Type t,
        BackgroundSceneConfig sceneConfig = {}
    );
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
    const GameCamera* camera = nullptr;
    const GameLevel* level = nullptr;

    const Type type;                        // Тип фона
    const sf::Vector2f position;            // Позиция при создании.
    const std::string name;                 // Название фона
    const BackgroundSceneConfig sceneConfig;
    sf::Vector2u bgTextureSize{};

    // Parallax
    sf::Vector2f parallaxFactor;            // Растет с удалением от центра спрайта
    sf::Clock animationClock;

    void applyParallax();
    sf::FloatRect getActiveViewRect() const;
    void drawRepeated(sf::RenderWindow& window) const;

    // Основной спрайт
    std::unique_ptr<sf::Sprite> bgFront;
    mutable std::unique_ptr<sf::Sprite> repeatedSprite;
    std::unique_ptr<BackgroundAtmosphere> atmosphere;
};
