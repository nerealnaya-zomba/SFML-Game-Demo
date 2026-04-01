#include<PlayerUI.h>

#include<GameData.h>

#include<algorithm>
#include<cmath>

namespace
{
const sf::Color kHudBronzeBorder(168, 126, 88, 255);
const sf::Color kHudBronzeGlow(228, 181, 126, 255);
const sf::Color kHudPanelShadow(0, 0, 0, 125);
const sf::Color kHudPanelFrame(20, 11, 12, 232);
const sf::Color kHudPanelInset(10, 8, 10, 220);
const sf::Color kHudTextPrimary(242, 229, 212, 255);
const sf::Color kHudTextSecondary(203, 186, 166, 255);
const sf::Color kHudCrimsonFill(153, 34, 29, 245);
const sf::Color kHudCrimsonHighlight(236, 112, 91, 180);
const sf::Color kHudCrimsonPlate(55, 15, 16, 235);
const sf::Color kHudArcaneFill(31, 89, 125, 245);
const sf::Color kHudArcaneHighlight(116, 207, 255, 180);
const sf::Color kHudArcanePlate(13, 27, 42, 235);
const sf::Color kCooldownMaskColor(7, 5, 7, 195);
const sf::Color kCooldownInactiveIconColor(172, 155, 144, 170);
const sf::Color kCooldownReadyIconColor(255, 239, 220, 255);
const sf::Color kCooldownReadyAccent(214, 173, 120, 255);
const sf::Color kCooldownChargeAccent(125, 90, 64, 255);
const sf::Color kCooldownFrameReady(183, 140, 98, 255);
const sf::Color kCooldownFrameInactive(102, 75, 60, 220);

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

void styleHudText(sf::Text& text, unsigned int size, sf::Color fillColor)
{
    text.setCharacterSize(size);
    text.setFillColor(fillColor);
    text.setOutlineThickness(1.5f);
    text.setOutlineColor(sf::Color(0, 0, 0, 180));
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

    const float totalHeight = cooldownRects.empty()
        ? 0.f
        : static_cast<float>(cooldownRects.size()) * BASE_UI_COOLDOWN_RECTS_SIZE.y +
            static_cast<float>(cooldownRects.size() - 1) * BASE_UI_COOLDOWN_STACK_GAP;
    const float stackX = rightCameraSidePos.x - BASE_UI_COOLDOWN_RECTS_SIZE.x - BASE_UI_COOLDOWN_STACK_OFFSET.x;
    float currentY = rightCameraSidePos.y - totalHeight / 2.f;

    for (auto &&r : cooldownRects)
    {
        r.shadow.setPosition({stackX + 5.f, currentY + 6.f});
        r.frame.setPosition({stackX, currentY});
        r.back.setPosition({
            stackX + (BASE_UI_COOLDOWN_RECTS_SIZE.x - BASE_UI_COOLDOWN_INNER_SIZE.x) / 2.f,
            currentY + (BASE_UI_COOLDOWN_RECTS_SIZE.y - BASE_UI_COOLDOWN_INNER_SIZE.y) / 2.f
        });
        r.front.setPosition(r.back.getPosition());
        r.accent.setPosition({
            r.back.getPosition().x,
            r.back.getPosition().y + r.back.getSize().y - r.accent.getSize().y
        });
        r.readyGlow.setPosition(r.back.getGlobalBounds().getCenter());
        r.icon->setPosition(r.back.getGlobalBounds().getCenter());

        currentY += BASE_UI_COOLDOWN_RECTS_SIZE.y + BASE_UI_COOLDOWN_STACK_GAP;
    }
}

void PlayerUI::updateIterpolation()
{
    for (auto&& r : cooldownRects)
    {
        const int currentCooldown = r.currentCooldown->getElapsedTime().asMilliseconds();
        const int targetCooldown = *r.targetCooldown;
        const float progress = (targetCooldown <= 0)
            ? 1.f
            : std::clamp(static_cast<float>(currentCooldown) / static_cast<float>(targetCooldown), 0.f, 1.f);

        r.accent.setSize({
            BASE_UI_COOLDOWN_INNER_SIZE.x * progress,
            r.accent.getSize().y
        });

        r.front.setSize(BASE_UI_COOLDOWN_INNER_SIZE);
    }
}

void PlayerUI::updateCooldownRectsColor()
{
    const float elapsed = uiAnimationClock.getElapsedTime().asSeconds();

    for (auto &&r : cooldownRects)
    {
        const int currentCooldown = r.currentCooldown->getElapsedTime().asMilliseconds();
        const int targetCooldown = *r.targetCooldown;
        const float progress = (targetCooldown <= 0)
            ? 1.f
            : std::clamp(static_cast<float>(currentCooldown) / static_cast<float>(targetCooldown), 0.f, 1.f);
        const bool isReady = (targetCooldown <= 0) || (currentCooldown >= targetCooldown);
        const float pulse = 0.85f + std::sin(elapsed * 3.5f + progress * 2.f) * 0.15f;

        r.frame.setOutlineColor(isReady ? kCooldownFrameReady : kCooldownFrameInactive);
        r.back.setOutlineColor(isReady
            ? sf::Color(kHudBronzeGlow.r, kHudBronzeGlow.g, kHudBronzeGlow.b, 120)
            : sf::Color(70, 53, 44, 120));
        r.accent.setFillColor(isReady ? kCooldownReadyAccent : kCooldownChargeAccent);

        const std::uint8_t maskAlpha = isReady
            ? 0
            : static_cast<std::uint8_t>(std::clamp((1.f - progress) * 180.f + 25.f, 0.f, 220.f));
        r.front.setFillColor(sf::Color(
            kCooldownMaskColor.r,
            kCooldownMaskColor.g,
            kCooldownMaskColor.b,
            maskAlpha
        ));

        r.readyGlow.setScale({pulse, pulse});
        r.readyGlow.setFillColor(sf::Color(
            kHudBronzeGlow.r,
            kHudBronzeGlow.g,
            kHudBronzeGlow.b,
            static_cast<std::uint8_t>(isReady ? 46.f + pulse * 34.f : 14.f + progress * 10.f)
        ));

        if (isReady)
        {
            r.icon->setColor(kCooldownReadyIconColor);
        }
        else
        {
            r.icon->setColor(sf::Color(
                kCooldownInactiveIconColor.r,
                kCooldownInactiveIconColor.g,
                kCooldownInactiveIconColor.b,
                static_cast<std::uint8_t>(170.f + progress * 45.f)
            ));
        }
    }
}

void PlayerUI::addCooldownRect(sf::Clock& currentCD, int& targetCD, sf::Texture& iconTexture)
{
    CooldownRect cr;
    cr.currentCooldown = &currentCD;
    cr.targetCooldown  = &targetCD;

    cr.shadow.setSize(BASE_UI_COOLDOWN_RECTS_SIZE);
    cr.shadow.setFillColor(kHudPanelShadow);

    cr.frame.setSize(BASE_UI_COOLDOWN_RECTS_SIZE);
    cr.frame.setFillColor(kHudPanelFrame);
    cr.frame.setOutlineThickness(2.f);
    cr.frame.setOutlineColor(kCooldownFrameReady);

    cr.back.setSize(BASE_UI_COOLDOWN_INNER_SIZE);
    cr.back.setFillColor(kHudPanelInset);
    cr.back.setOutlineThickness(1.f);
    cr.back.setOutlineColor(sf::Color(82, 59, 46, 120));

    cr.front.setSize(BASE_UI_COOLDOWN_INNER_SIZE);
    cr.front.setFillColor(kCooldownMaskColor);

    cr.accent.setSize({0.f, 5.f});
    cr.accent.setFillColor(kCooldownChargeAccent);

    cr.readyGlow.setRadius(22.f);
    cr.readyGlow.setOrigin({cr.readyGlow.getRadius(), cr.readyGlow.getRadius()});
    cr.readyGlow.setFillColor(sf::Color(kHudBronzeGlow.r, kHudBronzeGlow.g, kHudBronzeGlow.b, 0));

    cr.icon = std::make_unique<sf::Sprite>(iconTexture);

    const sf::Vector2f textureSize = static_cast<sf::Vector2f>(iconTexture.getSize());
    const sf::Vector2f targetSize = {28.f, 28.f};
    const float scaleX = targetSize.x / textureSize.x;
    const float scaleY = targetSize.y / textureSize.y;
    const float scale = std::min(scaleX, scaleY);
    cr.icon->setScale({scale, scale});
    setSpriteOriginToMiddle(*cr.icon);
    cr.icon->setColor(kCooldownReadyIconColor);

    cooldownRects.push_back(std::move(cr));
}

void PlayerUI::updateHP()
{
    const sf::Vector2f screenViewPos = camera->getScreenViewPos();
    const float elapsed = uiAnimationClock.getElapsedTime().asSeconds();
    const float pulse = 0.9f + std::sin(elapsed * 2.3f) * 0.08f;
    const sf::Vector2f basePos = {
        screenViewPos.x + BASE_HP_BAR_OFFSET.x,
        screenViewPos.y + BASE_HP_BAR_OFFSET.y
    };
    const sf::Vector2f innerPos = {basePos.x + 3.f, basePos.y + 3.f};

    hpShadow.setPosition({basePos.x + 6.f, basePos.y + 6.f});
    hpFrame.setPosition(basePos);
    hpBack.setPosition(innerPos);
    hpLabelPlate.setPosition(innerPos);

    hpFront.setPosition({
        innerPos.x + BASE_RESOURCE_LABEL_WIDTH + 10.f,
        innerPos.y + 5.f
    });
    hpHighlight.setPosition({
        hpFront.getPosition().x + 3.f,
        hpFront.getPosition().y + 2.f
    });
    hpFrame.setOutlineColor(sf::Color(
        kHudBronzeBorder.r,
        kHudBronzeBorder.g,
        kHudBronzeBorder.b,
        static_cast<std::uint8_t>(220.f + pulse * 22.f)
    ));

    updateHpInterpolation();
    updateHpText();
}

void PlayerUI::updateHpInterpolation()
{
    const float playerHP = static_cast<float>(player->getHP());
    const float playerMaxHP = static_cast<float>(player->getMaxHP());
    const float interpolationFactor = (playerMaxHP <= 0.f)
        ? 0.f
        : std::clamp(playerHP / playerMaxHP, 0.f, 1.f);

    const float fillWidth = hpBack.getSize().x - BASE_RESOURCE_LABEL_WIDTH - 16.f;
    const float currentWidth = fillWidth * interpolationFactor;
    hpFront.setSize({currentWidth, hpFront.getSize().y});
    hpHighlight.setSize({std::max(0.f, currentWidth - 6.f), hpHighlight.getSize().y});
}

void PlayerUI::updateHpText()
{
    hpTextInfo.setString(std::to_string(player->getHP()) + " / " + std::to_string(player->getMaxHP()));
    setTextOriginToMiddle(hpTextInfo);
    hpTextInfo.setPosition({
        hpFront.getPosition().x + (hpBack.getSize().x - BASE_RESOURCE_LABEL_WIDTH - 16.f) / 2.f,
        hpBack.getPosition().y + hpBack.getSize().y / 2.f - 2.f
    });

    hpText.setPosition({
        hpLabelPlate.getPosition().x + 16.f,
        hpLabelPlate.getPosition().y + 6.f
    });
}

void PlayerUI::updateEnergy()
{
    const sf::Vector2f screenViewPos = camera->getScreenViewPos();
    const float elapsed = uiAnimationClock.getElapsedTime().asSeconds();
    const float pulse = 0.9f + std::sin(elapsed * 2.f + 0.8f) * 0.07f;
    const sf::Vector2f basePos = {
        screenViewPos.x + BASE_HP_BAR_OFFSET.x,
        screenViewPos.y + BASE_HP_BAR_OFFSET.y + BASE_RESOURCE_BAR_SIZE.y + BASE_RESOURCE_BAR_GAP
    };
    const sf::Vector2f innerPos = {basePos.x + 3.f, basePos.y + 3.f};

    energyShadow.setPosition({basePos.x + 6.f, basePos.y + 6.f});
    energyFrame.setPosition(basePos);
    energyBack.setPosition(innerPos);
    energyLabelPlate.setPosition(innerPos);

    energyFront.setPosition({
        innerPos.x + BASE_RESOURCE_LABEL_WIDTH + 10.f,
        innerPos.y + 5.f
    });
    energyHighlight.setPosition({
        energyFront.getPosition().x + 3.f,
        energyFront.getPosition().y + 2.f
    });
    energyFrame.setOutlineColor(sf::Color(
        kHudBronzeBorder.r,
        kHudBronzeBorder.g,
        kHudBronzeBorder.b,
        static_cast<std::uint8_t>(208.f + pulse * 18.f)
    ));

    updateEnergyInterpolation();
    updateEnergyText();
}

void PlayerUI::updateEnergyInterpolation()
{
    const float playerEnergy = static_cast<float>(player->getEnergy());
    const float playerMaxEnergy = static_cast<float>(player->getMaxEnergy());
    const float interpolationFactor = (playerMaxEnergy <= 0.f)
        ? 0.f
        : std::clamp(playerEnergy / playerMaxEnergy, 0.f, 1.f);

    const float fillWidth = energyBack.getSize().x - BASE_RESOURCE_LABEL_WIDTH - 16.f;
    const float currentWidth = fillWidth * interpolationFactor;
    energyFront.setSize({currentWidth, energyFront.getSize().y});
    energyHighlight.setSize({std::max(0.f, currentWidth - 6.f), energyHighlight.getSize().y});
}

void PlayerUI::updateEnergyText()
{
    energyTextInfo.setString(std::to_string(player->getEnergy()) + " / " + std::to_string(player->getMaxEnergy()));
    setTextOriginToMiddle(energyTextInfo);
    energyTextInfo.setPosition({
        energyFront.getPosition().x + (energyBack.getSize().x - BASE_RESOURCE_LABEL_WIDTH - 16.f) / 2.f,
        energyBack.getPosition().y + energyBack.getSize().y / 2.f - 2.f
    });

    energyText.setPosition({
        energyLabelPlate.getPosition().x + 16.f,
        energyLabelPlate.getPosition().y + 6.f
    });
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
    hpShadow.setSize(BASE_RESOURCE_BAR_SIZE);
    hpShadow.setFillColor(kHudPanelShadow);

    hpFrame.setSize(BASE_RESOURCE_BAR_SIZE);
    hpFrame.setFillColor(kHudPanelFrame);
    hpFrame.setOutlineThickness(2.f);
    hpFrame.setOutlineColor(kHudBronzeBorder);

    hpBack.setSize({BASE_RESOURCE_BAR_SIZE.x - 6.f, BASE_RESOURCE_BAR_SIZE.y - 6.f});
    hpBack.setFillColor(kHudPanelInset);

    hpLabelPlate.setSize({BASE_RESOURCE_LABEL_WIDTH, hpBack.getSize().y});
    hpLabelPlate.setFillColor(kHudCrimsonPlate);
    hpLabelPlate.setOutlineThickness(1.f);
    hpLabelPlate.setOutlineColor(sf::Color(137, 70, 57, 220));

    hpFront.setSize({0.f, hpBack.getSize().y - 10.f});
    hpFront.setFillColor(kHudCrimsonFill);

    hpHighlight.setSize({0.f, 8.f});
    hpHighlight.setFillColor(kHudCrimsonHighlight);

    styleHudText(hpTextInfo, 21, kHudTextPrimary);
    hpTextInfo.setString(std::to_string(player->getHP()) + " / " + std::to_string(player->getMaxHP()));

    styleHudText(hpText, 20, kHudTextSecondary);
    hpText.setString("HEALTH");

    energyShadow.setSize(BASE_RESOURCE_BAR_SIZE);
    energyShadow.setFillColor(kHudPanelShadow);

    energyFrame.setSize(BASE_RESOURCE_BAR_SIZE);
    energyFrame.setFillColor(kHudPanelFrame);
    energyFrame.setOutlineThickness(2.f);
    energyFrame.setOutlineColor(kHudBronzeBorder);

    energyBack.setSize({BASE_RESOURCE_BAR_SIZE.x - 6.f, BASE_RESOURCE_BAR_SIZE.y - 6.f});
    energyBack.setFillColor(kHudPanelInset);

    energyLabelPlate.setSize({BASE_RESOURCE_LABEL_WIDTH, energyBack.getSize().y});
    energyLabelPlate.setFillColor(kHudArcanePlate);
    energyLabelPlate.setOutlineThickness(1.f);
    energyLabelPlate.setOutlineColor(sf::Color(67, 113, 145, 220));

    energyFront.setSize({0.f, energyBack.getSize().y - 10.f});
    energyFront.setFillColor(kHudArcaneFill);

    energyHighlight.setSize({0.f, 8.f});
    energyHighlight.setFillColor(kHudArcaneHighlight);

    styleHudText(energyTextInfo, 21, kHudTextPrimary);
    energyTextInfo.setString(std::to_string(player->getEnergy()) + " / " + std::to_string(player->getMaxEnergy()));

    styleHudText(energyText, 20, kHudTextSecondary);
    energyText.setString("ENERGY");

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
        window.draw(r.shadow);
        window.draw(r.readyGlow);
        window.draw(r.frame);
        window.draw(r.back);
        window.draw(*r.icon);
        window.draw(r.front);
        window.draw(r.accent);
    }
    
    // HP
    window.draw(hpShadow);
    window.draw(hpFrame);
    window.draw(hpBack);
    window.draw(hpFront);
    window.draw(hpHighlight);
    window.draw(hpLabelPlate);
    window.draw(hpTextInfo);
    window.draw(hpText);

    // Energy
    window.draw(energyShadow);
    window.draw(energyFrame);
    window.draw(energyBack);
    window.draw(energyFront);
    window.draw(energyHighlight);
    window.draw(energyLabelPlate);
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
