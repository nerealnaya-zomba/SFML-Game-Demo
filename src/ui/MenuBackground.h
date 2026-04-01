#pragma once
#include <SFML/Graphics.hpp>

#include <vector>

enum class MenuBackgroundTheme
{
    Main,
    Pause
};

class MenuBackground
{
private:
    struct Particle
    {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float radius;
        float alpha;
        float pulseSpeed;
        float phase;
        float glowScale;
    };

    struct MistBand
    {
        float verticalOffset;
        float amplitude;
        float speed;
        float thickness;
        float alpha;
        float rotation;
        float phase;
        float widthScale;
    };

    std::vector<Particle> particles;
    std::vector<MistBand> mistBands;

    sf::Color topColor_;
    sf::Color middleColor_;
    sf::Color bottomColor_;
    sf::Color accentColor_;
    sf::Color particleColor_;
    sf::Color vignetteColor_;

    float time_ = 0.f;
    int width = 0;
    int height = 0;
    MenuBackgroundTheme theme_ = MenuBackgroundTheme::Main;

    void createParticles(int count);
    void createMistBands();
    void applyTheme();
    void drawGradient(sf::RenderWindow& window) const;
    void drawMist(sf::RenderWindow& window) const;
    void drawSigil(sf::RenderWindow& window) const;
    void drawParticles(sf::RenderWindow& window) const;
    void drawVignette(sf::RenderWindow& window) const;

public:
    MenuBackground(int w, int h);

    void update(float deltaTime);
    void draw(sf::RenderWindow& window);

    void setTheme(MenuBackgroundTheme theme);
    void setColors(const sf::Color& c1, const sf::Color& c2);
    void setParticleCount(int count);
};
