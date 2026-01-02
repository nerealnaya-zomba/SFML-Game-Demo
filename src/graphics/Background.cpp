#include<Background.h>

class GameCamera;

Background::Background(GameData& d, GameCamera& c, sf::Vector2f pos, std::string bgName, sf::Vector2f parallaxFact, Type t=Type::SingleBackground) 
: position(pos), name(bgName), type(t), gamedata(&d), camera(&c), parallaxFactor(parallaxFact)
{
    // Получаем ссылку на нужную текстуру
    sf::Texture& bgTexture = d.backgroundTextures.at(bgName);
    bgFront = new sf::Sprite(bgTexture);
    
    // Уменьшаем размер фона, до размера окна
    sf::Vector2f windowSizes = {static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT)};
    sf::Vector2f mansionSizes = {static_cast<float>(bgTexture.getSize().x),static_cast<float>(bgTexture.getSize().y)};
    bgFront->setScale({windowSizes.x/mansionSizes.x , windowSizes.y/mansionSizes.y });
    setSpriteOriginToMiddle(*bgFront);
    bgFront->setPosition(pos);
}

Background::~Background()
{
}

void Background::update()
{
    // Двигаем слой в зависимости от глубины
    applyParallax();
    
}

void Background::draw(sf::RenderWindow &window)
{
    switch(type){
        case Type::SingleBackground:
            window.draw(*bgFront);
        break;

        case Type::RepeatedBackground:                    // IMPLEMENTME
        break;

        default:break;
    }
}

void Background::applyParallax()
{
    sf::Vector2f difference = camera->getCameraCenter()-position;
    difference = {difference.x,difference.y};
    bgFront->setPosition({
        (position.x)+(parallaxFactor.x*difference.x),
        (position.y)+(parallaxFactor.y*difference.y)
    });
}

void Background::setParallaxFactor(sf::Vector2f f)
{
    this->parallaxFactor = f;
}
