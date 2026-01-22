#include<PlayerUI.h>

void PlayerUI::updateCooldownRects()
{
    updateCooldownRectsPos();
    updateIterpolation();
    updateCooldownRectsColor();
}

void PlayerUI::updateCooldownRectsPos()
{
    sf::Vector2f rightCameraSidePos = {
        camera->getCameraCenterPos().x + (camera->getScreenViewSize().x/2),
        camera->getCameraCenterPos().y
    };

    for (auto &&r : cooldownRects)
    {
        unsigned int iterCount = 1;

        r.back.setPosition({
            rightCameraSidePos.x-r.back.getSize().x,
            rightCameraSidePos.y+(r.back.getSize().y*iterCount
        )});
        
        r.front.setPosition(r.back.getGlobalBounds().getCenter());
        
        r.icon->setPosition(r.back.getGlobalBounds().getCenter());
        
        iterCount++;
    }
}

void PlayerUI::updateIterpolation()
{
    for (auto &&r : cooldownRects)
    {
        if((*r.currentCooldown)<=(*r.targetCooldown))
        {
            r.front.setSize({
                BASE_UI_COOLDOWN_RECTS_SIZE.x*((*r.currentCooldown)/(*r.targetCooldown)),
                BASE_UI_COOLDOWN_RECTS_SIZE.y*((*r.currentCooldown)/(*r.targetCooldown))
            });
        }
    }
    
}

void PlayerUI::updateCooldownRectsColor()
{
    for (auto &&r : cooldownRects)
    {
        if(*(r.currentCooldown)>=(*r.targetCooldown))
        {
            r.back.setFillColor(BASE_UI_COOLDOWN_RECT_BACK_COLOR_ACTIVE);

            sf::Color frontColor = r.front.getFillColor();
            frontColor.a = BASE_UI_COOLDOWNT_RECT_FRONT_ALPHA_ACTIVE;
            r.front.setFillColor(frontColor);
        }
        else
        {
            r.back.setFillColor(BASE_UI_COOLDOWN_RECT_BACK_COLOR_INACTIVE);

            sf::Color frontColor = r.front.getFillColor();
            frontColor.a = BASE_UI_COOLDOWNT_RECT_FRONT_ALPHA_INACTIVE;
            r.front.setFillColor(frontColor);
        }
    }
    
}

void PlayerUI::addCooldownRect(int& currentCD, int& targetCD, sf::Texture& iconTexture)
{
    CooldownRect cr;
    cr.currentCooldown = &currentCD;
    cr.targetCooldown  = &targetCD;

    cr.back.setSize(BASE_UI_COOLDOWN_RECTS_SIZE);
    cr.back.setFillColor(BASE_UI_COOLDOWN_RECT_BACK_COLOR_ACTIVE);

    cr.front.setSize(BASE_UI_COOLDOWN_RECTS_SIZE);
    cr.front.setFillColor(BASE_UI_COOLDOWN_RECT_FRONT_COLOR);
    setRectangleOriginToMiddle(cr.front);

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
