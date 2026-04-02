#include "BackgroundAtmosphere.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <random>

namespace
{
constexpr float kPi = 3.14159265f;

float clamp01(float value)
{
    return std::clamp(value, 0.f, 1.f);
}

float wrap01(float value)
{
    while (value < 0.f)
    {
        value += 1.f;
    }

    while (value >= 1.f)
    {
        value -= 1.f;
    }

    return value;
}

std::uint8_t toChannel(float value)
{
    return static_cast<std::uint8_t>(std::clamp(value, 0.f, 255.f));
}

sf::Color lerpColor(const sf::Color& from, const sf::Color& to, float t)
{
    t = clamp01(t);
    return sf::Color(
        toChannel(static_cast<float>(from.r) + (static_cast<float>(to.r) - static_cast<float>(from.r)) * t),
        toChannel(static_cast<float>(from.g) + (static_cast<float>(to.g) - static_cast<float>(from.g)) * t),
        toChannel(static_cast<float>(from.b) + (static_cast<float>(to.b) - static_cast<float>(from.b)) * t),
        toChannel(static_cast<float>(from.a) + (static_cast<float>(to.a) - static_cast<float>(from.a)) * t)
    );
}

std::mt19937 makeRng(int themeValue, std::size_t layerIndex, std::size_t layerCount, sf::Vector2f parallaxFactor)
{
    std::seed_seq seed{
        themeValue,
        static_cast<int>(layerIndex),
        static_cast<int>(layerCount),
        static_cast<int>(parallaxFactor.x * 1000.f),
        static_cast<int>(parallaxFactor.y * 1000.f)
    };

    return std::mt19937(seed);
}
}

BackgroundAtmosphere::BackgroundAtmosphere(
    const std::string& themeName,
    std::size_t newLayerIndex,
    std::size_t newLayerCount,
    sf::Vector2f newParallaxFactor
)
    : theme(parseTheme(themeName))
    , layerIndex(newLayerIndex)
    , layerCount(std::max<std::size_t>(newLayerCount, 1))
    , parallaxFactor(newParallaxFactor)
{
    configureTheme();
}

void BackgroundAtmosphere::update(float deltaTime)
{
    time += deltaTime;
    updateParticles(deltaTime);
    updateRain(deltaTime);
}

void BackgroundAtmosphere::drawBehind(sf::RenderWindow& window, const sf::FloatRect& viewRect) const
{
    if (gradientEnabled)
    {
        drawGradient(window, viewRect);
    }

    if (celestialEnabled)
    {
        drawCelestial(window, viewRect);
    }

    if (mistEnabled && !isFrontLayer())
    {
        drawMist(window, viewRect, isBackLayer() ? 1.f : 0.65f);
    }
}

void BackgroundAtmosphere::drawOverlay(sf::RenderWindow& window, const sf::FloatRect& viewRect) const
{
    if (mistEnabled && isFrontLayer())
    {
        drawMist(window, viewRect, 0.85f);
    }

    if (particlesEnabled)
    {
        drawParticles(window, viewRect);
    }

    if (rainEnabled)
    {
        drawRain(window, viewRect);
    }

    if (flashEnabled)
    {
        drawFlash(window, viewRect);
    }
}

BackgroundAtmosphere::Theme BackgroundAtmosphere::parseTheme(const std::string& themeName) const
{
    std::string normalized = themeName;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char symbol) {
        return static_cast<char>(std::tolower(symbol));
    });

    if (normalized == "stormfront" || normalized == "storm")
    {
        return Theme::StormFront;
    }

    if (normalized == "bloodmoon" || normalized == "crimson")
    {
        return Theme::BloodMoon;
    }

    if (normalized == "bonecrypt" || normalized == "crypt")
    {
        return Theme::BoneCrypt;
    }

    if (normalized == "twilightrift" || normalized == "twilight")
    {
        return Theme::TwilightRift;
    }

    if (normalized == "midnightrain" || normalized == "rain")
    {
        return Theme::MidnightRain;
    }

    return Theme::VerdantDawn;
}

