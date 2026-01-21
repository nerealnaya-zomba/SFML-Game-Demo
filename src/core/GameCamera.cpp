#include "GameCamera.h"

void GameCamera::movementUpdate(float deltatime, unsigned int levelWidth, unsigned int levelHeight)
{
    float viewAreaWidth = abs(WINDOW_WIDTH*ZOOM_SCALE);
    float viewAreaHeight = abs(WINDOW_HEIGHT*ZOOM_SCALE);

    //Условия для коллизии цели с краями уровня
    // Обработка по X
    if (player->getCenterPosition().x < viewAreaWidth / 2) {
        // Уперлись в левую границу
        this->targetPos.x = viewAreaWidth / 2;
    } else if (player->getCenterPosition().x > levelWidth - viewAreaWidth / 2) {
        // Уперлись в правую границу
        this->targetPos.x = levelWidth - viewAreaWidth / 2;
    } else {
        // Свободное движение
        this->targetPos.x = player->getCenterPosition().x;
    }

    // Обработка по Y
    if (player->getCenterPosition().y < viewAreaHeight / 2) {
        this->targetPos.y = viewAreaHeight / 2;
    } else if (player->getCenterPosition().y > levelHeight - viewAreaHeight / 2) {
        this->targetPos.y = levelHeight - viewAreaHeight / 2;
    } else {
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
    // Проверка по X
    float newX = cameraPos.x + speed.x * deltatime;
    float minX = viewAreaWidth / 2;
    float maxX = levelWidth - viewAreaWidth / 2;

    if (newX > minX && newX < maxX) {
        // Свободное движение внутри границ
        cameraPos.x = newX;
    } else {
        // Уперлись в границу
        // Проверяем, пытаемся ли мы двигаться ОТ границы
        if (cameraPos.x <= minX && speed.x > 0) {
            // Камера у левой границы и пытается двигаться вправо
            cameraPos.x = std::min(newX, maxX);
        } else if (cameraPos.x >= maxX && speed.x < 0) {
            // Камера у правой границы и пытается двигаться влево
            cameraPos.x = std::max(newX, minX);
        }
        // Иначе остаёмся на границе (ничего не меняем)
    }

    // Проверка по Y (аналогично)
    float newY = cameraPos.y + speed.y * deltatime;
    float minY = viewAreaHeight / 2;
    float maxY = levelHeight - viewAreaHeight / 2;

    if (newY > minY && newY < maxY) {
        cameraPos.y = newY;
    } else {
        if (cameraPos.y <= minY && speed.y > 0) {
            cameraPos.y = std::min(newY, maxY);
        } else if (cameraPos.y >= maxY && speed.y < 0) {
            cameraPos.y = std::max(newY, minY);
        }
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

void GameCamera::setCenterPosition(sf::Vector2f pos)
{
    float viewAreaWidth = abs(WINDOW_WIDTH*ZOOM_SCALE);
    float viewAreaHeight = abs(WINDOW_HEIGHT*ZOOM_SCALE);
    screenViewSize = {viewAreaWidth,viewAreaHeight};
    // Проверка, не будет ли камера при перемещении выходить за границы
    if(pos.x-(screenViewSize.x/2)<0)
    {
        pos.x = (screenViewSize.x/2);
    } 
    else if(pos.x+(screenViewSize.x/2)>mapBorders.x)
    {
        pos.x = mapBorders.x-screenViewSize.x;
    }
    if(pos.y-(screenViewSize.y/2)>0)
    {
        pos.y = (screenViewSize.y/2);
    }
    else if(pos.y+(screenViewSize.y/2)<mapBorders.y)
    {
        pos.y = mapBorders.y-screenViewSize.y;
    }

    this->cameraPos = pos;
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
