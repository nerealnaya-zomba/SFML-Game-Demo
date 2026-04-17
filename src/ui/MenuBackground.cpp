#include "MenuBackground.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace
{
constexpr float kPi = 3.14159265f;
}

MenuBackground::MenuBackground(int w, int h)
    : width(w)
    , height(h)
{
    applyTheme();
    createMistBands();
    createParticles(200);
}

void MenuBackground::createParticles(int count)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posXDist(0.f, static_cast<float>(width));
    std::uniform_real_distribution<float> posYDist(0.f, static_cast<float>(height));
    std::uniform_real_distribution<float> phaseDist(0.f, kPi * 2.f);
    std::uniform_real_distribution<float> radiusDist(1.4f, theme_ == MenuBackgroundTheme::Main ? 4.2f : 3.4f);
    std::uniform_real_distribution<float> alphaDist(38.f, theme_ == MenuBackgroundTheme::Main ? 110.f : 88.f);
    std::uniform_real_distribution<float> pulseDist(0.55f, 1.8f);
    std::uniform_real_distribution<float> glowDist(2.2f, 4.6f);
    std::uniform_real_distribution<float> velocityXDist(
        theme_ == MenuBackgroundTheme::Main ? -12.f : -8.f,
        theme_ == MenuBackgroundTheme::Main ? 12.f : 8.f
    );
    std::uniform_real_distribution<float> velocityYDist(
        theme_ == MenuBackgroundTheme::Main ? -18.f : -10.f,
        theme_ == MenuBackgroundTheme::Main ? 6.f : 3.f
    );

    particles.clear();
    particles.reserve(static_cast<std::size_t>(count));

    for (int i = 0; i < count; ++i)
    {
        Particle particle;
        particle.position = {posXDist(gen), posYDist(gen)};
        particle.velocity = {velocityXDist(gen), velocityYDist(gen)};
        particle.radius = radiusDist(gen);
        particle.alpha = alphaDist(gen);
        particle.pulseSpeed = pulseDist(gen);
        particle.phase = phaseDist(gen);
        particle.glowScale = glowDist(gen);
        particles.push_back(particle);
    }
}

void MenuBackground::createMistBands()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> offsetDist(0.18f, 0.82f);
    std::uniform_real_distribution<float> amplitudeDist(28.f, 92.f);
    std::uniform_real_distribution<float> phaseDist(0.f, kPi * 2.f);
    std::uniform_real_distribution<float> widthScaleDist(0.88f, 1.38f);
    std::uniform_real_distribution<float> thicknessDist(
        theme_ == MenuBackgroundTheme::Main ? 120.f : 150.f,
        theme_ == MenuBackgroundTheme::Main ? 250.f : 290.f
    );
    std::uniform_real_distribution<float> speedDist(
        theme_ == MenuBackgroundTheme::Main ? 0.08f : 0.04f,
        theme_ == MenuBackgroundTheme::Main ? 0.22f : 0.09f
    );
    std::uniform_real_distribution<float> alphaDist(
        theme_ == MenuBackgroundTheme::Main ? 22.f : 16.f,
        theme_ == MenuBackgroundTheme::Main ? 46.f : 28.f
    );
    std::uniform_real_distribution<float> rotationDist(
        theme_ == MenuBackgroundTheme::Main ? -8.f : -4.f,
        theme_ == MenuBackgroundTheme::Main ? 7.f : 4.f
    );

    mistBands.clear();
    mistBands.reserve(6);
    for (int i = 0; i < 6; ++i)
    {
        MistBand band;
        band.verticalOffset = offsetDist(gen) * static_cast<float>(height);
        band.amplitude = amplitudeDist(gen);
        band.speed = speedDist(gen);
        band.thickness = thicknessDist(gen);
        band.alpha = alphaDist(gen);
        band.rotation = rotationDist(gen);
        band.phase = phaseDist(gen);
        band.widthScale = widthScaleDist(gen);
        mistBands.push_back(band);
    }
}

