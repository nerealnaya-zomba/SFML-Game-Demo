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
    , lifetime(particleLifetime)
    , maxLifetime(particleLifetime)
    , isAlive(true)
{
    shape.setRadius(particleRadius);
    shape.setFillColor(color);
    shape.setOrigin({particleRadius, particleRadius}); // Центрируем
    shape.setPosition(position);
}

void Particle::update(float deltaTime)
{
    if (!isAlive) return;

    // Уменьшаем время жизни
    lifetime -= deltaTime;
    if (lifetime <= 0.0f) {
        isAlive = false;
        return;
    }

    // Затухание ускорения (очень быстрое)
    currentAcceleration.x *= dampingFactor * deltaTime * 60.0f; // Нормализуем к кадрам в секунду
    currentAcceleration.y *= dampingFactor * deltaTime * 60.0f;

    // Применяем ускорение к скорости
    velocity.x += currentAcceleration.x * deltaTime;
    velocity.y += currentAcceleration.y * deltaTime;

    // Применяем гравитацию
    velocity.y += gravity * deltaTime;

    // Обновляем позицию
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;

    // Обновляем графическую часть
    shape.setPosition(position);
    
    // Изменяем альфа-канал в зависимости от оставшегося времени жизни
    float alpha = (lifetime / maxLifetime) * 255.0f;
    color.a = static_cast<uint8_t>(alpha);
    shape.setFillColor(color);
}

void Particle::draw(sf::RenderWindow& window) const
{
    if (isAlive) {
        window.draw(shape);
    }
}