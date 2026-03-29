#include<PlayerUI.h>

#include<GameData.h>

#include<algorithm>
#include<cmath>

namespace
{
sf::Color getInventoryQualityColor(Item::Quality quality)
{
    switch (quality)
    {
        case Item::COMMON:
            return sf::Color(170, 176, 193);
        case Item::RARE:
            return sf::Color(79, 180, 255);
        case Item::MYTH:
            return sf::Color(255, 119, 91);
        case Item::LEGENDARY:
            return sf::Color(255, 211, 91);
    }

    return sf::Color::White;
}
}

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
    hpTextInfo.setString(std::to_string(player->getHP()) + " / " + std::to_string(player->getMaxHP()));
    hpTextInfo.setPosition(hpBack.getGlobalBounds().getCenter());


    hpText.setPosition
        (
         {
            hpBack.getPosition().x + hpBack.getSize().x + BASE_HP_BAR_OFFSET.x,
            hpBack.getPosition().y
         }
        );
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
    energyTextInfo.setString(std::to_string(player->getEnergy()) + " / " + std::to_string(player->getMaxEnergy()));
    energyTextInfo.setPosition(energyBack.getGlobalBounds().getCenter());

    
    energyText.setPosition
        (
         {
            energyBack.getPosition().x + energyBack.getSize().x + BASE_HP_BAR_OFFSET.x,
            energyBack.getPosition().y
         }
        );
}

void PlayerUI::syncInventoryIcons()
{
    inventorySlots.clear();

    for (const auto& ownedItem : player->getInventory())
    {
        InventorySlotVisual slot;
        slot.shadow.setSize(BASE_INVENTORY_SLOT_SIZE);
        slot.shadow.setFillColor(sf::Color(0, 0, 0, 100));

        slot.background.setSize(BASE_INVENTORY_SLOT_SIZE);
        slot.background.setFillColor(sf::Color(18, 25, 39, 225));
        slot.background.setOutlineThickness(2.f);
        slot.background.setOutlineColor(getInventoryQualityColor(ownedItem.quality));

        slot.accent.setSize({BASE_INVENTORY_SLOT_SIZE.x, 5.f});
        slot.accent.setFillColor(getInventoryQualityColor(ownedItem.quality));

        const auto textureIt = data->itemsTextures.find(ownedItem.iconName);
        if(textureIt != data->itemsTextures.end())
        {
            slot.icon = std::make_unique<sf::Sprite>(textureIt->second);
            setSpriteOriginToMiddle(*slot.icon);
            const sf::Vector2u textureSize = textureIt->second.getSize();
            const sf::Vector2f targetSize = {30.f, 30.f};
            const float scaleX = targetSize.x / static_cast<float>(textureSize.x);
            const float scaleY = targetSize.y / static_cast<float>(textureSize.y);
            const float finalScale = std::min(scaleX, scaleY);
            slot.icon->setScale({finalScale, finalScale});
        }

        inventorySlots.push_back(std::move(slot));
    }
}

