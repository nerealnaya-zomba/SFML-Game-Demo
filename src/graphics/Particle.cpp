#include "Particle.h"

Particle::Particle(const sf::Vector2f& startPosition, 
                   const sf::Vector2f& initialVelocity,
                   const sf::Vector2f& startAcceleration,
                   const sf::Color& particleColor,
                   float particleRadius,
                   float gravityForce,
                   float accelerationDamping,
                   float particleLifetime)
    : position(startPosition)
    , velocity(initialVelocity)
    , initialAcceleration(startAcceleration)
    , currentAcceleration(startAcceleration)
    , color(particleColor)
    , dampingFactor(accelerationDamping)
    , gravity(gravityForce)
    , maxLifetime(particleLifetime)
    , isAlive(true)
{
    shape.setRadius(particleRadius);
    shape.setFillColor(color);
    shape.setOrigin({particleRadius, particleRadius});
    shape.setPosition(position);
    
    lifeClock.restart(); // Запускаем таймер при создании
}

void Particle::update()
{
    if (!isAlive) return;

    // Получаем время жизни частицы
    float lifetime = lifeClock.getElapsedTime().asSeconds();
    
    // Проверяем время жизни
    if (lifetime >= maxLifetime) {
        isAlive = false;
        return;
    }

    // Получаем дельту времени для плавного движения
    float deltaTime = 1.0f / 60.0f; // Предполагаем 60 FPS

    // Затухание ускорения
    currentAcceleration.x *= dampingFactor * deltaTime * 60.0f;
    currentAcceleration.y *= dampingFactor * deltaTime * 60.0f;

    // Применяем ускорение к скорости
    velocity.x += currentAcceleration.x * deltaTime;
    velocity.y += currentAcceleration.y * deltaTime;

    // Применяем гравитацию
    velocity.y += gravity * deltaTime;

    // Обновляем позицию
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;

    // Обновляем графику
    shape.setPosition(position);
    
    // Плавное исчезновение
    float lifeRatio = 1.0f - (lifetime / maxLifetime);
    color.a = static_cast<uint8_t>(lifeRatio * 255.0f);
    shape.setFillColor(color);
}

void Particle::draw(sf::RenderWindow& window) const
{
    if (isAlive) {
        window.draw(shape);
    }
}