void BackgroundAtmosphere::configureTheme()
{
    const bool backLayer = isBackLayer();
    const bool frontLayer = isFrontLayer();

    int mistCount = backLayer ? 5 : 3;
    int particleCount = 0;
    int rainCount = 0;
    bool risingParticles = false;

    switch (theme)
    {
        case Theme::VerdantDawn:
            topNight = sf::Color(8, 18, 26, 255);
            middleNight = sf::Color(16, 54, 56, 255);
            bottomNight = sf::Color(17, 44, 34, 255);
            topDay = sf::Color(102, 166, 183, 255);
            middleDay = sf::Color(173, 208, 172, 255);
            bottomDay = sf::Color(84, 142, 97, 255);
            accentColor = sf::Color(244, 194, 118, 205);
            hazeColor = sf::Color(148, 207, 170, 255);
            particleColor = sf::Color(244, 239, 170, 255);
            weatherColor = sf::Color(209, 255, 224, 255);
            cycleSpeed = 0.06f;
            cycleBias = 0.55f;
            cycleAmplitude = 0.32f;
            particleCount = frontLayer ? 28 : 12;
            break;

        case Theme::StormFront:
            topNight = sf::Color(7, 13, 22, 255);
            middleNight = sf::Color(17, 28, 45, 255);
            bottomNight = sf::Color(14, 19, 29, 255);
            topDay = sf::Color(42, 55, 78, 255);
            middleDay = sf::Color(58, 71, 96, 255);
            bottomDay = sf::Color(26, 33, 46, 255);
            accentColor = sf::Color(168, 200, 235, 165);
            hazeColor = sf::Color(88, 112, 147, 255);
            particleColor = sf::Color(222, 232, 245, 255);
            weatherColor = sf::Color(183, 205, 228, 255);
            cycleSpeed = 0.025f;
            cycleBias = 0.12f;
            cycleAmplitude = 0.05f;
            mistCount = backLayer ? 6 : 4;
            rainCount = frontLayer ? 92 : 38;
            particleCount = frontLayer ? 10 : 0;
            flashEnabled = true;
            break;

        case Theme::BloodMoon:
            topNight = sf::Color(12, 5, 12, 255);
            middleNight = sf::Color(49, 10, 24, 255);
            bottomNight = sf::Color(28, 8, 14, 255);
            topDay = sf::Color(68, 12, 31, 255);
            middleDay = sf::Color(114, 27, 34, 255);
            bottomDay = sf::Color(51, 12, 18, 255);
            accentColor = sf::Color(233, 89, 80, 200);
            hazeColor = sf::Color(122, 36, 40, 255);
            particleColor = sf::Color(255, 178, 122, 255);
            weatherColor = sf::Color(219, 97, 88, 255);
            cycleSpeed = 0.04f;
            cycleBias = 0.16f;
            cycleAmplitude = 0.08f;
            mistCount = backLayer ? 4 : 3;
            particleCount = frontLayer ? 30 : 16;
            risingParticles = true;
            break;

        case Theme::BoneCrypt:
            topNight = sf::Color(11, 14, 19, 255);
            middleNight = sf::Color(28, 32, 38, 255);
            bottomNight = sf::Color(22, 24, 28, 255);
            topDay = sf::Color(38, 45, 58, 255);
            middleDay = sf::Color(74, 78, 88, 255);
            bottomDay = sf::Color(46, 44, 42, 255);
            accentColor = sf::Color(215, 221, 232, 175);
            hazeColor = sf::Color(128, 134, 145, 255);
            particleColor = sf::Color(210, 208, 198, 255);
            weatherColor = sf::Color(182, 189, 205, 255);
            cycleSpeed = 0.022f;
            cycleBias = 0.11f;
            cycleAmplitude = 0.05f;
            mistCount = backLayer ? 6 : 4;
            particleCount = frontLayer ? 24 : 12;
            break;

        case Theme::TwilightRift:
            topNight = sf::Color(8, 10, 24, 255);
            middleNight = sf::Color(28, 22, 58, 255);
            bottomNight = sf::Color(26, 16, 34, 255);
            topDay = sf::Color(83, 98, 160, 255);
            middleDay = sf::Color(156, 110, 126, 255);
            bottomDay = sf::Color(74, 45, 56, 255);
            accentColor = sf::Color(255, 192, 126, 190);
            hazeColor = sf::Color(164, 120, 168, 255);
            particleColor = sf::Color(231, 227, 255, 255);
            weatherColor = sf::Color(224, 183, 255, 255);
            cycleSpeed = 0.08f;
            cycleBias = 0.38f;
            cycleAmplitude = 0.25f;
            mistCount = backLayer ? 5 : 3;
            particleCount = frontLayer ? 26 : 14;
            break;

        case Theme::MidnightRain:
            topNight = sf::Color(6, 7, 16, 255);
            middleNight = sf::Color(18, 17, 36, 255);
            bottomNight = sf::Color(14, 13, 22, 255);
            topDay = sf::Color(34, 42, 66, 255);
            middleDay = sf::Color(54, 55, 86, 255);
            bottomDay = sf::Color(24, 23, 41, 255);
            accentColor = sf::Color(151, 170, 255, 175);
            hazeColor = sf::Color(82, 88, 132, 255);
            particleColor = sf::Color(203, 212, 255, 255);
            weatherColor = sf::Color(166, 182, 242, 255);
            cycleSpeed = 0.03f;
            cycleBias = 0.13f;
            cycleAmplitude = 0.07f;
            mistCount = backLayer ? 5 : 3;
            rainCount = frontLayer ? 88 : 34;
            particleCount = frontLayer ? 12 : 6;
            flashEnabled = true;
            break;
    }

    gradientEnabled = backLayer;
    celestialEnabled = backLayer;
    mistEnabled = mistCount > 0;
    particlesEnabled = particleCount > 0;
    rainEnabled = rainCount > 0;

    seedMistBands(mistCount);
    seedParticles(particleCount, risingParticles);
    seedRain(rainCount);
}