void PlayerUI::updateInventoryPanel()
{
    syncInventoryIcons();

    const sf::Vector2f screenViewPos = camera->getScreenViewPos();
    const sf::Vector2f screenViewSize = camera->getScreenViewSize();
    const float pulse = 0.85f + std::sin(uiAnimationClock.getElapsedTime().asSeconds() * 3.f) * 0.08f;

    const unsigned int inventoryRows = player->getInventory().empty()
        ? 1u
        : static_cast<unsigned int>((player->getInventory().size() + BASE_INVENTORY_COLUMNS - 1) / BASE_INVENTORY_COLUMNS);

    const float panelWidth = BASE_INVENTORY_PANEL_SIZE.x;
    const float gridHeight = player->getInventory().empty()
        ? 56.f
        : inventoryRows * BASE_INVENTORY_SLOT_SIZE.y + (inventoryRows - 1) * BASE_INVENTORY_SLOT_GAP.y;
    const float panelHeight = 128.f + gridHeight;

    const sf::Vector2f panelPos = {
        screenViewPos.x + screenViewSize.x - panelWidth - BASE_INVENTORY_PANEL_OFFSET.x,
        screenViewPos.y + BASE_INVENTORY_PANEL_OFFSET.y
    };

    inventoryPanelShadow.setSize({panelWidth, panelHeight});
    inventoryPanelShadow.setPosition({panelPos.x + 8.f, panelPos.y + 10.f});

    inventoryPanelBack.setSize({panelWidth, panelHeight});
    inventoryPanelBack.setPosition(panelPos);

    inventoryHeaderAccent.setSize({panelWidth, 8.f});
    inventoryHeaderAccent.setPosition(panelPos);
    inventoryHeaderAccent.setFillColor(sf::Color(255, 198, 87, static_cast<uint8_t>(255.f * pulse)));

    goldChip.setSize({panelWidth - 36.f, 34.f});
    goldChip.setPosition({panelPos.x + 18.f, panelPos.y + 48.f});

    inventoryDivider.setSize({panelWidth - 36.f, 2.f});
    inventoryDivider.setPosition({panelPos.x + 18.f, panelPos.y + 94.f});

    inventoryTitleText.setPosition({panelPos.x + 18.f, panelPos.y + 16.f});
    inventoryGoldText.setString(std::to_string(player->getGold()) + " gold");
    inventoryGoldText.setPosition({panelPos.x + 58.f, panelPos.y + 52.f});

    goldCoinGlow.setScale({pulse, pulse});
    goldCoinGlow.setPosition({panelPos.x + 36.f, panelPos.y + 65.f});
    goldCoinOuter.setPosition({panelPos.x + 36.f, panelPos.y + 65.f});
    goldCoinInner.setPosition({panelPos.x + 36.f, panelPos.y + 65.f});
    goldCoinShine.setPosition({panelPos.x + 34.f, panelPos.y + 63.f});
    goldCoinShine.setRotation(sf::degrees(-24.f + std::sin(uiAnimationClock.getElapsedTime().asSeconds() * 5.f) * 5.f));

    const sf::Vector2f gridStart = {panelPos.x + 18.f, panelPos.y + 108.f};
    for (size_t index = 0; index < inventorySlots.size(); ++index)
    {
        const unsigned int row = static_cast<unsigned int>(index / BASE_INVENTORY_COLUMNS);
        const unsigned int column = static_cast<unsigned int>(index % BASE_INVENTORY_COLUMNS);
        const sf::Vector2f slotPos = {
            gridStart.x + column * (BASE_INVENTORY_SLOT_SIZE.x + BASE_INVENTORY_SLOT_GAP.x),
            gridStart.y + row * (BASE_INVENTORY_SLOT_SIZE.y + BASE_INVENTORY_SLOT_GAP.y)
        };

        InventorySlotVisual& slot = inventorySlots[index];
        slot.shadow.setPosition({slotPos.x + 4.f, slotPos.y + 5.f});
        slot.background.setPosition(slotPos);
        slot.accent.setPosition({slotPos.x, slotPos.y + BASE_INVENTORY_SLOT_SIZE.y - slot.accent.getSize().y});

        if(slot.icon)
        {
            slot.icon->setPosition({
                slotPos.x + BASE_INVENTORY_SLOT_SIZE.x / 2.f,
                slotPos.y + BASE_INVENTORY_SLOT_SIZE.y / 2.f - 2.f
            });
        }
    }

    inventoryEmptyText.setString("No relics yet");
    setTextOriginToMiddle(inventoryEmptyText);
    inventoryEmptyText.setPosition({
        panelPos.x + panelWidth / 2.f,
        gridStart.y + gridHeight / 2.f
    });
}

