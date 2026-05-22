#include <Background.h>
#include <BackgroundAtmosphere.h>

#include <algorithm>
#include <utility>

class GameCamera;

Background::Background(
    GameData& d,
    GameCamera& c,
    GameLevel& l,
    sf::Vector2f pos,
    std::string bgName,
    sf::Vector2f parallaxFact,
    Type t,
    BackgroundSceneConfig newSceneConfig
)
    : camera(&c)
    , level(&l)
    , type(t)
    , position(pos)
    , name(std::move(bgName))
    , sceneConfig(std::move(newSceneConfig))
    , parallaxFactor(parallaxFact)
{
    sf::Texture& bgTexture = d.backgroundTextures.at(name);
    bgFront = std::make_unique<sf::Sprite>(bgTexture);

    const sf::Vector2f windowSizes = {static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)};
    const sf::Vector2f textureSizes = {
        static_cast<float>(bgTexture.getSize().x),
        static_cast<float>(bgTexture.getSize().y)
    };
    bgFront->setScale({windowSizes.x / textureSizes.x, windowSizes.y / textureSizes.y});
    setSpriteOriginToMiddle(*bgFront);
    bgFront->setPosition(pos);

    bgTextureSize = bgTexture.getSize();
    atmosphere = std::make_unique<BackgroundAtmosphere>(
        sceneConfig.themeName,
        sceneConfig.layerIndex,
        sceneConfig.layerCount,
        parallaxFact
    );
}

Background::~Background() = default;

void Background::update()
{
    const float deltaTime = std::clamp(animationClock.restart().asSeconds(), 0.0001f, 0.05f);

    if (atmosphere)
    {
        atmosphere->update(deltaTime);
    }

    applyParallax();
}

void Background::draw(sf::RenderWindow &window)
{
    applyParallax();

    if (atmosphere)
    {
        atmosphere->drawBehind(window, getActiveViewRect());
    }

    switch(type)
    {
        case Type::SingleBackground:
            window.draw(*bgFront);
            break;

        case Type::RepeatedBackground:
            drawRepeated(window);
            break;

        default:
            break;
    }

    if (atmosphere)
    {
        atmosphere->drawOverlay(window, getActiveViewRect());
    }
}

void Background::applyParallax()
{
    const sf::Vector2f baseObjectPos = position;
    const sf::Vector2f cameraOffset = camera->getCameraCenterPos() - BASE_CAMERAPOS;
    const sf::Vector2f viewCompensatedParallax = {
        1.f - parallaxFactor.x,
        1.f - parallaxFactor.y
    };

    bgFront->setPosition({
        baseObjectPos.x + cameraOffset.x * viewCompensatedParallax.x,
        baseObjectPos.y + cameraOffset.y * viewCompensatedParallax.y
    });
}

sf::FloatRect Background::getActiveViewRect() const
{
    const sf::Vector2f viewSize = camera->getScreenViewSize();
    const float padding = 160.f;
    const sf::Vector2f finalSize = {viewSize.x + padding * 2.f, viewSize.y + padding * 2.f};
    const sf::Vector2f topLeft = {
        camera->getCameraCenterPos().x - finalSize.x / 2.f,
        camera->getCameraCenterPos().y - finalSize.y / 2.f
    };

    return {topLeft, finalSize};
}

void Background::drawRepeated(sf::RenderWindow& window) const
{
    const sf::FloatRect bounds = bgFront->getGlobalBounds();
    const float repeatedWidth = bounds.size.x;
    const float repeatedHeight = bounds.size.y;
    if (repeatedWidth <= 0.f || repeatedHeight <= 0.f)
    {
        window.draw(*bgFront);
        return;
    }

    const sf::FloatRect viewRect = getActiveViewRect();
    const sf::Vector2f center = bgFront->getPosition();
    const float leftEdge = center.x - repeatedWidth / 2.f;
    const float topEdge = center.y - repeatedHeight / 2.f;
    const float rightView = viewRect.position.x + viewRect.size.x;
    const float bottomView = viewRect.position.y + viewRect.size.y;

    const int startX = static_cast<int>(std::floor((viewRect.position.x - leftEdge) / repeatedWidth)) - 1;
    const int endX = static_cast<int>(std::ceil((rightView - leftEdge) / repeatedWidth)) + 1;
    const int startY = static_cast<int>(std::floor((viewRect.position.y - topEdge) / repeatedHeight)) - 1;
    const int endY = static_cast<int>(std::ceil((bottomView - topEdge) / repeatedHeight)) + 1;

    for (int y = startY; y <= endY; ++y)
    {
        for (int x = startX; x <= endX; ++x)
        {
            sf::Sprite repeatedSprite = *bgFront;
            repeatedSprite.setPosition({
                center.x + repeatedWidth * static_cast<float>(x),
                center.y + repeatedHeight * static_cast<float>(y)
            });
            window.draw(repeatedSprite);
        }
    }
}

void Background::setParallaxFactor(sf::Vector2f f)
{
    this->parallaxFactor = f;
}

sf::Sprite &Background::getSprite()
{
    return *bgFront;
}