void MenuBackground::applyTheme()
{
    if (theme_ == MenuBackgroundTheme::Main)
    {
        topColor_ = sf::Color(10, 5, 8, 255);
        middleColor_ = sf::Color(40, 11, 14, 255);
        bottomColor_ = sf::Color(14, 8, 10, 255);
        accentColor_ = sf::Color(150, 43, 28, 170);
        particleColor_ = sf::Color(238, 184, 122, 255);
        vignetteColor_ = sf::Color(6, 3, 4, 150);
    }
    else
    {
        topColor_ = sf::Color(7, 10, 15, 255);
        middleColor_ = sf::Color(18, 22, 31, 255);
        bottomColor_ = sf::Color(8, 9, 13, 255);
        accentColor_ = sf::Color(100, 122, 138, 120);
        particleColor_ = sf::Color(188, 196, 210, 255);
        vignetteColor_ = sf::Color(3, 5, 8, 165);
    }
}

void MenuBackground::update(float deltaTime)
{
    time_ += deltaTime;

    const float speedMultiplier = (theme_ == MenuBackgroundTheme::Main) ? 1.f : 0.42f;
    const float widthF = static_cast<float>(width);
    const float heightF = static_cast<float>(height);

    for (auto& particle : particles)
    {
        particle.position += particle.velocity * deltaTime * speedMultiplier;
        particle.position.x += std::sin(time_ * particle.pulseSpeed + particle.phase) * 6.f * deltaTime;
        particle.position.y += std::cos(time_ * 0.7f + particle.phase) * 2.5f * deltaTime;

        if (particle.position.x < -30.f)
        {
            particle.position.x = widthF + 30.f;
        }
        else if (particle.position.x > widthF + 30.f)
        {
            particle.position.x = -30.f;
        }

        if (particle.position.y < -30.f)
        {
            particle.position.y = heightF + 30.f;
        }
        else if (particle.position.y > heightF + 30.f)
        {
            particle.position.y = -30.f;
        }
    }
}

void MenuBackground::draw(sf::RenderWindow& window)
{
    drawGradient(window);
    drawMist(window);
    drawSigil(window);
    drawParticles(window);
    drawVignette(window);
}

void MenuBackground::setSize(int w, int h)
{
    if (width == w && height == h)
    {
        return;
    }

    width = std::max(1, w);
    height = std::max(1, h);
    createMistBands();
    createParticles(static_cast<int>(particles.size()));
}

void MenuBackground::drawGradient(sf::RenderWindow& window) const
{
    const float widthF = static_cast<float>(width);
    const float heightF = static_cast<float>(height);
    const float middleY = heightF * 0.56f;

    sf::VertexArray topStrip(sf::PrimitiveType::TriangleStrip, 4);
    topStrip[0].position = {0.f, 0.f};
    topStrip[1].position = {widthF, 0.f};
    topStrip[2].position = {0.f, middleY};
    topStrip[3].position = {widthF, middleY};
    topStrip[0].color = topColor_;
    topStrip[1].color = topColor_;
    topStrip[2].color = middleColor_;
    topStrip[3].color = middleColor_;

    sf::VertexArray bottomStrip(sf::PrimitiveType::TriangleStrip, 4);
    bottomStrip[0].position = {0.f, middleY};
    bottomStrip[1].position = {widthF, middleY};
    bottomStrip[2].position = {0.f, heightF};
    bottomStrip[3].position = {widthF, heightF};
    bottomStrip[0].color = middleColor_;
    bottomStrip[1].color = middleColor_;
    bottomStrip[2].color = bottomColor_;
    bottomStrip[3].color = bottomColor_;

    window.draw(topStrip);
    window.draw(bottomStrip);

    sf::CircleShape bloom(theme_ == MenuBackgroundTheme::Main ? 380.f : 300.f);
    bloom.setOrigin({bloom.getRadius(), bloom.getRadius()});
    bloom.setScale({
        theme_ == MenuBackgroundTheme::Main ? 1.6f : 1.35f,
        theme_ == MenuBackgroundTheme::Main ? 0.95f : 0.8f
    });
    bloom.setPosition({
        widthF * 0.5f,
        theme_ == MenuBackgroundTheme::Main ? heightF * 0.38f : heightF * 0.32f
    });
    bloom.setFillColor(sf::Color(
        accentColor_.r,
        accentColor_.g,
        accentColor_.b,
        theme_ == MenuBackgroundTheme::Main ? 54 : 30
    ));
    window.draw(bloom);
}

