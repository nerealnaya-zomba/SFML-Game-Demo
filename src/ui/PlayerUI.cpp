#include<PlayerUI.h>
#include<GameData.h>

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
        
        setRectangleOriginToMiddle(r.front);
        r.front.setPosition(r.back.getGlobalBounds().getCenter());
        
        r.icon->setPosition(r.back.getGlobalBounds().getCenter());
        
        currentY += r.back.getSize().y;
    }
}

void PlayerUI::updateIterpolation()
{
    for (auto&& r : cooldownRects)
    {
        int currentCooldown = r.currentCooldown->getElapsedTime().asMilliseconds();
        int targetCooldown = *r.targetCooldown;
        
        if (currentCooldown <= targetCooldown && targetCooldown > 0)
        {
            float progress = static_cast<float>(currentCooldown) / static_cast<float>(targetCooldown);
            
            r.front.setSize({
                BASE_UI_COOLDOWN_RECTS_SIZE.x * progress,
                BASE_UI_COOLDOWN_RECTS_SIZE.y * progress
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

void PlayerUI::updateHP()
{

    sf::Vector2f screenViewPos = camera->getScreenViewPos();

    hpBack.setPosition
    (
        {
            screenViewPos.x+BASE_HP_BAR_OFFSET.x,screenViewPos.y+BASE_HP_BAR_OFFSET.y
        }
    );
    hpFront.setPosition
    (
        {
            screenViewPos.x+BASE_HP_BAR_OFFSET.x,
            screenViewPos.y+BASE_HP_BAR_OFFSET.y
        }
    );

    updateHpInterpolation();

    updateHpText();
}

void PlayerUI::updateHpInterpolation()
{
    float playerHP = static_cast<float>(player->getHP());
    float playerMaxHP = static_cast<float>(player->getMaxHP());

    float interpolationFactor = playerHP/playerMaxHP;

    hpFront.setSize({hpBack.getSize().x*interpolationFactor,hpBack.getSize().y});
}

void PlayerUI::updateHpText()
{
    hpText.setString(std::to_string(player->getHP()) + " / " + std::to_string(player->getMaxHP()));
    hpText.setPosition(hpBack.getGlobalBounds().getCenter());
}

void PlayerUI::updateEnergy()
{

    sf::Vector2f screenViewPos = camera->getScreenViewPos();

    energyBack.setPosition
    (
        {
            screenViewPos.x + BASE_HP_BAR_OFFSET.x,
            screenViewPos.y + BASE_HP_BAR_OFFSET.y + hpFront.getSize().y + BASE_HP_BAR_OFFSET.y
        }
    );
    energyFront.setPosition
    (
        {
            screenViewPos.x+BASE_HP_BAR_OFFSET.x,
            screenViewPos.y + BASE_HP_BAR_OFFSET.y + hpFront.getSize().y + BASE_HP_BAR_OFFSET.y
        }
    );

    updateEnergyInterpolation();

    updateEnergyText();
}

void PlayerUI::updateEnergyInterpolation()
{
    float playerEnergy = static_cast<float>(player->getEnergy());
    float playerMaxEnergy = static_cast<float>(player->getMaxEnergy());

    float interpolationFactor = playerEnergy/playerMaxEnergy;

    energyFront.setSize({energyBack.getSize().x*interpolationFactor,energyBack.getSize().y});
}

void PlayerUI::updateEnergyText()
{
    energyText.setString(std::to_string(player->getEnergy()) + " / " + std::to_string(player->getMaxEnergy()));
    energyText.setPosition(energyBack.getGlobalBounds().getCenter());
}

PlayerUI::PlayerUI(Player &p, GameCamera &c, GameData &d)
    : camera(&c), player(&p), data(&d), hpText(*d.gameFont), energyText(*d.gameFont)
{

    sf::Vector2f screenViewPos = camera->getScreenViewPos();

    //hpBack hpFront init
    hpBack.setSize({600,30});
    hpBack.setFillColor(sf::Color::Black);
    hpBack.setPosition({screenViewPos.x+BASE_HP_BAR_OFFSET.x,screenViewPos.y+BASE_HP_BAR_OFFSET.y});

    hpFront.setSize({0,30});
    hpFront.setFillColor(sf::Color::Green);
    hpFront.setPosition
    (
        {
            screenViewPos.x + BASE_HP_BAR_OFFSET.x,
            screenViewPos.y + BASE_HP_BAR_OFFSET.y
        }
    );

    hpText.setCharacterSize(20);
    hpText.setFillColor(sf::Color::White);
    hpText.setString(std::to_string(player->getHP()) + " / " + std::to_string(player->getMaxHP()));
    hpText.setOrigin(hpText.getGlobalBounds().getCenter());
    hpText.setPosition(hpBack.getGlobalBounds().getCenter());

    //energyBack hpFront init
    energyBack.setSize({600,30});
    energyBack.setFillColor(sf::Color::Black);
    energyBack.setPosition
    (
        {
            screenViewPos.x + BASE_HP_BAR_OFFSET.x,
            screenViewPos.y + BASE_HP_BAR_OFFSET.y + hpFront.getSize().y + BASE_HP_BAR_OFFSET.y
        }
    );

    energyFront.setSize({0,30});
    energyFront.setFillColor(sf::Color::Blue);
    energyFront.setPosition
    (
        {
            screenViewPos.x+BASE_HP_BAR_OFFSET.x,
            screenViewPos.y + BASE_HP_BAR_OFFSET.y + hpFront.getSize().y + BASE_HP_BAR_OFFSET.y
        }
    );

    energyText.setCharacterSize(20);
    energyText.setFillColor(sf::Color::White);
    energyText.setString(std::to_string(player->getEnergy()) + " / " + std::to_string(player->getMaxHP()));
    energyText.setOrigin(energyText.getGlobalBounds().getCenter());
    energyText.setPosition(energyBack.getGlobalBounds().getCenter());
}

void PlayerUI::draw(sf::RenderWindow &window)
{
    // Cooldown
    for (auto &&r : cooldownRects)
    {
        window.draw(r.back);
        window.draw(*r.icon);
        window.draw(r.front);
    }
    
    // HP
        // Bar
    window.draw(hpBack);
    window.draw(hpFront);
        // Text
    window.draw(hpText);

    // Energy
        // Bar
    window.draw(energyBack);
    window.draw(energyFront);
        // Text
    window.draw(energyText);

}

void PlayerUI::update()
{
    updateCooldownRects();

    updateHP();

    updateEnergy();
}
