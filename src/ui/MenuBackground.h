#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class MenuBackground {
private:
    struct Particle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float radius;
        float alpha;
        float pulseSpeed;
    };
    
    std::vector<Particle> particles;
    sf::RenderTexture backgroundTexture;
    std::unique_ptr<sf::Sprite> backgroundSprite; // Используем указатель
    sf::Color color1, color2;
    float time;
    int width, height;
    bool textureCreated;
    
    void createParticles(int count);
    sf::Color interpolateColor(float t);
    
public:
    MenuBackground(int w, int h);
    ~MenuBackground() = default;
    
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    
    // Методы для настройки анимации
    void setColors(const sf::Color& c1, const sf::Color& c2);
    void setParticleCount(int count);
};
