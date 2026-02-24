#include "MenuBackground.h"
#include <random>
#include <iostream>

MenuBackground::MenuBackground()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDistX(100.f, SCREEN_WIDTH - 100.f);
    std::uniform_real_distribution<float> posDistY(100.f, SCREEN_HEIGHT - 100.f);
    std::uniform_real_distribution<float> velDist(-200.f, 200.f);
    std::uniform_real_distribution<float> angVelDist(-2.f, 2.f); // Уменьшил диапазон
    std::uniform_int_distribution<int> colorDist(100, 255);
    
    objects.resize(BASE_MENUBACKGROUND_COUNT_OF_SHAPES);
    
    for (auto& obj : objects) {
        sf::Vector2f pos(posDistX(gen), posDistY(gen));
        sf::Color color(colorDist(gen), colorDist(gen), colorDist(gen), 200);
        
        initTriangle(obj, pos, TRIANGLE_SIZE, color);
        
        obj.velocity = sf::Vector2f(velDist(gen), velDist(gen));
        obj.angularVelocity = angVelDist(gen);
        obj.active = true;
        
        // Сразу применяем ограничение скорости
        clampVelocity(obj);
    }
}

void MenuBackground::initTriangle(PhysicsObject& obj, const sf::Vector2f& pos, float size, const sf::Color& color)
{
    obj.position = pos;
    obj.angle = 0.f;
    obj.angularVelocity = 0.f;
    obj.mass = size * size; // масса пропорциональна площади
    
    // Создаём равносторонний треугольник
    float height = size * std::sqrt(3.f) / 2.f;
    
    // Вершины в локальных координатах (центр масс в центре треугольника)
    obj.localVertices[0] = sf::Vector2f(0.f, -height * 2.f / 3.f);          // верх
    obj.localVertices[1] = sf::Vector2f(-size / 2.f, height / 3.f);         // левый низ
    obj.localVertices[2] = sf::Vector2f(size / 2.f, height / 3.f);          // правый низ
    
    // Момент инерции для равностороннего треугольника относительно центра масс
    // I = (1/18) * m * (a^2 + b^2 + c^2) для треугольника, для равностороннего: I = (1/12) * m * side^2
    obj.momentOfInertia = obj.mass * size * size / 12.f;
    
    obj.shape.setPointCount(3);
    for (int i = 0; i < 3; ++i) {
        obj.shape.setPoint(i, obj.localVertices[i]);
    }
    
    obj.shape.setFillColor(color);
    obj.shape.setOutlineColor(sf::Color::White);
    obj.shape.setOutlineThickness(2.f);
    
    updateTransform(obj);
}

void MenuBackground::updateTransform(PhysicsObject& obj)
{
    obj.shape.setPosition(obj.position);
    obj.shape.setRotation(sf::degrees(obj.angle * 180.f / 3.14159f)); // Конвертируем радианы в градусы для SFML
}

void MenuBackground::update(float deltaTime)
{
    // Применяем физику
    for (auto& obj : objects) {
        if (!obj.active) continue;
        
        // Обновляем позицию
        obj.position += obj.velocity * deltaTime;
        
        // Обновляем вращение (угол в радианах)
        obj.angle += obj.angularVelocity * deltaTime;
        
        // Применяем трение
        obj.velocity *= FRICTION;
        obj.angularVelocity *= ANGULAR_FRICTION;
        
        // Проверяем коллизии со стенами
        applyBoundaryCollision(obj);
        
        // Ограничиваем скорость ПОСЛЕ всех изменений
        clampVelocity(obj);
        
        // Если скорость падает ниже минимума - добавляем случайный импульс
        float speed = std::sqrt(obj.velocity.x * obj.velocity.x + obj.velocity.y * obj.velocity.y);
        if (speed < MIN_VELOCITY) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<float> dist(-100.f, 100.f);
            
            obj.velocity += sf::Vector2f(dist(gen), dist(gen));
            
            // Снова ограничиваем после добавления импульса
            clampVelocity(obj);
        }
        
        // Обновляем трансформацию фигуры
        updateTransform(obj);
    }
    
    // Проверяем коллизии между треугольниками
    checkCollisions();
}

