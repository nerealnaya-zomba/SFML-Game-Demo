#include <Background.h>
#include <BackgroundAtmosphere.h>

#include <algorithm>
#include <cmath>
#include <utility>

class GameCamera;

namespace
{
constexpr sf::Vector2f kBackgroundTileSize{
    static_cast<float>(WINDOW_WIDTH),
    static_cast<float>(WINDOW_HEIGHT)
};

sf::Vector2f snapBackgroundCenterToTile(sf::Vector2f position)
{
    const float column = std::max(0.f, std::floor(position.x / kBackgroundTileSize.x));
    const float row = std::max(0.f, std::floor(position.y / kBackgroundTileSize.y));
    return {
        column * kBackgroundTileSize.x + kBackgroundTileSize.x * 0.5f,
        row * kBackgroundTileSize.y + kBackgroundTileSize.y * 0.5f
    };
}
}

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
    const sf::Vector2f baseObjectPos = snapBackgroundCenterToTile(position);
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
    const sf::Vector2i levelSize = level ? level->getLevelSize() : sf::Vector2i{
        static_cast<int>(WINDOW_WIDTH),
        static_cast<int>(WINDOW_HEIGHT)
    };
    const int columns = std::max(1, static_cast<int>(std::ceil(static_cast<float>(std::max(levelSize.x, 0)) / kBackgroundTileSize.x)));
    const int rows = std::max(1, static_cast<int>(std::ceil(static_cast<float>(std::max(levelSize.y, 0)) / kBackgroundTileSize.y)));
    const sf::Vector2f cameraOffset = camera->getCameraCenterPos() - BASE_CAMERAPOS;
    const sf::Vector2f viewCompensatedParallax = {
        1.f - parallaxFactor.x,
        1.f - parallaxFactor.y
    };

    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < columns; ++x)
        {
            sf::Sprite repeatedSprite = *bgFront;
            repeatedSprite.setPosition({
                kBackgroundTileSize.x * (static_cast<float>(x) + 0.5f) + cameraOffset.x * viewCompensatedParallax.x,
                kBackgroundTileSize.y * (static_cast<float>(y) + 0.5f) + cameraOffset.y * viewCompensatedParallax.y
            });
            if (viewRect.findIntersection(repeatedSprite.getGlobalBounds()).has_value())
            {
                window.draw(repeatedSprite);
            }
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
