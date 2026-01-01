#include<Background.h>

Background::Background(GameData& data,sf::Vector2f pos, std::string bgName, Type t = Type::SingleBackground) : position(pos), name(bgName), type(t)
{
    // Получаем ссылку на нужную текстуру
    sf::Texture& bgTexture = data.backgroundTextures.at(bgName);
    bgFront = new sf::Sprite(bgTexture);
    
    // Уменьшаем размер фона, до размера окна
    sf::Vector2f windowSizes = {static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT)};
    sf::Vector2f mansionSizes = {static_cast<float>(bgTexture.getSize().x),static_cast<float>(bgTexture.getSize().y)};
    bgFront->setScale({windowSizes.x/mansionSizes.x , windowSizes.y/mansionSizes.y });
}

Background::~Background()
{
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