void MenuBackground::applyBoundaryCollision(PhysicsObject& obj)
{
    sf::FloatRect bounds = obj.shape.getGlobalBounds();
    
    // Коллизия с левой стеной
    if (bounds.position.x < 0.f) {
        obj.position.x += -bounds.position.x;
        obj.velocity.x = -obj.velocity.x * RESTITUTION;
        obj.angularVelocity += obj.velocity.y * 0.0001f;
    }
    // Коллизия с правой стеной
    else if (bounds.position.x + bounds.size.x > SCREEN_WIDTH) {
        obj.position.x -= (bounds.position.x + bounds.size.x - SCREEN_WIDTH);
        obj.velocity.x = -obj.velocity.x * RESTITUTION;
        obj.angularVelocity -= obj.velocity.y * 0.0001f;
    }
    
    // Коллизия с верхней стеной
    if (bounds.position.y < 0.f) {
        obj.position.y += -bounds.position.y;
        obj.velocity.y = -obj.velocity.y * RESTITUTION;
        obj.angularVelocity -= obj.velocity.x * 0.0001f;
    }
    // Коллизия с нижней стеной
    else if (bounds.position.y + bounds.size.y > SCREEN_HEIGHT) {
        obj.position.y -= (bounds.position.y + bounds.size.y - SCREEN_HEIGHT);
        obj.velocity.y = -obj.velocity.y * RESTITUTION;
        obj.angularVelocity += obj.velocity.x * 0.0001f;
    }
    
    // Ограничиваем скорость после столкновения со стеной
    clampVelocity(obj);
}

bool MenuBackground::checkTriangleCollision(const PhysicsObject& a, const PhysicsObject& b, 
                                           sf::Vector2f& collisionNormal, float& penetration)
{
    // Упрощённая проверка коллизий через SAT (Separating Axis Theorem)
    // Для более точной физики нужна полная реализация SAT
    
    // Для простоты используем bounding box для быстрой проверки
    sf::FloatRect boundsA = a.shape.getGlobalBounds();
    sf::FloatRect boundsB = b.shape.getGlobalBounds();
    
    if (!boundsA.findIntersection(boundsB))
        return false;
    
    // В реальной SAT нужно проверять все оси (нормали граней)
    // Здесь упрощённая версия - проверяем расстояние между центрами
    sf::Vector2f delta = a.position - b.position;
    float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    float minDistance = TRIANGLE_SIZE; // приблизительное минимальное расстояние
    
    if (distance < minDistance && distance > 0.1f) {
        collisionNormal = delta / distance;
        penetration = minDistance - distance;
        return true;
    }
    
    return false;
}

void MenuBackground::clampVelocity(PhysicsObject& obj)
{
    // Ограничиваем линейную скорость
    float speed = std::sqrt(obj.velocity.x * obj.velocity.x + obj.velocity.y * obj.velocity.y);
    if (speed > MAX_VELOCITY) {
        obj.velocity = (obj.velocity / speed) * MAX_VELOCITY;
    }
    
    // Ограничиваем угловую скорость
    if (std::abs(obj.angularVelocity) > MAX_ANGULAR_VELOCITY) {
        obj.angularVelocity = (obj.angularVelocity > 0 ? MAX_ANGULAR_VELOCITY : -MAX_ANGULAR_VELOCITY);
    }
}

void MenuBackground::resolveCollision(PhysicsObject& a, PhysicsObject& b, 
                                     const sf::Vector2f& normal, float penetration)
{
    // Разделяем объекты
    if (penetration > 0) {
        sf::Vector2f correction = normal * (penetration * 0.5f);
        a.position += correction;
        b.position -= correction;
    }
    
    // Относительная скорость
    sf::Vector2f relativeVelocity = a.velocity - b.velocity;
    float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;
    
    // Если объекты удаляются друг от друга, не обрабатываем коллизию
    if (velocityAlongNormal > 0)
        return;
    
    // Коэффициент восстановления
    float e = RESTITUTION;
    
    // Импульс
    float impulseMagnitude = -(1 + e) * velocityAlongNormal;
    impulseMagnitude /= (1 / a.mass + 1 / b.mass);
    
    // Применяем импульс
    sf::Vector2f impulse = normal * impulseMagnitude;
    a.velocity += impulse / a.mass;
    b.velocity -= impulse / b.mass;
    
    // Добавляем вращение от удара
    sf::Vector2f contactPointA = (a.position + b.position) * 0.5f - a.position;
    sf::Vector2f contactPointB = (a.position + b.position) * 0.5f - b.position;
    
    float torqueA = (contactPointA.x * impulse.y - contactPointA.y * impulse.x) * 0.1f;
    float torqueB = (contactPointB.x * -impulse.y - contactPointB.y * -impulse.x) * 0.1f;
    
    a.angularVelocity += torqueA / a.momentOfInertia;
    b.angularVelocity += torqueB / b.momentOfInertia;
    
    // Ограничиваем скорость после коллизии
    clampVelocity(a);
    clampVelocity(b);
}

void MenuBackground::checkCollisions()
{
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = i + 1; j < objects.size(); ++j) {
            sf::Vector2f normal;
            float penetration;
            
            if (checkTriangleCollision(objects[i], objects[j], normal, penetration)) {
                resolveCollision(objects[i], objects[j], normal, penetration);
            }
        }
    }
}

void MenuBackground::draw(sf::RenderWindow& window)
{
    for (auto& obj : objects) {
        window.draw(obj.shape);
    }
}