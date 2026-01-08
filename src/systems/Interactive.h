#pragma once
#include<SFML/Graphics.hpp>
#include<iostream>
#include<Mounting.h>

const float BASE_OFFSET_TO_INTERACT = 0.f;
const sf::Texture errorTexture("images/error.png");

// Базовый абстрактный класс для всех интерактивных объектов
class InteractiveObject {
protected:
    sf::Vector2f basePosition;
    float offsetToInteract;
    bool isCanInteract;
    
    std::shared_ptr<sf::Sprite> sprite;

public:
    InteractiveObject(const sf::Vector2f& pos)
        : basePosition(pos),offsetToInteract(BASE_OFFSET_TO_INTERACT), isCanInteract(false) { sprite = std::make_shared<sf::Sprite>(errorTexture); }
    
    virtual ~InteractiveObject() = default;
    
    // Виртуальные методы, которые должны быть реализованы в производных классах
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void update() = 0;
    virtual void handleEvent(const sf::Event& event) = 0;

    
    
    // Общие методы для всех интерактивных объектов
    bool isInAreaOfInteraction(const sf::Vector2f& executorObjectPos) const{
        sf::Vector2f portalPos = sprite->getGlobalBounds().getCenter();
        sf::Vector2u portalSize = sprite->getTexture().getSize();
        
        // Проверяем в радиусе взаимодействия ли объект
        if(executorObjectPos.x >= (portalPos.x - ((portalSize.x/2) + offsetToInteract)) &&
            executorObjectPos.x <= (portalPos.x + ((portalSize.x/2) + offsetToInteract)) &&
            executorObjectPos.y >= (portalPos.y - ((portalSize.y/2) + offsetToInteract)) && 
            executorObjectPos.y <= (portalPos.y + ((portalSize.y/2) + offsetToInteract)))
            {
                return true;
            }
        return false;
    }

    void setPosition(const sf::Vector2f& newPos) {
        setSpriteOriginToMiddle(*sprite);
        sprite->setPosition(newPos);
        onPositionChanged();
    }
    
    sf::Vector2f getPosition() const { return basePosition; }
    
    // Виртуальные методы-обработчики событий
    virtual void onClick() {
        std::cout << "Clicked! \n";
    }
    
    virtual void onInteractionEnter() {
        isCanInteract = true;
    }
    
    virtual void onInteractionExit() {
        isCanInteract = false;
    }
    
protected:
    // Методы, которые могут быть переопределены в производных классах
    virtual void onPositionChanged() {}
    virtual void onActiveChanged() {}
};