void BackgroundAtmosphere::seedMistBands(int count)
{
    mistBands.clear();
    if (count <= 0)
    {
        return;
    }

    auto rng = makeRng(static_cast<int>(theme), layerIndex, layerCount, parallaxFactor);
    std::uniform_real_distribution<float> verticalDist(0.16f, 0.84f);
    std::uniform_real_distribution<float> amplitudeDist(0.02f, 0.11f);
    std::uniform_real_distribution<float> speedDist(0.08f, 0.26f);
    std::uniform_real_distribution<float> thicknessDist(0.12f, 0.28f);
    std::uniform_real_distribution<float> alphaDist(18.f, 56.f);
    std::uniform_real_distribution<float> phaseDist(0.f, kPi * 2.f);
    std::uniform_real_distribution<float> rotationDist(-8.f, 8.f);
    std::uniform_real_distribution<float> widthScaleDist(0.9f, 1.45f);

    mistBands.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i)
    {
        MistBand band;
        band.vertical = verticalDist(rng);
        band.amplitude = amplitudeDist(rng);
        band.speed = speedDist(rng);
        band.thickness = thicknessDist(rng);
        band.alpha = alphaDist(rng);
        band.rotation = rotationDist(rng);
        band.phase = phaseDist(rng);
        band.widthScale = widthScaleDist(rng);
        mistBands.push_back(band);
    }
}

