#include <Background.h>

#include <algorithm>
#include <cmath>
#include <utility>

class GameCamera;

Background::Background(GameData& d, GameCamera& c, GameLevel& l, sf::Vector2f pos, std::string bgName, sf::Vector2f parallaxFact, Type t)
: position(pos), name(std::move(bgName)), type(t), camera(&c), level(&l), parallaxFactor(parallaxFact)
{
    sf::Texture& bgTexture = d.backgroundTextures.at(name);
    bgFront = std::make_unique<sf::Sprite>(bgTexture);
    
    sf::Vector2f windowSizes = {static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT)};
    sf::Vector2f textureSizes = {static_cast<float>(bgTexture.getSize().x),static_cast<float>(bgTexture.getSize().y)};
    bgFront->setScale({windowSizes.x/textureSizes.x , windowSizes.y/textureSizes.y });
    setSpriteOriginToMiddle(*bgFront);
    bgFront->setPosition(pos);

    bgTextureSize = bgTexture.getSize();
}

Background::~Background() = default;

void Background::update()
{
    applyParallax();
}

void Background::draw(sf::RenderWindow &window)
{
    switch(type){
        case Type::SingleBackground:
            window.draw(*bgFront);
            break;

        case Type::RepeatedBackground:
        {
            const float repeatedWidth = bgFront->getGlobalBounds().size.x;
            if (repeatedWidth <= 0.f)
            {
                window.draw(*bgFront);
                break;
            }

            const int countOfRepeatsX = std::max(
                1,
                static_cast<int>(std::ceil(static_cast<float>(level->getLevelSize().x) / repeatedWidth)) + 2
            );

            for (int i = -1; i < countOfRepeatsX; ++i)
            {
                sf::Sprite repeatedSprite = *bgFront;
                repeatedSprite.setPosition({
                    bgFront->getPosition().x + repeatedWidth * static_cast<float>(i),
                    bgFront->getPosition().y
                });
                window.draw(repeatedSprite);
            }
            break;
        }

        default:
            break;
    }
}

void Background::applyParallax()
{
    const sf::Vector2f baseObjectPos = position;
    const sf::Vector2f cameraOffset = camera->getCameraCenterPos() - BASE_CAMERAPOS;

    bgFront->setPosition({
        baseObjectPos.x + cameraOffset.x * parallaxFactor.x,
        baseObjectPos.y + cameraOffset.y * parallaxFactor.y
    });
}

void Background::setParallaxFactor(sf::Vector2f f)
{
    this->parallaxFactor = f;
}

sf::Sprite &Background::getSprite()
{
    return *bgFront;
}