void MenuBackground::drawMist(sf::RenderWindow& window) const
{
    const float widthF = static_cast<float>(width);

    for (std::size_t index = 0; index < mistBands.size(); ++index)
    {
        const auto& band = mistBands[index];
        const float oscillation = std::sin(time_ * band.speed + band.phase);
        const float xOffset = oscillation * band.amplitude * (index % 2 == 0 ? 1.f : -1.f);

        sf::RectangleShape mist({widthF * band.widthScale, band.thickness});
        mist.setOrigin({mist.getSize().x / 2.f, mist.getSize().y / 2.f});
        mist.setPosition({
            widthF * 0.5f + xOffset,
            band.verticalOffset + std::cos(time_ * (band.speed * 0.7f) + band.phase) * 18.f
        });
        mist.setRotation(sf::degrees(band.rotation + oscillation * 4.f));
        mist.setFillColor(sf::Color(
            accentColor_.r,
            accentColor_.g,
            accentColor_.b,
            static_cast<std::uint8_t>(band.alpha)
        ));
        window.draw(mist);
    }
}

void MenuBackground::drawSigil(sf::RenderWindow& window) const
{
    const sf::Vector2f center = {
        static_cast<float>(width) * 0.5f,
        theme_ == MenuBackgroundTheme::Main ? static_cast<float>(height) * 0.44f : static_cast<float>(height) * 0.5f
    };

    const float rotation = (theme_ == MenuBackgroundTheme::Main ? 8.f : 2.4f) * time_;
    const float pulse = 0.92f + std::sin(time_ * (theme_ == MenuBackgroundTheme::Main ? 1.4f : 0.7f)) * 0.05f;

    sf::CircleShape outerRing(theme_ == MenuBackgroundTheme::Main ? 250.f : 190.f);
    outerRing.setOrigin({outerRing.getRadius(), outerRing.getRadius()});
    outerRing.setPosition(center);
    outerRing.setScale({pulse * 1.08f, pulse});
    outerRing.setFillColor(sf::Color::Transparent);
    outerRing.setOutlineThickness(theme_ == MenuBackgroundTheme::Main ? 2.f : 1.5f);
    outerRing.setOutlineColor(sf::Color(
        accentColor_.r,
        accentColor_.g,
        accentColor_.b,
        theme_ == MenuBackgroundTheme::Main ? 74 : 42
    ));
    window.draw(outerRing);

    sf::CircleShape innerRing(theme_ == MenuBackgroundTheme::Main ? 170.f : 122.f);
    innerRing.setOrigin({innerRing.getRadius(), innerRing.getRadius()});
    innerRing.setPosition(center);
    innerRing.setScale({pulse, pulse * 0.95f});
    innerRing.setFillColor(sf::Color::Transparent);
    innerRing.setOutlineThickness(1.4f);
    innerRing.setOutlineColor(sf::Color(
        particleColor_.r,
        particleColor_.g,
        particleColor_.b,
        theme_ == MenuBackgroundTheme::Main ? 40 : 28
    ));
    window.draw(innerRing);

    const int spokeCount = theme_ == MenuBackgroundTheme::Main ? 8 : 4;
    for (int i = 0; i < spokeCount; ++i)
    {
        const float angle = rotation + (static_cast<float>(i) / static_cast<float>(spokeCount)) * 360.f;
        sf::RectangleShape spoke({
            theme_ == MenuBackgroundTheme::Main ? 180.f : 120.f,
            theme_ == MenuBackgroundTheme::Main ? 2.f : 1.5f
        });
        spoke.setOrigin({spoke.getSize().x / 2.f, spoke.getSize().y / 2.f});
        spoke.setPosition(center);
        spoke.setRotation(sf::degrees(angle));
        spoke.setFillColor(sf::Color(
            accentColor_.r,
            accentColor_.g,
            accentColor_.b,
            theme_ == MenuBackgroundTheme::Main ? 36 : 22
        ));
        window.draw(spoke);
    }
}

