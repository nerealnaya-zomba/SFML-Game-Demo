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

sf::Vector2i backgroundTileIndex(sf::Vector2f position)
{
    return {
        std::max(0, static_cast<int>(std::floor(position.x / kBackgroundTileSize.x))),
        static_cast<int>(std::floor(position.y / kBackgroundTileSize.y))
    };
}

bool repeatsX(const Type type)
{
    return type == Type::RepeatedBackgroundX || type == Type::RepeatedBackgroundXY;
}

bool repeatsY(const Type type)
{
    return type == Type::RepeatedBackgroundY || type == Type::RepeatedBackgroundXY;
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
    repeatedSprite = std::make_unique<sf::Sprite>(bgTexture);

    const sf::Vector2f windowSizes = {static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)};
    const sf::Vector2f textureSizes = {
        static_cast<float>(bgTexture.getSize().x),
        static_cast<float>(bgTexture.getSize().y)
    };
    bgFront->setScale({windowSizes.x / textureSizes.x, windowSizes.y / textureSizes.y});
    setSpriteOriginToMiddle(*bgFront);
    bgFront->setPosition(pos);
    repeatedSprite->setScale(bgFront->getScale());
    repeatedSprite->setOrigin(bgFront->getOrigin());
    repeatedSprite->setPosition(pos);

    bgTextureSize = bgTexture.getSize();
    atmosphere = std::make_unique<BackgroundAtmosphere>(
        sceneConfig.themeName,
        sceneConfig.layerIndex,
        sceneConfig.layerCount,
        parallaxFact
    );
    applyParallax();
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
    const sf::FloatRect activeViewRect = getActiveViewRect();
    if (atmosphere)
    {
        atmosphere->drawBehind(window, activeViewRect);
    }

    switch(type)
    {
        case Type::SingleBackground:
            window.draw(*bgFront);
            break;

        case Type::RepeatedBackgroundX:
        case Type::RepeatedBackgroundY:
        case Type::RepeatedBackgroundXY:
            drawRepeated(window);
            break;

        default:
            break;
    }

    if (atmosphere)
    {
        atmosphere->drawOverlay(window, activeViewRect);
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
        baseObjectPos.y + sceneConfig.tileOffsetY + cameraOffset.y * viewCompensatedParallax.y
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
    const int levelColumns = std::max(1, static_cast<int>(std::ceil(static_cast<float>(std::max(levelSize.x, 0)) / kBackgroundTileSize.x)));
    const float levelHeight = static_cast<float>(std::max(levelSize.y, 1));
    const float levelTop = std::min(0.f, static_cast<float>(WINDOW_HEIGHT) - levelHeight);
    const float levelBottom = levelTop + levelHeight;
    const int firstLevelRow = static_cast<int>(std::floor((levelTop - sceneConfig.tileOffsetY) / kBackgroundTileSize.y));
    const int lastLevelRow = static_cast<int>(std::ceil((levelBottom - sceneConfig.tileOffsetY) / kBackgroundTileSize.y)) - 1;
    const sf::Vector2i anchoredTile = {
        std::clamp(backgroundTileIndex(position).x, 0, levelColumns - 1),
        std::clamp(backgroundTileIndex(position).y, firstLevelRow, lastLevelRow)
    };
    const bool repeatX = repeatsX(type);
    const bool repeatY = repeatsY(type);
    const sf::Vector2f cameraOffset = camera->getCameraCenterPos() - BASE_CAMERAPOS;
    const sf::Vector2f viewCompensatedParallax = {
        1.f - parallaxFactor.x,
        1.f - parallaxFactor.y
    };
    const sf::Vector2f parallaxOffset{
        cameraOffset.x * viewCompensatedParallax.x,
        cameraOffset.y * viewCompensatedParallax.y
    };
    const int firstColumn = repeatX
        ? std::clamp(
            static_cast<int>(std::floor((viewRect.position.x - parallaxOffset.x) / kBackgroundTileSize.x)),
            0,
            levelColumns - 1
        )
        : anchoredTile.x;
    const int lastColumn = repeatX
        ? std::clamp(
            static_cast<int>(std::floor((viewRect.position.x + viewRect.size.x - parallaxOffset.x) / kBackgroundTileSize.x)),
            0,
            levelColumns - 1
        )
        : anchoredTile.x;
    const int firstRow = repeatY
        ? std::clamp(
            static_cast<int>(std::floor((viewRect.position.y - parallaxOffset.y) / kBackgroundTileSize.y)),
            firstLevelRow,
            lastLevelRow
        )
        : anchoredTile.y;
    const int lastRow = repeatY
        ? std::clamp(
            static_cast<int>(std::floor((viewRect.position.y + viewRect.size.y - parallaxOffset.y) / kBackgroundTileSize.y)),
            firstLevelRow,
            lastLevelRow
        )
        : anchoredTile.y;

    sf::Sprite& tileSprite = repeatedSprite ? *repeatedSprite : *bgFront;

    for (int y = firstRow; y <= lastRow; ++y)
    {
        for (int x = firstColumn; x <= lastColumn; ++x)
        {
            tileSprite.setPosition({
                kBackgroundTileSize.x * (static_cast<float>(x) + 0.5f) + parallaxOffset.x,
                kBackgroundTileSize.y * (static_cast<float>(y) + 0.5f) + sceneConfig.tileOffsetY + parallaxOffset.y
            });
            if (viewRect.findIntersection(tileSprite.getGlobalBounds()).has_value())
            {
                window.draw(tileSprite);
            }
        }
    }
}

void Background::setParallaxFactor(sf::Vector2f f)
{
    this->parallaxFactor = f;
    applyParallax();
}

sf::Sprite &Background::getSprite()
{
    return *bgFront;
}
