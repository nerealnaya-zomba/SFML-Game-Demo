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
    std::unique_ptr<sf::Sprite> backgroundSprite;
    sf::Color color1, color2;
    float time;
    int width, height;
    bool textureReady;
    bool firstFrame;  // Флаг первого кадра
    
    void createParticles(int count);
    sf::Color interpolateColor(float t);
    void initializeTexture();  // Новый метод для инициализации текстуры
    
public:
    MenuBackground(int w, int h);
    
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    
    void setColors(const sf::Color& c1, const sf::Color& c2);
    void setParticleCount(int count);
};