void MenuBackground::drawParticles(sf::RenderWindow& window) const
{
    for (const auto& particle : particles)
    {
        const float alphaPulse = 0.6f + 0.4f * std::sin(time_ * particle.pulseSpeed + particle.phase);
        const std::uint8_t alpha = static_cast<std::uint8_t>(std::clamp(particle.alpha * alphaPulse, 8.f, 255.f));

        sf::CircleShape glow(particle.radius * particle.glowScale);
        glow.setOrigin({glow.getRadius(), glow.getRadius()});
        glow.setPosition(particle.position);
        glow.setFillColor(sf::Color(
            particleColor_.r,
            particleColor_.g,
            particleColor_.b,
            static_cast<std::uint8_t>(alpha * 0.18f)
        ));
        window.draw(glow);

        sf::CircleShape core(particle.radius);
        core.setOrigin({core.getRadius(), core.getRadius()});
        core.setPosition(particle.position);
        core.setFillColor(sf::Color(
            particleColor_.r,
            particleColor_.g,
            particleColor_.b,
            alpha
        ));
        window.draw(core);
    }
}

void MenuBackground::drawVignette(sf::RenderWindow& window) const
{
    sf::RectangleShape topVeil({static_cast<float>(width), static_cast<float>(height) * 0.18f});
    topVeil.setPosition({0.f, 0.f});
    topVeil.setFillColor(vignetteColor_);

    sf::RectangleShape bottomVeil({static_cast<float>(width), static_cast<float>(height) * 0.22f});
    bottomVeil.setPosition({0.f, static_cast<float>(height) - bottomVeil.getSize().y});
    bottomVeil.setFillColor(vignetteColor_);

    sf::RectangleShape leftVeil({static_cast<float>(width) * 0.08f, static_cast<float>(height)});
    leftVeil.setPosition({0.f, 0.f});
    leftVeil.setFillColor(vignetteColor_);

    sf::RectangleShape rightVeil({static_cast<float>(width) * 0.08f, static_cast<float>(height)});
    rightVeil.setPosition({static_cast<float>(width) - rightVeil.getSize().x, 0.f});
    rightVeil.setFillColor(vignetteColor_);

    window.draw(topVeil);
    window.draw(bottomVeil);
    window.draw(leftVeil);
    window.draw(rightVeil);
}

void MenuBackground::setTheme(MenuBackgroundTheme theme)
{
    if (theme_ == theme)
    {
        return;
    }

    theme_ = theme;
    applyTheme();
    createMistBands();
    createParticles(static_cast<int>(particles.size()));
}

void MenuBackground::setColors(const sf::Color& c1, const sf::Color& c2)
{
    topColor_ = c1;
    middleColor_ = sf::Color(
        static_cast<std::uint8_t>((static_cast<int>(c1.r) + static_cast<int>(c2.r)) / 2),
        static_cast<std::uint8_t>((static_cast<int>(c1.g) + static_cast<int>(c2.g)) / 2),
        static_cast<std::uint8_t>((static_cast<int>(c1.b) + static_cast<int>(c2.b)) / 2)
    );
    bottomColor_ = c2;
}

void MenuBackground::setParticleCount(int count)
{
    createParticles(count);
}