PlayerUI::PlayerUI(Player &p, GameCamera &c, GameData &d)
    : camera(&c)
    , data(&d)
    , player(&p)
    , hpTextInfo(*d.gameFont)
    , hpText(*d.gameFont)
    , energyTextInfo(*d.gameFont)
    , energyText(*d.gameFont)
    , inventoryTitleText(*d.gameFont)
    , inventoryGoldText(*d.gameFont)
    , inventoryEmptyText(*d.gameFont)
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

    hpTextInfo.setCharacterSize(20);
    hpTextInfo.setFillColor(sf::Color::White);
    hpTextInfo.setString(std::to_string(player->getHP()) + " / " + std::to_string(player->getMaxHP()));
    hpTextInfo.setOrigin(hpTextInfo.getGlobalBounds().getCenter());
    hpTextInfo.setPosition(hpBack.getGlobalBounds().getCenter());

    hpText.setCharacterSize(20);
    hpText.setFillColor(sf::Color::White);
    hpText.setString("Health");
    hpText.setPosition
        (
         {
            hpBack.getPosition().x + hpBack.getSize().x + BASE_HP_BAR_OFFSET.x,
            hpBack.getPosition().y
         }
        );


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

    energyTextInfo.setCharacterSize(20);
    energyTextInfo.setFillColor(sf::Color::White);
    energyTextInfo.setString(std::to_string(player->getEnergy()) + " / " + std::to_string(player->getMaxHP()));
    energyTextInfo.setOrigin(energyTextInfo.getGlobalBounds().getCenter());
    energyTextInfo.setPosition(energyBack.getGlobalBounds().getCenter());

    energyText.setCharacterSize(20);
    energyText.setFillColor(sf::Color::White);
    energyText.setString("Energy");
    energyText.setPosition
        (
         {
            energyBack.getPosition().x + energyBack.getSize().x + BASE_HP_BAR_OFFSET.x,
            energyBack.getPosition().y
         }
        );

    inventoryPanelShadow.setFillColor(sf::Color(0, 0, 0, 115));

    inventoryPanelBack.setFillColor(sf::Color(9, 15, 26, 230));
    inventoryPanelBack.setOutlineThickness(2.f);
    inventoryPanelBack.setOutlineColor(sf::Color(61, 81, 110, 255));

    inventoryHeaderAccent.setFillColor(sf::Color(255, 198, 87));
    inventoryDivider.setFillColor(sf::Color(52, 65, 89, 255));

    goldChip.setFillColor(sf::Color(35, 24, 11, 230));
    goldChip.setOutlineThickness(1.f);
    goldChip.setOutlineColor(sf::Color(126, 93, 31, 255));

    goldCoinGlow.setRadius(14.f);
    goldCoinGlow.setOrigin({goldCoinGlow.getRadius(), goldCoinGlow.getRadius()});
    goldCoinGlow.setFillColor(sf::Color(255, 210, 106, 60));

    goldCoinOuter.setRadius(10.f);
    goldCoinOuter.setOrigin({goldCoinOuter.getRadius(), goldCoinOuter.getRadius()});
    goldCoinOuter.setFillColor(sf::Color(255, 200, 70));
    goldCoinOuter.setOutlineThickness(2.f);
    goldCoinOuter.setOutlineColor(sf::Color(163, 102, 19));

    goldCoinInner.setRadius(7.f);
    goldCoinInner.setOrigin({goldCoinInner.getRadius(), goldCoinInner.getRadius()});
    goldCoinInner.setFillColor(sf::Color(255, 233, 145));

    goldCoinShine.setSize({4.f, 12.f});
    setRectangleOriginToMiddle(goldCoinShine);
    goldCoinShine.setFillColor(sf::Color(255, 250, 226, 220));

    inventoryTitleText.setCharacterSize(23);
    inventoryTitleText.setFillColor(sf::Color(245, 239, 227));
    inventoryTitleText.setString("Inventory");

    inventoryGoldText.setCharacterSize(20);
    inventoryGoldText.setFillColor(sf::Color(255, 219, 120));

    inventoryEmptyText.setCharacterSize(17);
    inventoryEmptyText.setFillColor(sf::Color(173, 181, 201));
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
    window.draw(hpBack);
    window.draw(hpFront);
    window.draw(hpTextInfo);
    window.draw(hpText);

    // Energy
    window.draw(energyBack);
    window.draw(energyFront);
    window.draw(energyTextInfo);
    window.draw(energyText);

    // Inventory panel
    window.draw(inventoryPanelShadow);
    window.draw(inventoryPanelBack);
    window.draw(inventoryHeaderAccent);
    window.draw(goldChip);
    window.draw(goldCoinGlow);
    window.draw(goldCoinOuter);
    window.draw(goldCoinInner);
    window.draw(goldCoinShine);
    window.draw(inventoryDivider);
    window.draw(inventoryTitleText);
    window.draw(inventoryGoldText);

    for (auto&& slot : inventorySlots)
    {
        window.draw(slot.shadow);
        window.draw(slot.background);
        window.draw(slot.accent);
        if(slot.icon)
        {
            window.draw(*slot.icon);
        }
    }

    if(inventorySlots.empty())
    {
        window.draw(inventoryEmptyText);
    }
}

void PlayerUI::update()
{
    updateCooldownRects();
    updateHP();
    updateEnergy();
    updateInventoryPanel();
}
