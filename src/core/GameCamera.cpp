#include "GameCamera.h"

void GameCamera::movementUpdate(float deltatime)
{
    sf::Vector2f targetPos = player->getCenterPosition();
    sf::Vector2f offset = targetPos - cameraPos;
    float distance = std::sqrt(offset.x * offset.x + offset.y * offset.y);
    
    // Настройки пружины
    float stiffness = 0.1f;    // Жёсткость пружины
    float damping = 0.8f;      // Демпфирование
    
    // Сила пружины (сильнее при большем расстоянии)
    sf::Vector2f springForce = offset * stiffness;
    
    // Демпфирующая сила (против скорости)
    sf::Vector2f dampingForce = -speed * damping;
    
    // Результирующая сила
    sf::Vector2f totalForce = (springForce * 480.f) + dampingForce;
    
    // Интегрируем (упрощённый Euler)
    speed += totalForce * deltatime;
    cameraPos += speed * deltatime;
    
    // Дополнительное мягкое торможение вблизи цели
    if (distance < brakeRadius) {
        float brakeFactor = distance / brakeRadius;  // 1.0 на краю, 0.0 в центре
        speed *= brakeFactor * 0.95f;  // Плавное замедление
    }

    // Зажим скорости в рамки максимальной
    speed.x = std::clamp(speed.x, -maxSpeed.x, maxSpeed.x);
    speed.y = std::clamp(speed.y, -maxSpeed.y, maxSpeed.y);
    
    // Логи
    std::cout << "Camera speed: {"<< speed.x << ", " << speed.y << "};" << std:: endl;
}

GameCamera::GameCamera(sf::View &view, Player &player)
{
    // Привязываю ссылки к указателям
    this->player = &player;
    this->view = &view;
    this->view->zoom(0.75f);


}

GameCamera::~GameCamera()
{
}

void GameCamera::update()
{
    static sf::Clock clock;
    float dt = clock.restart().asSeconds();

    if(this->isConditionSuccessed)
    {   
        this->movementUpdate(dt);
        
        this->view->setCenter(this->cameraPos);
        return;
    }
}