#include "GameCamera.h"

void GameCamera::movementUpdate(float deltatime)
{
    this->targetPos = player->getCenterPosition();


    // Выравнивание цели, чтобы не заходила за края
    // REMINDER СУПЕР ВАЖНО! После того как закончишь с менеджером уровней поменяй WINDOW_WIDTH WINDOW_HEIGHT на levelSize

    float viewAreaWidth = abs(WINDOW_WIDTH*ZOOM_SCALE);
    float viewAreaHeight = abs(WINDOW_HEIGHT*ZOOM_SCALE);


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

    //mapBorderCollision();

    // Зажим скорости в рамки максимальной
    speed.x = std::clamp(speed.x, -maxSpeed.x, maxSpeed.x);
    speed.y = std::clamp(speed.y, -maxSpeed.y, maxSpeed.y);

    //Условия для коллизии камеры с краями уровня
    if(((cameraPos.x + speed.x * deltatime)<(WINDOW_WIDTH-viewAreaWidth/2)) && ((cameraPos.x + speed.x * deltatime) > (viewAreaWidth/2))) cameraPos.x += speed.x * deltatime;
    if(((cameraPos.y + speed.y * deltatime)<(WINDOW_HEIGHT-viewAreaHeight/2)) && ((cameraPos.y + speed.y * deltatime) > (viewAreaHeight/2))) cameraPos.y += speed.y * deltatime;
    
    // Дополнительное мягкое торможение вблизи цели
    if (distance < brakeRadius) {
        float brakeFactor = distance / brakeRadius;  // 1.0 на краю, 0.0 в центре
        speed *= brakeFactor * 0.95f;  // Плавное замедление
    }

    // Логи
    std::cout << "Camera speed: {"<< speed.x << ", " << speed.y << "};" << std:: endl;
    std::cout << "Target position: {"<< targetPos.x << ", " << targetPos.y << "};" << std:: endl;
}

void GameCamera::mapBorderCollision()
{   
    // Края уровня
    sf::Vector2f levelSize = levelManager->getCurrentLevelSize();
    float rightBorder = levelSize.x;
    float bottomBorder = levelSize.y;

    // Область, которую видит игрок
    sf::RectangleShape viewArea;
    // Учитываем приближение
    float viewAreaWidth = abs(WINDOW_WIDTH*ZOOM_SCALE);
    float viewAreaHeight = abs(WINDOW_HEIGHT*ZOOM_SCALE);
    // Настраиваем
    viewArea.setSize({viewAreaWidth,viewAreaHeight});
    viewArea.setOrigin(viewArea.getGlobalBounds().getCenter());
    viewArea.setPosition(cameraPos);

    // Позиции области зрения
    float viewAreaLeftSide = viewArea.getGlobalBounds().position.x;
    float viewAreaRightSide = viewArea.getGlobalBounds().position.x + viewAreaWidth;
    float viewAreaTopSide = viewArea.getGlobalBounds().position.y;
    float viewAreaBottomSide = viewArea.getGlobalBounds().position.y + viewAreaHeight;
    
    //Коллизия области зрения
    // REMINDER СУПЕР ВАЖНО! После того как закончишь с менеджером уровней, поставь вместо WINDOW_WIDTH - levelSize.x и Вместо WINDOW_HEIGHT - levelSize.y
        //Горизонталь
    if(viewAreaLeftSide < 0.f){
        float fixPos = 0.f + (viewAreaWidth/2);
        cameraPos.x = fixPos;
    } else if (viewAreaRightSide > WINDOW_WIDTH){
        float fixPos = WINDOW_WIDTH - (viewAreaWidth/2);
        cameraPos.x = fixPos;
    }
        //Вертикаль
    if(viewAreaTopSide < 0.f){
        float fixPos = 0.f + (viewAreaHeight/2);
        cameraPos.y = fixPos;
    } else if (viewAreaBottomSide > WINDOW_HEIGHT){
        float fixPos = WINDOW_HEIGHT - (viewAreaHeight/2);
        cameraPos.y = fixPos;
    }

    //Коллизия цели targetPos
    if(targetPos.x>viewAreaWidth){
        targetPos.x = viewAreaWidth;
    } else if(targetPos.x<(WINDOW_WIDTH-viewAreaWidth)){
        targetPos.x = (WINDOW_WIDTH-viewAreaWidth);
    }
}

GameCamera::GameCamera(sf::View& view, Player& player, GameLevelManager& levelManager)
{
    // Привязываю ссылки к указателям
    this->player = &player;
    this->levelManager = &levelManager;
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
        this->movementUpdate(dt);
        
        this->view->setCenter(this->cameraPos);
        return;
    }
}
