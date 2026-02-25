#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include<GameCamera.h>

class ScreenTransition {
private:
    sf::RenderWindow& window;
    sf::RectangleShape overlay;
    sf::Clock clock;
    GameCamera* camera;
    
    // Параметры анимации
    float duration;
    float currentAlpha;
    bool isTransitioning;
    bool fadingOut;  // true - затемнение, false - появление
    
    // Дополнительные эффекты для красоты
    sf::CircleShape vignette;
    sf::Shader* shader;
    bool useShader;

public:
    ScreenTransition(sf::RenderWindow& win, GameCamera& c, float transitionDuration = 1.0f);
    ~ScreenTransition();
    
    void fadeOut();  // Начать затемнение
    void fadeIn();   // Начать появление
    void update();   // Обновление анимации
    void draw();     // Отрисовка перехода
    
    bool isActive() const;           // Проверка, идет ли переход
    bool isFadeOutComplete() const;  // Проверка, закончилось ли затемнение
    bool isFadeInComplete() const;   // Проверка, закончилось ли появление
    float getProgress() const;       // Получить прогресс перехода (0-1)
};