void BackgroundAtmosphere::seedParticles(int count, bool rising)
{
    particles.clear();
    if (count <= 0)
    {
        return;
    }

    auto rng = makeRng(static_cast<int>(theme) + 91, layerIndex, layerCount, parallaxFactor);
    std::uniform_real_distribution<float> posDist(0.f, 1.f);
    std::uniform_real_distribution<float> phaseDist(0.f, kPi * 2.f);
    std::uniform_real_distribution<float> radiusDist(1.2f, 4.6f);
    std::uniform_real_distribution<float> alphaDist(30.f, 126.f);
    std::uniform_real_distribution<float> glowDist(2.f, 4.8f);
    std::uniform_real_distribution<float> driftXDist(-0.06f, 0.06f);
    std::uniform_real_distribution<float> driftYDist(rising ? -0.18f : -0.04f, rising ? -0.05f : 0.04f);

    particles.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i)
    {
        DriftParticle particle;
        particle.uv = {posDist(rng), posDist(rng)};
        particle.velocity = {driftXDist(rng), driftYDist(rng)};
        particle.radius = radiusDist(rng);
        particle.alpha = alphaDist(rng);
        particle.phase = phaseDist(rng);
        particle.glowScale = glowDist(rng);
        particles.push_back(particle);
    }
}

void BackgroundAtmosphere::seedRain(int count)
{
    rainDrops.clear();
    if (count <= 0)
    {
        return;
    }

    auto rng = makeRng(static_cast<int>(theme) + 173, layerIndex, layerCount, parallaxFactor);
    std::uniform_real_distribution<float> posDist(0.f, 1.f);
    std::uniform_real_distribution<float> speedDist(0.75f, 1.55f);
    std::uniform_real_distribution<float> lengthDist(0.05f, 0.16f);
    std::uniform_real_distribution<float> alphaDist(50.f, 140.f);
    std::uniform_real_distribution<float> skewDist(0.06f, 0.16f);

    rainDrops.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i)
    {
        RainDrop drop;
        drop.uv = {posDist(rng), posDist(rng)};
        drop.speed = speedDist(rng);
        drop.length = lengthDist(rng);
        drop.alpha = alphaDist(rng);
        drop.skew = skewDist(rng);
        rainDrops.push_back(drop);
    }
}

void BackgroundAtmosphere::updateParticles(float deltaTime)
{
    for (auto& particle : particles)
    {
        particle.uv += particle.velocity * deltaTime;
        particle.uv.x = wrap01(particle.uv.x);
        particle.uv.y = wrap01(particle.uv.y);
    }
}

void BackgroundAtmosphere::updateRain(float deltaTime)
{
    for (auto& drop : rainDrops)
    {
        drop.uv.x = wrap01(drop.uv.x + drop.skew * deltaTime * 0.18f);
        drop.uv.y = wrap01(drop.uv.y + drop.speed * deltaTime);
    }
}

void BackgroundAtmosphere::drawGradient(sf::RenderWindow& window, const sf::FloatRect& viewRect) const
{
    const float cycle = cycleMix();
    const sf::Color top = lerpColor(topNight, topDay, cycle);
    const sf::Color middle = lerpColor(middleNight, middleDay, cycle);
    const sf::Color bottom = lerpColor(bottomNight, bottomDay, cycle);
    const float middleY = viewRect.position.y + viewRect.size.y * 0.58f;

    sf::VertexArray topStrip(sf::PrimitiveType::TriangleStrip, 4);
    topStrip[0].position = viewRect.position;
    topStrip[1].position = {viewRect.position.x + viewRect.size.x, viewRect.position.y};
    topStrip[2].position = {viewRect.position.x, middleY};
    topStrip[3].position = {viewRect.position.x + viewRect.size.x, middleY};
    topStrip[0].color = top;
    topStrip[1].color = top;
    topStrip[2].color = middle;
    topStrip[3].color = middle;

    sf::VertexArray bottomStrip(sf::PrimitiveType::TriangleStrip, 4);
    bottomStrip[0].position = {viewRect.position.x, middleY};
    bottomStrip[1].position = {viewRect.position.x + viewRect.size.x, middleY};
    bottomStrip[2].position = {viewRect.position.x, viewRect.position.y + viewRect.size.y};
    bottomStrip[3].position = {viewRect.position.x + viewRect.size.x, viewRect.position.y + viewRect.size.y};
    bottomStrip[0].color = middle;
    bottomStrip[1].color = middle;
    bottomStrip[2].color = bottom;
    bottomStrip[3].color = bottom;

    window.draw(topStrip);
    window.draw(bottomStrip);

    sf::CircleShape bloom(viewRect.size.y * 0.28f);
    bloom.setOrigin({bloom.getRadius(), bloom.getRadius()});
    bloom.setScale({1.75f, 0.92f});
    bloom.setPosition({
        viewRect.position.x + viewRect.size.x * 0.52f,
        viewRect.position.y + viewRect.size.y * 0.32f
    });
    bloom.setFillColor(sf::Color(
        accentColor.r,
        accentColor.g,
        accentColor.b,
        toChannel(28.f + cycle * 34.f)
    ));
    window.draw(bloom);
}

