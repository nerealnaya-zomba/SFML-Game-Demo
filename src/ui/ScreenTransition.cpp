#include "ScreenTransition.h"
#include <algorithm>
#include<Defines.h>

ScreenTransition::ScreenTransition(sf::RenderWindow& win, GameCamera& c, float transitionDuration) 
    : window(win),camera(&c), duration(transitionDuration), isTransitioning(false), 
      fadingOut(true), currentAlpha(0), useShader(false) {
    
    // Основной оверлей
    overlay.setSize({camera->getScreenViewSize().x*2,camera->getScreenViewSize().y*2});
    overlay.setOrigin(overlay.getGlobalBounds().getCenter());
    overlay.setPosition(camera->getCameraCenterPos());
    overlay.setFillColor(sf::Color(0, 0, 0, 0));
    
    // Виньетка для красивого эффекта
    vignette.setRadius(camera->getScreenViewSize().x * 0.8f);
    vignette.setOrigin({vignette.getRadius(), vignette.getRadius()});
    vignette.setPosition(camera->getCameraCenterPos());
    vignette.setFillColor(sf::Color::Transparent);
    vignette.setOutlineThickness(camera->getScreenViewSize().x * 0.5f);
    vignette.setOutlineColor(sf::Color(0, 0, 0, 0));
    
    // Попытка загрузить шейдер (если есть)
    if (sf::Shader::isAvailable()) {
        shader = new sf::Shader();
        // Можно добавить путь к файлу шейдера
        useShader = false; // Пока отключаем, если нет шейдера
    }
}

ScreenTransition::~ScreenTransition() {
    if (useShader) delete shader;
}

void ScreenTransition::fadeOut() {
    isTransitioning = true;
    fadingOut = true;
    currentAlpha = 0;
    clock.restart();
}

void ScreenTransition::fadeIn() {
    isTransitioning = true;
    fadingOut = false;
    currentAlpha = 255;
    clock.restart();
}

void ScreenTransition::update() {
    if (!isTransitioning) return;
    
    overlay.setPosition(camera->getCameraCenterPos());
    vignette.setPosition(camera->getCameraCenterPos());

    float elapsed = clock.getElapsedTime().asSeconds();
    float progress = std::min(elapsed / duration, 1.0f);
    
    if (fadingOut) {
        // Затемнение: от 0 до 255
        currentAlpha = 255 * progress;
        
        // Закончили затемнение
        if (progress >= 1.0f) {
            isTransitioning = false;
        }
    } else {
        // Появление: от 255 до 0
        currentAlpha = 255 * (1.0f - progress);
        
        // Закончили появление
        if (progress >= 1.0f) {
            isTransitioning = false;
            currentAlpha = 0;
        }
    }
    
    // Обновляем цвета
    overlay.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(currentAlpha)));
    vignette.setOutlineColor(sf::Color(0, 0, 0, static_cast<uint8_t>(currentAlpha * 0.7f)));
}

void ScreenTransition::draw() {
    if (currentAlpha > 0) {
        // Рисуем основной оверлей
        window.draw(overlay);
        
        // Добавляем виньетку для красивого эффекта
        if (currentAlpha < 255) {
            window.draw(vignette);
        }
        
        // Можно добавить эффект пикселизации через шейдер
        if (useShader && shader) {
            shader->setUniform("alpha", currentAlpha / 255.f);
            shader->setUniform("time", clock.getElapsedTime().asSeconds());
            // Применяем шейдер ко всему экрану
            sf::RenderStates states;
            states.shader = shader;
            window.draw(overlay, states);
        }
    }
}

bool ScreenTransition::isActive() const { 
    return isTransitioning; 
}

bool ScreenTransition::isFadeOutComplete() const {
    return !isTransitioning && fadingOut && currentAlpha >= 255;
}

bool ScreenTransition::isFadeInComplete() const {
    return !isTransitioning && !fadingOut && currentAlpha <= 0;
}

float ScreenTransition::getProgress() const {
    if (!isTransitioning) return fadingOut ? 1.0f : 0.0f;
    float elapsed = clock.getElapsedTime().asSeconds();
    return std::min(elapsed / duration, 1.0f);
}