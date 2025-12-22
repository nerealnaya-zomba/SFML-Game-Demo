#include<SFML/Graphics.hpp>

#include<Player.h>

#include<iostream>
#include<functional>

//////////////////////////////////////////////////
// Камера плавно преследующая игрока в обычном состоянии.
// 
// Можно направить на определенную точку, и оставлять ее там, пока выполняется какое-то условие.
// Камера плавно передвигается на выбранную точку
//////////////////////////////////////////////////
class GameCamera
{
private:
    sf::Vector2f cameraPos;     // Позиция камеры прямо сейчас
    sf::Vector2f targetPos;     // Позиция куда камера должна попасть
    sf::Vector2f moveSpeed;     // Скорость передвижения камеры

    bool chasePlayer = true;            // Должна ли камера преследовать игрока
    bool isConditionSuccessed = true;   // Выполнено ли условие заданное pointCameraAt()

    Player* player; // Указетль на игрока для управления
    sf::View* view; // Указетль на вид для управления
public:
    GameCamera(sf::View& view, Player& player);
    ~GameCamera();

    //////////////////////////////////////////////////
    // Двигает камеру с определенной скоростью к цели
    // И меняет view
    //////////////////////////////////////////////////
    void update();                                                          //IMPLEMENTME

    //////////////////////////////////////////////////
    // Меняет isConditionSuccessed на false
    //
    // Смотрит на точку пока условие не будет выполнено, или пока время таймера не истечет.
    //////////////////////////////////////////////////
    void pointCameraAt(sf::Vector2f pos, std::function<bool()> condition);  // IMPLEMENTME
    void pointCameraAt(sf::Vector2f pos, unsigned int time);                // IMPLEMENTME

    //Setters
    void setCameraPos(sf::Vector2f pos);                                    // IMPLEMENTME
    void setMoveSpeed(sf::Vector2f pos);                                    // IMPLEMENTME
};