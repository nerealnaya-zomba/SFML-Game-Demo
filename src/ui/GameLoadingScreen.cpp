#include <GameLoadingScreen.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>

namespace
{
constexpr float kPi = 3.14159265f;

void setTextOriginToTopCenter(sf::Text& text)
{
    const auto bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y});
}

std::string wrapTextToWidth(const sf::Text& prototype, const std::string& source, float maxWidth)
{
    if (source.empty())
    {
        return {};
    }

    sf::Text measuring = prototype;
    std::istringstream stream(source);
    std::string word;
    std::string currentLine;
    std::string wrapped;

    while (stream >> word)
    {
        const std::string candidate = currentLine.empty() ? word : currentLine + " " + word;
        measuring.setString(candidate);

        if (!currentLine.empty() && measuring.getLocalBounds().size.x > maxWidth)
        {
            if (!wrapped.empty())
            {
                wrapped += '\n';
            }
            wrapped += currentLine;
            currentLine = word;
        }
        else
        {
            currentLine = candidate;
        }
    }

    if (!currentLine.empty())
    {
        if (!wrapped.empty())
        {
            wrapped += '\n';
        }
        wrapped += currentLine;
    }

    return wrapped;
}
}

LoadingScreen::LoadingScreen(sf::RenderWindow& window, sf::Font& font)
    : window_m(&window)
    , font_m(&font)
    , background_m(static_cast<int>(window.getSize().x), static_cast<int>(window.getSize().y))
    , titleText_(font)
    , chapterText_(font)
    , statusText_(font)
    , loreText_(font)
    , percentText_(font)
    , counterText_(font)
{
    background_m.setTheme(MenuBackgroundTheme::Main);
    background_m.setColors(sf::Color(7, 5, 10, 255), sf::Color(18, 9, 12, 255));
    background_m.setParticleCount(240);

    loreLines_ = {
        "Ash settles slower when the dead are listening.",
        "Iron sigils glow brightest before the gate gives way.",
        "The roots beneath the ruins still remember every oath.",
        "No pilgrim enters untouched by the red mist.",
        "Every lantern here was lit for someone who never returned."
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posXDist(0.f, static_cast<float>(window.getSize().x));
    std::uniform_real_distribution<float> posYDist(0.f, static_cast<float>(window.getSize().y));
    std::uniform_real_distribution<float> speedDist(18.f, 54.f);
    std::uniform_real_distribution<float> driftDist(-11.f, 11.f);
    std::uniform_real_distribution<float> radiusDist(1.3f, 4.6f);
    std::uniform_real_distribution<float> alphaDist(34.f, 120.f);
    std::uniform_real_distribution<float> phaseDist(0.f, kPi * 2.f);

    embers_.reserve(64);
    for (int index = 0; index < 64; ++index)
    {
        embers_.push_back(Ember{
            .position = {posXDist(gen), posYDist(gen)},
            .speed = speedDist(gen),
            .drift = driftDist(gen),
            .radius = radiusDist(gen),
            .alpha = alphaDist(gen),
            .phase = phaseDist(gen)
        });
    }

    configureText(titleText_, 72, sf::Color(230, 216, 198, 255), 1.22f);
    titleText_.setStyle(sf::Text::Bold);
    titleText_.setString("AWAKENING THE RUINS");

    configureText(chapterText_, 22, sf::Color(191, 153, 118, 235), 1.3f);
    chapterText_.setString("A RED MIST GATHERS AT THE GATE");

    configureText(statusText_, 28, sf::Color(237, 224, 206, 255), 1.05f);
    configureText(loreText_, 22, sf::Color(174, 159, 147, 228), 1.02f);
    configureText(percentText_, 44, sf::Color(255, 230, 200, 255), 1.08f);
    percentText_.setStyle(sf::Text::Bold);
    configureText(counterText_, 20, sf::Color(166, 149, 139, 230), 1.04f);

    updateTexts();
}

LoadingScreen::~LoadingScreen() = default;

void LoadingScreen::configureText(
    sf::Text& text,
    unsigned int size,
    sf::Color fillColor,
    float letterSpacing
) const
{
    text = sf::Text(*font_m);
    text.setCharacterSize(size);
    text.setFillColor(fillColor);
    text.setLetterSpacing(letterSpacing);
}

void LoadingScreen::update(float deltaTime, const LoadingProgressSnapshot& snapshot)
{
    time_m += deltaTime;
    loreTimer_m += deltaTime;

    background_m.update(deltaTime);
    updateEmbers(deltaTime);

    completedOperations_m = snapshot.completedOperations;
    totalOperations_m = snapshot.totalOperations;
    failed_m = snapshot.failed;

    if (!snapshot.stageLabel.empty())
    {
        stageLabel_m = snapshot.stageLabel;
    }
    if (!snapshot.errorMessage.empty())
    {
        errorMessage_m = snapshot.errorMessage;
    }

    if (failed_m)
    {
        stageLabel_m = "The ritual has been severed";
    }
    else if (snapshot.finished && totalOperations_m > 0 && completedOperations_m >= totalOperations_m)
    {
        stageLabel_m = "The gate yields";
    }

    if (totalOperations_m > 0)
    {
        targetProgress_m = static_cast<float>(completedOperations_m) / static_cast<float>(totalOperations_m);
    }
    else if (snapshot.finished && !failed_m)
    {
        targetProgress_m = 1.f;
    }
    else
    {
        targetProgress_m = 0.f;
    }

    const float catchUpSpeed = failed_m ? 9.f : 5.6f;
    displayedProgress_m += (targetProgress_m - displayedProgress_m) * std::min(deltaTime * catchUpSpeed, 1.f);
    displayedProgress_m = std::clamp(displayedProgress_m, 0.f, 1.f);

    if (loreTimer_m >= 4.5f && !loreLines_.empty())
    {
        loreTimer_m = 0.f;
        loreIndex_m = (loreIndex_m + 1) % loreLines_.size();
    }

    updateTexts();
}

void LoadingScreen::updateTexts()
{
    statusText_.setCharacterSize(stageLabel_m.size() > 34 ? 24u : 28u);
    statusText_.setString(stageLabel_m);

    std::ostringstream percentStream;
    if (totalOperations_m > 0)
    {
        percentStream << std::setw(3) << static_cast<int>(std::round(displayedProgress_m * 100.f)) << "%";
    }
    else
    {
        percentStream << "--%";
    }
    percentText_.setString(percentStream.str());

    std::ostringstream counterStream;
    if (totalOperations_m > 0)
    {
        counterStream << completedOperations_m << " / " << totalOperations_m << " seals broken";
    }
    else
    {
        counterStream << "Preparing the outer ward";
    }
    counterText_.setString(counterStream.str());

    const std::string loreSource = failed_m && !errorMessage_m.empty()
        ? errorMessage_m
        : (loreLines_.empty() ? std::string{} : loreLines_[loreIndex_m]);
    const bool loreIsLong = loreSource.size() > 58;
    loreText_.setCharacterSize(failed_m ? (loreIsLong ? 18u : 20u) : (loreIsLong ? 19u : 22u));

    if (failed_m && !errorMessage_m.empty())
    {
        loreText_.setFillColor(sf::Color(235, 150, 140, 240));
        loreText_.setString(wrapTextToWidth(loreText_, loreSource, 680.f));
    }
    else
    {
        loreText_.setFillColor(sf::Color(174, 159, 147, 228));
        loreText_.setString(wrapTextToWidth(loreText_, loreSource, 680.f));
    }
}

void LoadingScreen::updateEmbers(float deltaTime)
{
    const float width = static_cast<float>(window_m->getSize().x);
    const float height = static_cast<float>(window_m->getSize().y);

    for (std::size_t index = 0; index < embers_.size(); ++index)
    {
        auto& ember = embers_[index];
        ember.position.y -= ember.speed * deltaTime;
        ember.position.x += std::sin(time_m * 0.9f + ember.phase) * ember.drift * deltaTime;

        if (ember.position.y < -20.f)
        {
            ember.position.y = height + 20.f;
            ember.position.x = std::fmod(ember.position.x + width * 0.37f, width);
            if (ember.position.x < 0.f)
            {
                ember.position.x += width;
            }
        }
    }
}

void LoadingScreen::drawBackdrop()
{
    window_m->clear(sf::Color(2, 2, 3, 255));
    background_m.draw(*window_m);
}

void LoadingScreen::drawArchitecture()
{
    const float width = static_cast<float>(window_m->getSize().x);
    const float height = static_cast<float>(window_m->getSize().y);

    sf::CircleShape eclipse(200.f);
    eclipse.setOrigin({eclipse.getRadius(), eclipse.getRadius()});
    eclipse.setScale({1.85f, 1.15f});
    eclipse.setPosition({width * 0.5f, height * 0.33f});
    eclipse.setFillColor(sf::Color(150, 44, 31, 56));
    window_m->draw(eclipse);

    sf::CircleShape innerGlow(112.f);
    innerGlow.setOrigin({innerGlow.getRadius(), innerGlow.getRadius()});
    innerGlow.setScale({1.2f, 1.f});
    innerGlow.setPosition({width * 0.5f, height * 0.33f});
    innerGlow.setFillColor(sf::Color(237, 170, 114, 18));
    window_m->draw(innerGlow);

    sf::ConvexShape horizon(8);
    horizon.setPoint(0, {0.f, height * 0.71f});
    horizon.setPoint(1, {width * 0.12f, height * 0.6f});
    horizon.setPoint(2, {width * 0.24f, height * 0.69f});
    horizon.setPoint(3, {width * 0.39f, height * 0.52f});
    horizon.setPoint(4, {width * 0.56f, height * 0.7f});
    horizon.setPoint(5, {width * 0.74f, height * 0.58f});
    horizon.setPoint(6, {width, height * 0.67f});
    horizon.setPoint(7, {width, height});
    horizon.setFillColor(sf::Color(12, 7, 11, 230));
    window_m->draw(horizon);

    sf::RectangleShape ground({width, height * 0.22f});
    ground.setPosition({0.f, height * 0.78f});
    ground.setFillColor(sf::Color(8, 5, 8, 245));
    window_m->draw(ground);

    sf::RectangleShape monolithBase({width * 0.21f, height * 0.19f});
    monolithBase.setOrigin({monolithBase.getSize().x / 2.f, monolithBase.getSize().y});
    monolithBase.setPosition({width * 0.5f, height * 0.73f});
    monolithBase.setFillColor(sf::Color(17, 11, 14, 245));
    monolithBase.setOutlineThickness(2.f);
    monolithBase.setOutlineColor(sf::Color(83, 40, 33, 180));
    window_m->draw(monolithBase);

    sf::ConvexShape monolithSpire(5);
    monolithSpire.setPoint(0, {width * 0.46f, height * 0.73f});
    monolithSpire.setPoint(1, {width * 0.485f, height * 0.44f});
    monolithSpire.setPoint(2, {width * 0.5f, height * 0.26f});
    monolithSpire.setPoint(3, {width * 0.515f, height * 0.44f});
    monolithSpire.setPoint(4, {width * 0.54f, height * 0.73f});
    monolithSpire.setFillColor(sf::Color(10, 6, 9, 252));
    window_m->draw(monolithSpire);

    sf::RectangleShape altarGlow({16.f, height * 0.23f});
    altarGlow.setOrigin({altarGlow.getSize().x / 2.f, altarGlow.getSize().y});
    altarGlow.setPosition({width * 0.5f, height * 0.71f});
    altarGlow.setFillColor(sf::Color(223, 120, 78, 105));
    window_m->draw(altarGlow);

    sf::RectangleShape altarCore({6.f, height * 0.2f});
    altarCore.setOrigin({altarCore.getSize().x / 2.f, altarCore.getSize().y});
    altarCore.setPosition({width * 0.5f, height * 0.71f});
    altarCore.setFillColor(sf::Color(245, 196, 130, 125));
    window_m->draw(altarCore);
}

void LoadingScreen::drawSigils()
{
    const float width = static_cast<float>(window_m->getSize().x);
    const float height = static_cast<float>(window_m->getSize().y);
    const sf::Vector2f panelCenter{width * 0.5f, height * 0.77f};

    const float pulse = 0.94f + std::sin(time_m * 1.4f) * 0.03f;
    const float rotation = time_m * 11.f;

    sf::CircleShape outerRing(128.f);
    outerRing.setOrigin({outerRing.getRadius(), outerRing.getRadius()});
    outerRing.setPosition(panelCenter);
    outerRing.setScale({pulse * 1.28f, pulse * 0.7f});
    outerRing.setFillColor(sf::Color::Transparent);
    outerRing.setOutlineThickness(2.f);
    outerRing.setOutlineColor(sf::Color(143, 58, 43, 58));
    window_m->draw(outerRing);

    sf::CircleShape innerRing(76.f);
    innerRing.setOrigin({innerRing.getRadius(), innerRing.getRadius()});
    innerRing.setPosition(panelCenter);
    innerRing.setScale({pulse, pulse * 0.56f});
    innerRing.setFillColor(sf::Color::Transparent);
    innerRing.setOutlineThickness(1.5f);
    innerRing.setOutlineColor(sf::Color(240, 197, 141, 32));
    window_m->draw(innerRing);

    for (int spoke = 0; spoke < 6; ++spoke)
    {
        const float angle = rotation + (static_cast<float>(spoke) / 6.f) * 360.f;
        sf::RectangleShape line({112.f, 2.f});
        line.setOrigin({line.getSize().x / 2.f, line.getSize().y / 2.f});
        line.setPosition(panelCenter);
        line.setRotation(sf::degrees(angle));
        line.setFillColor(sf::Color(160, 72, 55, 42));
        window_m->draw(line);
    }
}

void LoadingScreen::drawProgressPanel()
{
    const float width = static_cast<float>(window_m->getSize().x);
    const float height = static_cast<float>(window_m->getSize().y);
    const sf::Vector2f panelCenter{width * 0.5f, height * 0.77f};
    const sf::Vector2f panelSize{780.f, 236.f};
    const sf::Vector2f barSize{620.f, 24.f};
    const sf::Vector2f barPosition{panelCenter.x - barSize.x / 2.f, panelCenter.y + 14.f};

    sf::RectangleShape shadow(panelSize);
    shadow.setOrigin({panelSize.x / 2.f, panelSize.y / 2.f});
    shadow.setPosition({panelCenter.x, panelCenter.y + 14.f});
    shadow.setFillColor(sf::Color(0, 0, 0, 120));
    window_m->draw(shadow);

    sf::RectangleShape panel(panelSize);
    panel.setOrigin({panelSize.x / 2.f, panelSize.y / 2.f});
    panel.setPosition(panelCenter);
    panel.setFillColor(sf::Color(12, 8, 10, 228));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(134, 91, 67, 210));
    window_m->draw(panel);

    sf::RectangleShape titleBand({panelSize.x - 24.f, 42.f});
    titleBand.setOrigin({titleBand.getSize().x / 2.f, titleBand.getSize().y / 2.f});
    titleBand.setPosition({panelCenter.x, panelCenter.y - 82.f});
    titleBand.setFillColor(sf::Color(53, 14, 15, 220));
    window_m->draw(titleBand);

    sf::RectangleShape inset({panelSize.x - 32.f, panelSize.y - 74.f});
    inset.setOrigin({inset.getSize().x / 2.f, inset.getSize().y / 2.f});
    inset.setPosition({panelCenter.x, panelCenter.y + 10.f});
    inset.setFillColor(sf::Color(7, 5, 7, 150));
    inset.setOutlineThickness(1.f);
    inset.setOutlineColor(sf::Color(76, 36, 32, 165));
    window_m->draw(inset);

    sf::RectangleShape barBack(barSize);
    barBack.setPosition(barPosition);
    barBack.setFillColor(sf::Color(15, 10, 12, 245));
    barBack.setOutlineThickness(2.f);
    barBack.setOutlineColor(sf::Color(142, 100, 72, 255));
    window_m->draw(barBack);

    float fillWidth = barSize.x * displayedProgress_m;
    if (fillWidth > 0.f && fillWidth < 16.f)
    {
        fillWidth = 16.f;
    }
    if (totalOperations_m == 0 && !failed_m)
    {
        fillWidth = 56.f + std::sin(time_m * 2.8f) * 28.f;
    }
    fillWidth = std::clamp(fillWidth, 0.f, barSize.x);

    sf::RectangleShape barFill({fillWidth, barSize.y});
    barFill.setPosition(barPosition);
    barFill.setFillColor(failed_m ? sf::Color(164, 64, 66, 240) : sf::Color(181, 69, 46, 240));
    window_m->draw(barFill);

    if (!failed_m && fillWidth > 0.f)
    {
        sf::RectangleShape warmCore({fillWidth, std::max(8.f, barSize.y - 10.f)});
        warmCore.setPosition({barPosition.x, barPosition.y + 5.f});
        warmCore.setFillColor(sf::Color(240, 181, 119, 90));
        window_m->draw(warmCore);

        const float shimmerWidth = 110.f;
        const float shimmerX = std::fmod(time_m * 250.f, barSize.x + shimmerWidth) - shimmerWidth;
        if (shimmerX < fillWidth)
        {
            sf::RectangleShape shimmer({std::min(shimmerWidth, fillWidth - std::max(shimmerX, 0.f)), barSize.y});
            shimmer.setPosition({barPosition.x + std::max(shimmerX, 0.f), barPosition.y});
            shimmer.setFillColor(sf::Color(255, 232, 198, 34));
            window_m->draw(shimmer);
        }
    }

    titleText_.setPosition({width * 0.5f, height * 0.14f});
    setTextOriginToMiddle(titleText_);
    window_m->draw(titleText_);

    chapterText_.setPosition({width * 0.5f, height * 0.195f});
    setTextOriginToMiddle(chapterText_);
    window_m->draw(chapterText_);

    statusText_.setPosition({panelCenter.x, panelCenter.y - 34.f});
    setTextOriginToMiddle(statusText_);
    window_m->draw(statusText_);

    percentText_.setPosition({panelCenter.x + 298.f, panelCenter.y - 78.f});
    setTextOriginToMiddle(percentText_);
    window_m->draw(percentText_);

    counterText_.setPosition({panelCenter.x, panelCenter.y + 72.f});
    setTextOriginToMiddle(counterText_);
    window_m->draw(counterText_);

    const bool loreWrapped = loreText_.getString().find('\n') != std::string::npos;
    loreText_.setPosition({panelCenter.x, panelCenter.y + (loreWrapped ? 84.f : 96.f)});
    setTextOriginToTopCenter(loreText_);
    window_m->draw(loreText_);
}

void LoadingScreen::drawEmbers()
{
    for (const auto& ember : embers_)
    {
        const float pulse = 0.58f + 0.42f * std::sin(time_m * 1.6f + ember.phase);
        const auto alpha = static_cast<std::uint8_t>(std::clamp(ember.alpha * pulse, 10.f, 255.f));

        sf::CircleShape glow(ember.radius * 3.4f);
        glow.setOrigin({glow.getRadius(), glow.getRadius()});
        glow.setPosition(ember.position);
        glow.setFillColor(sf::Color(217, 118, 72, static_cast<std::uint8_t>(alpha * 0.18f)));
        window_m->draw(glow);

        sf::CircleShape core(ember.radius);
        core.setOrigin({core.getRadius(), core.getRadius()});
        core.setPosition(ember.position);
        core.setFillColor(sf::Color(246, 199, 135, alpha));
        window_m->draw(core);
    }
}

void LoadingScreen::draw()
{
    drawBackdrop();
    drawArchitecture();
    drawSigils();
    drawProgressPanel();
    drawEmbers();
    window_m->display();
}

bool LoadingScreen::isReadyToClose() const
{
    return std::abs(targetProgress_m - displayedProgress_m) < 0.002f && displayedProgress_m >= 0.995f;
}
