#include "GameCamera.h"

void GameCamera::movementUpdate(float deltatime, unsigned int levelWidth, unsigned int levelHeight)
{
    
    // REMINDER 
    float viewAreaWidth = abs(WINDOW_WIDTH*ZOOM_SCALE);
    float viewAreaHeight = abs(WINDOW_HEIGHT*ZOOM_SCALE);

    //Условия для коллизии цели с краями уровня
    if((player->getCenterPosition().x<(levelWidth-(viewAreaHeight/2))) && (player->getCenterPosition().x>(0.f+(viewAreaHeight/2)))){
        this->targetPos.x = player->getCenterPosition().x;
    } 
    if((player->getCenterPosition().y<(levelHeight-(viewAreaHeight/2))) && (player->getCenterPosition().y>(0.f+(viewAreaHeight/2)))) {
        this->targetPos.y = player->getCenterPosition().y;
    } 

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

    // Зажим скорости в рамки максимальной
    speed.x = std::clamp(speed.x, -maxSpeed.x, maxSpeed.x);
    speed.y = std::clamp(speed.y, -maxSpeed.y, maxSpeed.y);

    //Условия для коллизии камеры с краями уровня
    if(((cameraPos.x + speed.x * deltatime)<(levelWidth-viewAreaWidth/2)) && ((cameraPos.x + speed.x * deltatime) > (viewAreaWidth/2))) {
        cameraPos.x += speed.x * deltatime;
    } 
    if(((cameraPos.y + speed.y * deltatime)<(levelHeight-viewAreaHeight/2)) && ((cameraPos.y + speed.y * deltatime) > (viewAreaHeight/2))) {
        cameraPos.y += speed.y * deltatime;
    } 
    
    // Дополнительное мягкое торможение вблизи цели
    if (distance < brakeRadius) {
        float brakeFactor = distance / brakeRadius;  // 1.0 на краю, 0.0 в центре
        speed *= brakeFactor * 0.95f;  // Плавное замедление
    }

}

GameCamera::GameCamera(sf::View& view)
{
    // Привязываем ссылки к указателям
    this->view = &view;

    this->view->zoom( ZOOM_SCALE ); // Приближаем камеру
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
        this->movementUpdate(
            dt,
            levelManager->getCurrentLevelSize().x,
            levelManager->getCurrentLevelSize().y
        );
        
        this->view->setCenter(this->cameraPos);
        return;
    }
}

void GameCamera::attachGameLevelManager(GameLevelManager &m)
{
    this->levelManager = &m;
}

void GameCamera::attachPlayer(Player &player)
{
    this->player = &player;
}

float GameCamera::getZoom() const
{
    return ZOOM_SCALE;
}

sf::Vector2f GameCamera::getScreenViewSize() const
{
    float viewAreaWidth = abs(WINDOW_WIDTH*ZOOM_SCALE);
    float viewAreaHeight = abs(WINDOW_HEIGHT*ZOOM_SCALE);


    return sf::Vector2f(viewAreaWidth,viewAreaHeight);
}

sf::Vector2f GameCamera::getScreenViewPos() const
{
    sf::Vector2f pos = {view->getCenter().x-(getScreenViewSize().x)/2,
                        view->getCenter().y-(getScreenViewSize().y)/2};
    return pos;
}

sf::Vector2f GameCamera::getSpeed() const
{
    return this->speed;
}

sf::Vector2f GameCamera::getCameraCenter() const
{
    return view->getCenter();
}

sf::Vector2f GameCamera::getCameraCenterPos() const
{
    return this->cameraPos;
}
