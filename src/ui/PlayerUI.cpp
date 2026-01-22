#include<PlayerUI.h>

void PlayerUI::updateCooldownRects()
{
}

void PlayerUI::addCooldownRect(int& cd, sf::Texture& iconTexture)
{
    CooldownRect cr;
    cr.cooldown = &cd;

    cr.back.setSize(BASE_UI_COOLDOWN_RECTS_SIZE);
    cr.back.setFillColor(BASE_UI_COOLDOWN_RECT_BACK_COLOR_ACTIVE);

    cr.front.setSize(BASE_UI_COOLDOWN_RECTS_SIZE);
    cr.front.setFillColor(BASE_UI_COOLDOWN_RECT_FRONT_COLOR);

    cr.icon = std::make_unique<sf::Sprite>(iconTexture);

    sf::Vector2f textureSize = static_cast<sf::Vector2f>(iconTexture.getSize());
    sf::Vector2f calculatedScale = {
        BASE_UI_COOLDOWN_RECTS_SIZE.x/textureSize.x,
        BASE_UI_COOLDOWN_RECTS_SIZE.y/textureSize.y
    };
    cr.icon->setScale(calculatedScale);
    setSpriteOriginToMiddle(*cr.icon);

    cooldownRects.push_back(cr);
}

PlayerUI::PlayerUI(Player &p, GameCamera &c, const sf::Vector2f mnS, const sf::Vector2f mxS)
    : minSize(mnS), maxSize(mxS)
{
}

void PlayerUI::draw(sf::RenderWindow &window)
{
}