void BackgroundAtmosphere::drawCelestial(sf::RenderWindow& window, const sf::FloatRect& viewRect) const
{
    const float cycle = cycleMix();

    float xFactor = 0.18f + cycle * 0.64f;
    float yFactor = 0.24f + std::sin(time * 0.15f + static_cast<float>(layerIndex) * 0.5f) * 0.03f;
    sf::Color orbColor = lerpColor(sf::Color(219, 228, 244, 235), sf::Color(255, 210, 145, 245), cycle);
    sf::Color haloColor = sf::Color(accentColor.r, accentColor.g, accentColor.b, 62);

    if (theme == Theme::BloodMoon)
    {
        xFactor = 0.72f;
        yFactor = 0.24f;
        orbColor = sf::Color(235, 108, 90, 242);
        haloColor = sf::Color(188, 48, 44, 72);
    }
    else if (theme == Theme::BoneCrypt)
    {
        xFactor = 0.78f;
        yFactor = 0.22f;
        orbColor = sf::Color(224, 228, 232, 238);
        haloColor = sf::Color(157, 168, 182, 56);
    }
    else if (theme == Theme::StormFront || theme == Theme::MidnightRain)
    {
        xFactor = 0.72f;
        yFactor = 0.20f;
        orbColor = sf::Color(214, 224, 240, 225);
        haloColor = sf::Color(130, 155, 204, 42);
    }

    const sf::Vector2f center = {
        viewRect.position.x + viewRect.size.x * xFactor,
        viewRect.position.y + viewRect.size.y * yFactor
    };

    sf::CircleShape halo(viewRect.size.y * 0.12f);
    halo.setOrigin({halo.getRadius(), halo.getRadius()});
    halo.setPosition(center);
    halo.setScale({1.9f, 1.18f});
    halo.setFillColor(haloColor);
    window.draw(halo);

    sf::CircleShape core(viewRect.size.y * 0.06f);
    core.setOrigin({core.getRadius(), core.getRadius()});
    core.setPosition(center);
    core.setFillColor(orbColor);
    window.draw(core);
}

void BackgroundAtmosphere::drawMist(sf::RenderWindow& window, const sf::FloatRect& viewRect, float alphaMultiplier) const
{
    for (std::size_t index = 0; index < mistBands.size(); ++index)
    {
        const MistBand& band = mistBands[index];
        const float oscillation = std::sin(time * band.speed + band.phase);
        const float xOffset = oscillation * band.amplitude * viewRect.size.x * (index % 2 == 0 ? 1.f : -1.f);
        const float yOffset = std::cos(time * band.speed * 0.8f + band.phase) * viewRect.size.y * 0.018f;

        sf::RectangleShape mist({viewRect.size.x * band.widthScale, viewRect.size.y * band.thickness});
        mist.setOrigin({mist.getSize().x / 2.f, mist.getSize().y / 2.f});
        mist.setPosition({
            viewRect.position.x + viewRect.size.x * 0.5f + xOffset,
            viewRect.position.y + band.vertical * viewRect.size.y + yOffset
        });
        mist.setRotation(sf::degrees(band.rotation + oscillation * 6.f));
        mist.setFillColor(sf::Color(
            hazeColor.r,
            hazeColor.g,
            hazeColor.b,
            toChannel(band.alpha * alphaMultiplier)
        ));
        window.draw(mist);
    }
}

