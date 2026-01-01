#pragma once
#include<SFML/Graphics.hpp>

#include<Defines.h>
#include<Player.h>
#include<GameLevel.h>

#include<iostream>
#include<functional>

constexpr sf::Vector2f BASE_CAMERAPOS = {WINDOW_WIDTH/2,WINDOW_HEIGHT/2};
constexpr sf::Vector2f BASE_SPEED = {0.2f,0.2f};
constexpr sf::Vector2f BASE_MAX_SPEED = {600.f,600.f};
constexpr sf::Vector2f BASE_BRAKE_SPEED = {25.f,25.f};  //NOTE Не используется нигде
constexpr float BASE_BRAKE_RADIUS = 100.f;

constexpr float ZOOM_SCALE = 0.75f;

//////////////////////////////////////////////////
// Камера плавно преследующая игрока в обычном состоянии.
// 
// Можно направить на определенную точку, и оставлять ее там, пока выполняется какое-то условие.
// Камера плавно передвигается на выбранную точку
//////////////////////////////////////////////////
class GameCamera
{
private:
    sf::Vector2f cameraPos = BASE_CAMERAPOS;        // Позиция камеры прямо сейчас
    sf::Vector2f targetPos;                         // Позиция куда камера должна попасть

    sf::Vector2f acceleration = BASE_SPEED;         // Скорость ускорения камеры
    sf::Vector2f speed = {0,0};                     // Скорость передвижения камеры
    sf::Vector2f maxSpeed = BASE_MAX_SPEED;         // Максимальная скорость передвижения
    sf::Vector2f suddenBrake = BASE_BRAKE_SPEED;    // Внезапный тормоз камеры когда она на цели

    sf::Vector2f mapBorders;                        // Края карты, чтобы камера не заходила за края

    sf::Vector2f screenViewSize;                    // Размер области которую видит игрок.  
    sf::Vector2f screenViewPos;                     // Позиция области которую видит игрок. Origin позиции с левого верхнего угла
    
    float brakeRadius = BASE_BRAKE_RADIUS;          // Радиус вокруг цели, при котором срабатывает тормоз

    bool chasePlayer = true;                        // Должна ли камера преследовать игрока
    bool isConditionSuccessed = true;               // Выполнено ли условие заданное pointCameraAt()

    Player* player;                                 // Указатель на игрока для управления
    GameLevelManager* levelManager;                 // Указатель на менеджер, для получения данных о рамках уровня
    sf::View* view;                                 // Указатель на вид для управления

    //////////////////////////////////////////////////
    // Обновление переменных передвижения
    //////////////////////////////////////////////////
    void movementUpdate(float deltatime);

    //////////////////////////////////////////////////
    // Останавливает камеру, когда она касается краев карты.
    // 
    // Нужно чтобы камера не выходила за края, и не показывала то, что не должен видеть игрок.
    //////////////////////////////////////////////////
    void mapBorderCollision(); // NOTE Не работал корректно, так что я его сейчас не использую. Вместо этого я в movementUpdate() использовал другой вариант.
public:
    GameCamera(sf::View& view, Player& player, GameLevelManager& levelManager);
    ~GameCamera();

    //////////////////////////////////////////////////
    // Двигает камеру с определенной скоростью к цели
    // И меняет view
    //////////////////////////////////////////////////
    void update();                                                         
    //////////////////////////////////////////////////
    // Меняет isConditionSuccessed на false
    //
    // Смотрит на точку пока условие не будет выполнено, или пока время таймера не истечет.
    //////////////////////////////////////////////////
    void pointCameraAt(sf::Vector2f pos, std::function<bool()> condition);  // IMPLEMENTME
    void pointCameraAt(sf::Vector2f pos, unsigned int time);                // IMPLEMENTME

    // Setters
    void setMoveSpeed(sf::Vector2f pos);                                    // IMPLEMENTME

    // Getters
    float getZoom()                     const;
    sf::Vector2f getScreenViewSize()    const;
    sf::Vector2f getScreenViewPos()     const;
    sf::Vector2f getSpeed()             const;
    sf::Vector2f getCameraCenter()      const;
};