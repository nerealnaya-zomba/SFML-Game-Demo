#include "MenuBackground.h"
#include <cmath>
#include <random>
#include <algorithm>

MenuBackground::MenuBackground(int w, int h) 
    : width(w), 
      height(h), 
      time(0), 
      backgroundTexture(), 
      color1(20, 30, 50),    
      color2(70, 40, 80),    
      textureReady(false),
      firstFrame(true) {
    
    // Создаем текстуру
    textureReady = backgroundTexture.resize(sf::Vector2u(width, height));
    
    if (textureReady) {
        // Инициализируем текстуру чистым цветом
        initializeTexture();
    }
    
    createParticles(200);
}

void MenuBackground::initializeTexture() {
    // Очищаем текстуру прозрачным цветом
    backgroundTexture.clear(sf::Color::Transparent);
    
    // Рисуем начальный градиент
    for (int y = 0; y < height; y += 2) {
        float t = static_cast<float>(y) / height;
        sf::Color color = interpolateColor(t);
        
        sf::RectangleShape line(sf::Vector2f(static_cast<float>(width), 2.0f));
        line.setPosition({0, static_cast<float>(y)});
        line.setFillColor(color);
        backgroundTexture.draw(line);
    }
    
    // Рисуем начальные частицы
    for (const auto& p : particles) {
        sf::CircleShape particle(p.radius);
        particle.setPosition(p.position);
        particle.setFillColor(sf::Color(255, 255, 255, static_cast<uint8_t>(p.alpha)));
        backgroundTexture.draw(particle);
    }
    
    backgroundTexture.display();
    
    // Создаем спрайт с готовой текстурой
    this->backgroundSprite = std::make_unique<sf::Sprite>(backgroundTexture.getTexture());
}

void MenuBackground::createParticles(int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0, 1);
    std::uniform_real_distribution<float> radiusDist(1.0f, 3.0f);
    std::uniform_real_distribution<float> speedDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> pulseDist(0.5f, 2.0f);
    
    particles.clear();
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = sf::Vector2f(posDist(gen) * width, posDist(gen) * height);
        p.velocity = sf::Vector2f(speedDist(gen), speedDist(gen));
        p.radius = radiusDist(gen);
        p.alpha = posDist(gen) * 100 + 50;
        p.pulseSpeed = pulseDist(gen);
        particles.push_back(p);
    }
}

sf::Color MenuBackground::interpolateColor(float t) {
    return sf::Color(
        static_cast<uint8_t>(color1.r + (color2.r - color1.r) * t),
        static_cast<uint8_t>(color1.g + (color2.g - color1.g) * t),
        static_cast<uint8_t>(color1.b + (color2.b - color1.b) * t)
    );
}

void MenuBackground::update(float deltaTime) {
    time += deltaTime * 0.5f;
    
    // Обновляем позиции частиц
    for (auto& p : particles) {
        p.position += p.velocity * deltaTime * 50.0f;
        
        // Плавное появление частиц (избегаем резких скачков)
        p.alpha = 100 + 50 * std::sin(time * p.pulseSpeed);
        
        // Телепортация с плавным появлением
        if (p.position.x < 0) {
            p.position.x = static_cast<float>(width);
            p.alpha = 50; // Начинаем с меньшей яркости
        }
        if (p.position.x > width) {
            p.position.x = 0;
            p.alpha = 50;
        }
        if (p.position.y < 0) {
            p.position.y = static_cast<float>(height);
            p.alpha = 50;
        }
        if (p.position.y > height) {
            p.position.y = 0;
            p.alpha = 50;
        }
    }
    
    // Плавная смена цветов
    if (static_cast<int>(time * 2) % 300 == 0 && !firstFrame) {
        std::swap(color1, color2);
    }
}

void MenuBackground::draw(sf::RenderWindow& window) {
    if (!textureReady) return;
    
    // Для первого кадра используем предварительно инициализированную текстуру
    if (firstFrame) {
        window.draw(*backgroundSprite);
        firstFrame = false;
        return;
    }
    
    // Очищаем текстуру
    backgroundTexture.clear(sf::Color::Transparent);
    
    // Рисуем градиентный фон
    for (int y = 0; y < height; y += 2) {
        float t = static_cast<float>(y) / height;
        sf::Color color = interpolateColor(t);
        
        sf::RectangleShape line(sf::Vector2f(static_cast<float>(width), 2.0f));
        line.setPosition({0, static_cast<float>(y)});
        line.setFillColor(color);
        backgroundTexture.draw(line);
    }
    
    // Рисуем частицы с эффектом свечения
    for (const auto& p : particles) {
        // Основная частица
        sf::CircleShape particle(p.radius);
        particle.setPosition(p.position);
        particle.setFillColor(sf::Color(255, 255, 255, static_cast<uint8_t>(p.alpha)));
        backgroundTexture.draw(particle);
        
        // Эффект свечения
        sf::CircleShape glow(p.radius * 3);
        glow.setPosition(p.position - sf::Vector2f(p.radius * 3, p.radius * 3));
        glow.setFillColor(sf::Color(255, 255, 255, static_cast<uint8_t>(p.alpha * 0.3f)));
        backgroundTexture.draw(glow);
    }
    
    // Добавляем звездочки
    for (int i = 0; i < 50; ++i) {
        float x = std::fmod(time * 20 + i * 50, static_cast<float>(width));
        float y = std::sin(time + static_cast<float>(i)) * 50 + height / 2;
        
        sf::CircleShape star(1.5f);
        star.setPosition({x, y});
        star.setFillColor(sf::Color(255, 255, 200, 150));
        backgroundTexture.draw(star);
    }
    
    backgroundTexture.display();
    
    // Обновляем спрайт
    backgroundSprite->setTexture(backgroundTexture.getTexture());
    
    // Рисуем финальный фон
    window.draw(*backgroundSprite);
}

void MenuBackground::setColors(const sf::Color& c1, const sf::Color& c2) {
    color1 = c1;
    color2 = c2;
}

void MenuBackground::setParticleCount(int count) {
    createParticles(count);
}