void BackgroundAtmosphere::drawParticles(sf::RenderWindow& window, const sf::FloatRect& viewRect) const
{
    const float themePulse = 0.5f + 0.5f * std::sin(time * 0.7f);

    for (const auto& particle : particles)
    {
        const sf::Vector2f position = {
            viewRect.position.x + viewRect.size.x * particle.uv.x
                + std::sin(time * 0.8f + particle.phase) * 8.f,
            viewRect.position.y + viewRect.size.y * particle.uv.y
                + std::cos(time * 0.65f + particle.phase) * 6.f
        };

        const float alphaPulse = 0.55f + 0.45f * std::sin(time * 1.6f + particle.phase);
        const float alpha = std::clamp(particle.alpha * alphaPulse * (0.75f + themePulse * 0.25f), 8.f, 255.f);

        sf::CircleShape glow(particle.radius * particle.glowScale);
        glow.setOrigin({glow.getRadius(), glow.getRadius()});
        glow.setPosition(position);
        glow.setFillColor(sf::Color(
            particleColor.r,
            particleColor.g,
            particleColor.b,
            toChannel(alpha * 0.18f)
        ));
        window.draw(glow);

        sf::CircleShape core(particle.radius);
        core.setOrigin({core.getRadius(), core.getRadius()});
        core.setPosition(position);
        core.setFillColor(sf::Color(
            particleColor.r,
            particleColor.g,
            particleColor.b,
            toChannel(alpha)
        ));
        window.draw(core);
    }
}

void BackgroundAtmosphere::drawRain(sf::RenderWindow& window, const sf::FloatRect& viewRect) const
{
    for (const auto& drop : rainDrops)
    {
        sf::RectangleShape streak({2.f, std::max(18.f, viewRect.size.y * drop.length)});
        streak.setOrigin({streak.getSize().x / 2.f, streak.getSize().y / 2.f});
        streak.setPosition({
            viewRect.position.x + viewRect.size.x * drop.uv.x,
            viewRect.position.y + viewRect.size.y * drop.uv.y
        });
        streak.setRotation(sf::degrees(18.f + drop.skew * 140.f));
        streak.setFillColor(sf::Color(
            weatherColor.r,
            weatherColor.g,
            weatherColor.b,
            toChannel(drop.alpha)
        ));
        window.draw(streak);
    }
}

void BackgroundAtmosphere::drawFlash(sf::RenderWindow& window, const sf::FloatRect& viewRect) const
{
    const float flashWaveA = std::sin(time * 0.95f + static_cast<float>(layerIndex) * 0.7f);
    const float flashWaveB = std::sin(time * 3.1f + 1.4f);
    float flash = std::max(0.f, flashWaveA * flashWaveB * 1.9f - 1.08f);
    flash = flash * flash;

    if (flash <= 0.002f)
    {
        return;
    }

    sf::RectangleShape veil(viewRect.size);
    veil.setPosition(viewRect.position);
    veil.setFillColor(sf::Color(
        accentColor.r,
        accentColor.g,
        accentColor.b,
        toChannel(65.f * flash)
    ));
    window.draw(veil);
}

bool BackgroundAtmosphere::isBackLayer() const
{
    return layerIndex == 0;
}

bool BackgroundAtmosphere::isFrontLayer() const
{
    return layerIndex + 1 >= layerCount;
}

float BackgroundAtmosphere::cycleMix() const
{
    const float pulse = 0.5f + 0.5f * std::sin(time * cycleSpeed + static_cast<float>(layerIndex) * 0.4f);
    return clamp01(cycleBias + (pulse - 0.5f) * 2.f * cycleAmplitude);
}
