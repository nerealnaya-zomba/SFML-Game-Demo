#pragma once
#include<SFML/Graphics.hpp>

#include<Defines.h>
#include<Player.h>
#include<GameLevel.h>

#include<iostream>
#include<functional>

class GameLevelManager;
class Player;

constexpr sf::Vector2f BASE_CAMERAPOS = {WINDOW_WIDTH/2,WINDOW_HEIGHT/2};
constexpr sf::Vector2f BASE_CAMERA_SMOOTH_TIME = {0.16f,0.22f};
constexpr sf::Vector2f BASE_MAX_SPEED = {4200.f,3200.f};
constexpr sf::Vector2f BASE_CAMERA_LOOK_AHEAD_MAX = {220.f,120.f};
constexpr float BASE_CAMERA_LOOK_AHEAD_TIME = 0.18f;
constexpr float BASE_CAMERA_LOOK_AHEAD_SHARPNESS = 8.f;
constexpr float BASE_CAMERA_SNAP_DISTANCE_FACTOR = 1.1f;

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
    sf::Vector2f targetPos = BASE_CAMERAPOS;        // Позиция куда камера должна попасть
    sf::Vector2f speed = {0,0};                     // Текущая скорость камеры в world-space
    sf::Vector2f maxSpeed = BASE_MAX_SPEED;         // Максимальная скорость передвижения
    sf::Vector2f smoothedPlayerVelocity = {0,0};    // Сглаженная скорость игрока для look-ahead
    sf::Vector2f lastPlayerPos = BASE_CAMERAPOS;    // Нужна чтобы вычислять скорость игрока
    bool hasLastPlayerPos = false;

    sf::Vector2f mapBorders = {0.f, 0.f};          // Края карты, чтобы камера не заходила за края

    sf::Vector2f screenViewSize = {0.f, 0.f};      // Размер области которую видит игрок.  
    sf::Vector2f screenViewPos = {0.f, 0.f};       // Позиция области которую видит игрок. Origin позиции с левого верхнего угла

    bool chasePlayer = true;                        // Должна ли камера преследовать игрока
    bool isConditionSuccessed = true;               // Выполнено ли условие заданное pointCameraAt()
    std::function<bool()> releaseCondition;         // Условие выхода из pointCameraAt
    sf::Clock pointTargetClock;                     // Таймер режима pointCameraAt
    sf::Time pointTargetDuration = sf::Time::Zero;
    bool useTimedPointTarget = false;

    Player* player = nullptr;                       // Указатель на игрока для управления
    GameLevelManager* levelManager = nullptr;       // Указатель на менеджер, для получения данных о рамках уровня
    sf::View* view = nullptr;                       // Указатель на вид для управления

    //////////////////////////////////////////////////
    // Обновление переменных передвижения
    //////////////////////////////////////////////////
    void movementUpdate(float deltatime, unsigned int levelWidth, unsigned int levelHeight);
    sf::Vector2f clampToLevelBounds(sf::Vector2f pos, unsigned int levelWidth, unsigned int levelHeight) const;
    sf::Vector2f calculateFollowTarget(float deltatime, unsigned int levelWidth, unsigned int levelHeight);
    float smoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeedValue, float deltaTime);
    void resetMotionState();

public:
    GameCamera(sf::View& view);
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
    void pointCameraAt(sf::Vector2f pos, std::function<bool()> condition);
    void pointCameraAt(sf::Vector2f pos, unsigned int time);

    // Setters
    void setMoveSpeed(sf::Vector2f pos);
    void attachGameLevelManager(GameLevelManager& m);
    void attachPlayer(Player& player);
    void setCenterPosition(sf::Vector2f pos);

    // Getters
    float getZoom()                     const;
    sf::Vector2f getScreenViewSize()    const;
    sf::Vector2f getScreenViewPos()     const;
    sf::Vector2f getSpeed()             const;
    sf::Vector2f getCameraCenter()      const; // Size-based
    sf::Vector2f getCameraCenterPos()   const;
};
