#pragma once
#include <SFML/Graphics.hpp>

class Particle {
private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f initialAcceleration;
    sf::Vector2f currentAcceleration;
    sf::Color color;
    sf::CircleShape shape;
    sf::Clock lifeClock; // Таймер жизни частицы
    
    // Физические параметры
    float dampingFactor;
    float gravity;
    float maxLifetime;
    bool isAlive;
    
public:
    Particle(const sf::Vector2f& startPosition, 
             const sf::Vector2f& initialVelocity,
             const sf::Vector2f& startAcceleration,
             const sf::Color& particleColor = sf::Color::White,
             float particleRadius = 2.0f,
             float gravityForce = 98.0f,
             float accelerationDamping = 0.85f,
             float particleLifetime = 2.0f);

    void update(); // Убрал deltaTime из параметров
    void draw(sf::RenderWindow& window) const;
    
    bool getIsAlive() const { return isAlive; }
    sf::Vector2f getPosition() const { return position; }
    
    void setColor(const sf::Color& newColor) { color = newColor; shape.setFillColor(color); }
    void setGravity(float newGravity) { gravity = newGravity; }
    void setDamping(float newDamping) { dampingFactor = newDamping; }
};