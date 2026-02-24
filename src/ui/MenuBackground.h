#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>

const size_t BASE_MENUBACKGROUND_COUNT_OF_SHAPES = 30;
const float SCREEN_WIDTH = 1920.f;
const float SCREEN_HEIGHT = 1080.f;

struct PhysicsObject {
    sf::ConvexShape shape;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float angle;           // текущий угол поворота (в радианах)
    float angularVelocity; // угловая скорость
    float mass;
    float momentOfInertia; // момент инерции
    bool active;
    
    // Вершины треугольника в локальных координатах
    static const int VERTEX_COUNT = 3;
    sf::Vector2f localVertices[VERTEX_COUNT];
};

class MenuBackground
{
public:
    MenuBackground();
    ~MenuBackground() = default;

    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    
private:
    void initTriangle(PhysicsObject& obj, const sf::Vector2f& pos, float size, const sf::Color& color);
    void updateTransform(PhysicsObject& obj);
    void applyBoundaryCollision(PhysicsObject& obj);
    void checkCollisions();
    void resolveCollision(PhysicsObject& a, PhysicsObject& b, const sf::Vector2f& normal, float penetration);
    bool checkTriangleCollision(const PhysicsObject& a, const PhysicsObject& b, sf::Vector2f& collisionNormal, float& penetration);
    void clampVelocity(PhysicsObject& obj); // Новая функция для ограничения скорости
    
    std::vector<PhysicsObject> objects;
    const float TRIANGLE_SIZE = 50.f;
    const float RESTITUTION = 0.7f;           // коэффициент упругости
    const float FRICTION = 1.f;                // трение для линейного движения
    const float ANGULAR_FRICTION = 1.f;      // трение для вращения
    const float MIN_VELOCITY = 20.f;           // минимальная скорость
    const float MAX_VELOCITY = 400.f;          // МАКСИМАЛЬНАЯ СКОРОСТЬ
    const float MAX_ANGULAR_VELOCITY = 15.f;    // максимальная угловая скорость
};