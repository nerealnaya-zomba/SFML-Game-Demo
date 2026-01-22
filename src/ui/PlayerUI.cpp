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

    float currentY = rightCameraSidePos.y;
    for (auto &&r : cooldownRects)
    {
        r.back.setPosition({
        rightCameraSidePos.x - r.back.getSize().x,
        currentY
        });
        
        r.front.setPosition(r.back.getGlobalBounds().getCenter());
        
        r.icon->setPosition(r.back.getGlobalBounds().getCenter());
        
        currentY += r.back.getSize().y;
    }
}

void PlayerUI::updateIterpolation()
{
    for (auto &&r : cooldownRects)
    {
        int currentCooldown = r.currentCooldown->getElapsedTime().asMilliseconds();
        if((currentCooldown)<=(*r.targetCooldown))
        {
            r.front.setSize({
                BASE_UI_COOLDOWN_RECTS_SIZE.x*((currentCooldown)/(*r.targetCooldown)),
                BASE_UI_COOLDOWN_RECTS_SIZE.y*((currentCooldown)/(*r.targetCooldown))
            });
        }
    }
    
}

void PlayerUI::updateCooldownRectsColor()
{
    for (auto &&r : cooldownRects)
    {
        int currentCooldown = r.currentCooldown->getElapsedTime().asMilliseconds();
        if((currentCooldown)>=(*r.targetCooldown))
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

void PlayerUI::addCooldownRect(sf::Clock& currentCD, int& targetCD, sf::Texture& iconTexture)
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

    cooldownRects.push_back(std::move(cr));
}

PlayerUI::PlayerUI(Player &p, GameCamera &c)
    : camera(&c), player(&p)
{
}

void PlayerUI::draw(sf::RenderWindow &window)
{
    for (auto &&r : cooldownRects)
    {
        window.draw(r.back);
        window.draw(*r.icon);
        window.draw(r.front);
    }
    
}

void PlayerUI::update()
{
    updateCooldownRects();
}
