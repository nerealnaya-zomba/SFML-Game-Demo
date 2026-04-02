#include<PlayerUI.h>

#include<GameData.h>

#include<algorithm>
#include<cmath>
#include<cstdint>
#include<iomanip>
#include<sstream>

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
const sf::Color kStatsPanelInset(12, 10, 14, 226);
const sf::Color kStatsPanelAccent(176, 61, 77, 255);
const sf::Color kStatsLineFill(22, 18, 22, 216);
const sf::Color kObjectivePanelFill(13, 16, 24, 228);
const sf::Color kObjectivePanelBorder(77, 103, 134, 255);
const sf::Color kObjectiveAccent(232, 162, 88, 255);
const sf::Color kObjectiveProgressBack(33, 42, 57, 230);
const sf::Color kObjectiveProgressFill(221, 132, 64, 255);
const sf::Color kObjectiveProgressGlow(255, 226, 164, 190);
const sf::Color kObjectiveSigilCore(255, 206, 116, 255);
const sf::Color kObjectiveSigilGlow(255, 190, 118, 62);
const sf::Color kObjectiveToastFill(18, 12, 18, 236);
const sf::Color kObjectiveToastBorder(188, 123, 72, 255);
const sf::Color kObjectiveHintFill(16, 18, 26, 228);
const sf::Color kObjectiveHintBorder(110, 130, 154, 224);
const sf::Color kObjectiveHintAccent(232, 162, 88, 255);
const sf::Color kStatsHintFill(18, 16, 20, 224);
const sf::Color kStatsHintBorder(134, 100, 74, 220);
const sf::Color kStatsHintAccent(202, 142, 96, 255);
const sf::Color kInventoryHintFill(11, 17, 28, 228);
const sf::Color kInventoryHintBorder(78, 98, 128, 220);
const sf::Color kInventoryHintAccent(255, 198, 87, 255);
const sf::Vector2f kObjectiveHintSize(BASE_OBJECTIVE_PANEL_SIZE.x, 34.f);
const sf::Vector2f kStatsHintSize(188.f, 30.f);
constexpr float kInventoryHintHeight = 34.f;
constexpr float kHudPanelAutoHideSeconds = 5.f;

float cubicEaseOut(float value)
{
    const float t = std::clamp(value, 0.f, 1.f);
    return 1.f - std::pow(1.f - t, 3.f);
}

float inverseLerpClamped(float value, float minValue, float maxValue)
{
    if (maxValue <= minValue)
    {
        return 0.f;
    }

    return std::clamp((value - minValue) / (maxValue - minValue), 0.f, 1.f);
}

float lerp(float start, float end, float amount)
{
    return start + (end - start) * amount;
}

std::uint8_t toAlpha(float value)
{
    return static_cast<std::uint8_t>(std::clamp(value, 0.f, 255.f));
}

sf::Color withAlpha(sf::Color color, float alpha)
{
    color.a = toAlpha(alpha);
    return color;
}

std::string formatFloatValue(float value, int precision = 1)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}

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

void PlayerUI::rebuildStatLines()
{
    struct StatData
    {
        std::string label;
        std::string value;
        sf::Color accentColor;
    };

    const std::vector<StatData> stats = {
        {"Sanctuary", player->getCampaignBoonTitle(), sf::Color(255, 188, 104)},
        {"Energy gain", std::to_string(player->getEnergyGainValue()), sf::Color(118, 212, 230)},
        {"Damage", std::to_string(player->getDamageValue()), sf::Color(232, 169, 94)},
        {"Shot CD", std::to_string(player->getShootCooldownValue()) + " ms", sf::Color(196, 145, 94)},
        {"Shot cost", std::to_string(player->getShootCostValue()), sf::Color(226, 196, 102)},
        {"Bolt speed", formatFloatValue(player->getBulletSpeedValue(), 1), sf::Color(121, 212, 202)},
        {"Range", formatFloatValue(player->getBulletRangeValue(), 0), sf::Color(118, 185, 164)},
        {"Run speed", formatFloatValue(player->getMaxWalkSpeedValue(), 1), sf::Color(198, 213, 228)},
        {"Dash force", formatFloatValue(player->getDashForceValue(), 1), sf::Color(210, 115, 126)},
        {"Dash CD", std::to_string(player->getDashCooldownValue()) + " ms", sf::Color(186, 88, 100)},
        {"Jump power", formatFloatValue(player->getJumpPowerValue(), 1), sf::Color(176, 148, 222)},
        {"Air jumps", std::to_string(player->getExtraJumpCountValue()), sf::Color(156, 128, 214)},
        {"Slow fall", std::to_string(player->getSlowFallPercentValue()) + "%", sf::Color(176, 204, 190)}
    };

    statLines.clear();
    statLines.reserve(stats.size());

    for (const auto& stat : stats)
    {
        statLines.emplace_back(*data->gameFont);
        StatLineVisual& line = statLines.back();
        line.plate.setFillColor(kStatsLineFill);
        line.plate.setOutlineThickness(1.f);
        line.plate.setOutlineColor(sf::Color(stat.accentColor.r, stat.accentColor.g, stat.accentColor.b, 70));

        line.accent.setFillColor(stat.accentColor);

        styleHudText(line.label, 12, kHudTextSecondary);
        styleHudText(line.value, 13, kHudTextPrimary);
        line.label.setString(stat.label);
        line.value.setString(stat.value);
        line.value.setFillColor(stat.accentColor);
    }
}

void PlayerUI::updateStatsPanel()
{
    rebuildStatLines();

    if (statsPanelVisible_ && statsPanelVisibilityClock_.getElapsedTime().asSeconds() >= kHudPanelAutoHideSeconds)
    {
        statsPanelVisible_ = false;
    }

    const sf::Vector2f screenViewPos = camera->getScreenViewPos();
    const float deltaTime = std::min(statsPanelAnimationClock_.restart().asSeconds(), 0.05f);
    const float elapsed = uiAnimationClock.getElapsedTime().asSeconds();
    const float pulse = 0.85f + std::sin(elapsed * 2.8f + 0.35f) * 0.1f;

    if (statsPanelVisible_)
    {
        statsPanelReveal_ = std::min(1.f, statsPanelReveal_ + deltaTime * 5.4f);
    }
    else
    {
        statsPanelReveal_ = std::max(0.f, statsPanelReveal_ - deltaTime * 5.4f);
    }

    const float panelReveal = cubicEaseOut(statsPanelReveal_);
    const float panelWidth = BASE_STATS_PANEL_SIZE.x;
    const float gridWidth = panelWidth - 32.f;
    const float cardGap = BASE_STATS_GRID_GAP;
    const float cardWidth = (gridWidth - cardGap * static_cast<float>(BASE_STATS_GRID_COLUMNS - 1)) / static_cast<float>(BASE_STATS_GRID_COLUMNS);
    const float gridHeight = static_cast<float>(BASE_STATS_GRID_ROWS) * BASE_STATS_CARD_HEIGHT
        + static_cast<float>(BASE_STATS_GRID_ROWS - 1) * BASE_STATS_GRID_GAP;
    const float panelHeight = 64.f + gridHeight + BASE_STATS_PANEL_PADDING;

    const sf::Vector2f finalPanelPos = {
        screenViewPos.x + BASE_STATS_PANEL_OFFSET.x,
        screenViewPos.y
            + BASE_HP_BAR_OFFSET.y
            + BASE_RESOURCE_BAR_SIZE.y * 2.f
            + BASE_RESOURCE_BAR_GAP
            + BASE_STATS_PANEL_TOP_GAP_FROM_RESOURCES
    };
    const sf::Vector2f hintPos = {finalPanelPos.x + 2.f, finalPanelPos.y + 4.f};
    const float hintAlphaFactor = 1.f - panelReveal;
    const float hintAlpha = 255.f * hintAlphaFactor;

    statsHintShadow.setPosition({hintPos.x + 4.f, hintPos.y + 5.f});
    statsHintShadow.setSize(kStatsHintSize);
    statsHintShadow.setFillColor(withAlpha(sf::Color(0, 0, 0, 92), 92.f * hintAlphaFactor));

    statsHintBack.setPosition(hintPos);
    statsHintBack.setSize(kStatsHintSize);
    statsHintBack.setFillColor(withAlpha(kStatsHintFill, 224.f * hintAlphaFactor));
    statsHintBack.setOutlineThickness(2.f);
    statsHintBack.setOutlineColor(withAlpha(kStatsHintBorder, 220.f * hintAlphaFactor));

    statsHintAccent.setPosition(hintPos);
    statsHintAccent.setSize({kStatsHintSize.x, 4.f});
    statsHintAccent.setFillColor(withAlpha(kStatsHintAccent, 255.f * hintAlphaFactor));

    statsHintText.setString("Press P to show stats");
    setTextOriginToMiddle(statsHintText);
    statsHintText.setPosition({hintPos.x + kStatsHintSize.x / 2.f, hintPos.y + kStatsHintSize.y / 2.f - 2.f});
    statsHintText.setFillColor(withAlpha(sf::Color(242, 229, 212, 255), hintAlpha));
    statsHintText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * hintAlphaFactor));

    const float currentPanelWidth = lerp(kStatsHintSize.x, panelWidth, panelReveal);
    const float currentPanelHeight = lerp(kStatsHintSize.y, panelHeight, panelReveal);
    const sf::Vector2f panelPos = {
        finalPanelPos.x,
        finalPanelPos.y + (panelHeight - currentPanelHeight)
    };
    const float panelAlphaFactor = panelReveal;
    const float panelAlpha = 255.f * panelAlphaFactor;

    statsPanelShadow.setSize({currentPanelWidth, currentPanelHeight});
    statsPanelShadow.setPosition({panelPos.x + 6.f, panelPos.y + 8.f});
    statsPanelShadow.setFillColor(withAlpha(sf::Color(0, 0, 0, 116), 116.f * panelAlphaFactor));

    statsPanelBack.setSize({currentPanelWidth, currentPanelHeight});
    statsPanelBack.setPosition(panelPos);
    statsPanelBack.setFillColor(withAlpha(kStatsPanelInset, 226.f * panelAlphaFactor));
    statsPanelBack.setOutlineColor(withAlpha(sf::Color(96, 64, 52, 255), 255.f * panelAlphaFactor));

    statsHeaderAccent.setSize({currentPanelWidth, 6.f});
    statsHeaderAccent.setPosition(panelPos);
    statsHeaderAccent.setFillColor(withAlpha(sf::Color(
        kStatsPanelAccent.r,
        kStatsPanelAccent.g,
        kStatsPanelAccent.b,
        255
    ), (220.f + 28.f * pulse) * panelAlphaFactor));

    statsSideSigil.setSize({4.f, std::max(0.f, currentPanelHeight - 18.f)});
    statsSideSigil.setPosition({panelPos.x + 10.f, panelPos.y + 10.f});
    statsSideSigil.setFillColor(withAlpha(sf::Color(92, 38, 45, 218), 218.f * panelAlphaFactor));

    statsTitleText.setPosition({panelPos.x + 20.f, panelPos.y + 12.f});
    statsTitleText.setFillColor(withAlpha(sf::Color(245, 238, 226), panelAlpha));
    statsTitleText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * panelAlphaFactor));
    statsWeaponText.setString(player->getCurrentWeaponName());
    statsWeaponText.setFillColor(withAlpha(getInventoryQualityColor(player->getCurrentWeaponQuality()), panelAlpha));
    statsWeaponText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * panelAlphaFactor));
    statsWeaponText.setPosition({
        panelPos.x + currentPanelWidth - statsWeaponText.getLocalBounds().size.x - 20.f,
        panelPos.y + 16.f
    });

    statsDivider.setSize({std::max(0.f, currentPanelWidth - 32.f), 2.f});
    statsDivider.setPosition({panelPos.x + 16.f, panelPos.y + 46.f});
    statsDivider.setFillColor(withAlpha(sf::Color(78, 56, 46, 255), 255.f * panelAlphaFactor));

    const sf::Vector2f finalGridOrigin = {finalPanelPos.x + 16.f, finalPanelPos.y + 58.f};
    const float contentReveal = cubicEaseOut(inverseLerpClamped(panelReveal, 0.18f, 1.f));
    const float contentAlphaFactor = contentReveal;
    const float contentAlpha = 255.f * contentAlphaFactor;
    const sf::Vector2f collapsedCardOrigin = {
        panelPos.x + 16.f,
        panelPos.y + currentPanelHeight - BASE_STATS_CARD_HEIGHT - 8.f
    };
    const float collapsedCardWidth = std::min(cardWidth, kStatsHintSize.x - 26.f);
    const float collapsedCardHeight = 24.f;

    for (std::size_t index = 0; index < statLines.size(); ++index)
    {
        StatLineVisual& line = statLines[index];
        const bool isFifthColumnSingle = statLines.size() == 13u && index == 12u;
        const unsigned int column = isFifthColumnSingle
            ? 4u
            : static_cast<unsigned int>(index / 3u);
        const unsigned int row = isFifthColumnSingle
            ? 1u
            : static_cast<unsigned int>(index % 3u);
        const sf::Vector2f finalCardPos = {
            finalGridOrigin.x + static_cast<float>(column) * (cardWidth + cardGap),
            finalGridOrigin.y + static_cast<float>(row) * (BASE_STATS_CARD_HEIGHT + BASE_STATS_GRID_GAP)
        };
        const sf::Vector2f cardPos = {
            lerp(collapsedCardOrigin.x, finalCardPos.x, contentReveal),
            lerp(collapsedCardOrigin.y, finalCardPos.y, contentReveal)
        };
        const float currentCardWidth = lerp(collapsedCardWidth, cardWidth, contentReveal);
        const float currentCardHeight = lerp(collapsedCardHeight, BASE_STATS_CARD_HEIGHT, contentReveal);

        const sf::Color plateColor = ((column + row) % 2 == 0)
            ? sf::Color(23, 18, 23, 216)
            : sf::Color(17, 14, 18, 216);
        line.plate.setFillColor(withAlpha(plateColor, 216.f * contentAlphaFactor));
        line.plate.setOutlineColor(withAlpha(line.accent.getFillColor(), 70.f * contentAlphaFactor));
        line.plate.setSize({currentCardWidth, currentCardHeight});
        line.plate.setPosition(cardPos);

        line.accent.setSize({currentCardWidth, 4.f});
        line.accent.setPosition(line.plate.getPosition());
        line.accent.setFillColor(withAlpha(line.accent.getFillColor(), 255.f * contentAlphaFactor));

        setTextOriginToMiddle(line.label);
        setTextOriginToMiddle(line.value);
        line.label.setFillColor(withAlpha(kHudTextSecondary, contentAlpha));
        line.value.setFillColor(withAlpha(line.value.getFillColor(), contentAlpha));
        line.label.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * contentAlphaFactor));
        line.value.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * contentAlphaFactor));
        line.label.setPosition({
            line.plate.getPosition().x + currentCardWidth / 2.f,
            line.plate.getPosition().y + currentCardHeight * 0.32f
        });
        line.value.setPosition({
            line.plate.getPosition().x + currentCardWidth / 2.f,
            line.plate.getPosition().y + currentCardHeight * 0.70f
        });
    }
}

void PlayerUI::updateObjectivePanel()
{
    const CampaignObjectiveSnapshot snapshot = player->getCampaignSnapshot();
    const sf::Vector2f screenViewPos = camera->getScreenViewPos();
    const sf::Vector2f screenViewSize = camera->getScreenViewSize();
    const float elapsed = uiAnimationClock.getElapsedTime().asSeconds();
    const float deltaTime = std::min(objectivePanelAnimationClock_.restart().asSeconds(), 0.05f);
    const float pulse = 0.9f + std::sin(elapsed * 2.2f + 0.5f) * 0.08f;

    if (objectivePanelExpanded_ && objectivePanelVisibilityClock_.getElapsedTime().asSeconds() >= kHudPanelAutoHideSeconds)
    {
        objectivePanelExpanded_ = false;
    }

    if (objectivePanelExpanded_)
    {
        objectivePanelReveal_ = std::min(1.f, objectivePanelReveal_ + deltaTime * BASE_OBJECTIVE_PANEL_SLIDE_SPEED);
    }
    else
    {
        objectivePanelReveal_ = std::max(0.f, objectivePanelReveal_ - deltaTime * BASE_OBJECTIVE_PANEL_SLIDE_SPEED);
    }

    const float revealEase = cubicEaseOut(objectivePanelReveal_);
    const float inventoryPanelX = screenViewPos.x + screenViewSize.x - BASE_INVENTORY_PANEL_SIZE.x - BASE_INVENTORY_PANEL_OFFSET.x;

    const sf::Vector2f targetPanelPos = {
        inventoryPanelX - BASE_OBJECTIVE_PANEL_GAP_FROM_INVENTORY - BASE_OBJECTIVE_PANEL_SIZE.x,
        screenViewPos.y + BASE_OBJECTIVE_PANEL_TOP_OFFSET
    };
    const float hintAlphaFactor = 1.f - revealEase;
    const float hintAlpha = 255.f * hintAlphaFactor;
    const sf::Vector2f hintPos = {targetPanelPos.x, targetPanelPos.y + 2.f};
    const float hiddenY = targetPanelPos.y - BASE_OBJECTIVE_PANEL_SIZE.y - BASE_OBJECTIVE_PANEL_HIDDEN_MARGIN;
    const sf::Vector2f panelPos = {
        targetPanelPos.x,
        hiddenY + (targetPanelPos.y - hiddenY) * revealEase
    };
    const float panelAlphaFactor = revealEase;
    const float panelAlpha = 255.f * panelAlphaFactor;

    if (!objectiveStateInitialized_)
    {
        previousObjectiveChapter_ = snapshot.chapterTitle;
        previousObjectiveTask_ = snapshot.objective;
        objectiveStateInitialized_ = true;
    }
    else if (snapshot.chapterTitle != previousObjectiveChapter_ || snapshot.objective != previousObjectiveTask_)
    {
        previousObjectiveChapter_ = snapshot.chapterTitle;
        previousObjectiveTask_ = snapshot.objective;
        objectiveToastVisible_ = true;
        objectiveToastClock.restart();
    }

    objectiveHintShadow.setPosition({hintPos.x + 6.f, hintPos.y + 7.f});
    objectiveHintShadow.setSize(kObjectiveHintSize);
    objectiveHintShadow.setFillColor(withAlpha(sf::Color(0, 0, 0, 92), 92.f * hintAlphaFactor));

    objectiveHintBack.setPosition(hintPos);
    objectiveHintBack.setSize(kObjectiveHintSize);
    objectiveHintBack.setFillColor(withAlpha(kObjectiveHintFill, 228.f * hintAlphaFactor));
    objectiveHintBack.setOutlineThickness(2.f);
    objectiveHintBack.setOutlineColor(withAlpha(kObjectiveHintBorder, 224.f * hintAlphaFactor));

    objectiveHintAccent.setPosition(hintPos);
    objectiveHintAccent.setSize({kObjectiveHintSize.x, 6.f});
    objectiveHintAccent.setFillColor(withAlpha(kObjectiveHintAccent, 255.f * hintAlphaFactor));

    objectiveHintText.setString("Press O to show current objective");
    setTextOriginToMiddle(objectiveHintText);
    objectiveHintText.setPosition({
        hintPos.x + kObjectiveHintSize.x / 2.f,
        hintPos.y + kObjectiveHintSize.y / 2.f - 2.f
    });
    objectiveHintText.setFillColor(withAlpha(sf::Color(242, 229, 212, 255), hintAlpha));
    objectiveHintText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * hintAlphaFactor));

    objectivePanelShadow.setPosition({panelPos.x + 8.f, panelPos.y + 10.f});
    objectivePanelShadow.setFillColor(withAlpha(sf::Color(0, 0, 0, 116), 116.f * panelAlphaFactor));
    objectivePanelBack.setPosition(panelPos);
    objectivePanelBack.setFillColor(withAlpha(kObjectivePanelFill, 228.f * panelAlphaFactor));
    objectivePanelBack.setOutlineColor(withAlpha(kObjectivePanelBorder, 255.f * panelAlphaFactor));

    objectiveHeaderAccent.setPosition(panelPos);
    objectiveHeaderAccent.setFillColor(withAlpha(sf::Color(
        kObjectiveAccent.r,
        kObjectiveAccent.g,
        kObjectiveAccent.b,
        255
    ), (220.f + pulse * 24.f) * panelAlphaFactor));

    objectiveSigilGlow.setPosition({panelPos.x + 24.f, panelPos.y + 24.f});
    objectiveSigilGlow.setScale({pulse, pulse});
    objectiveSigilGlow.setFillColor(withAlpha(kObjectiveSigilGlow, 62.f * panelAlphaFactor));
    objectiveSigilCore.setPosition({panelPos.x + 24.f, panelPos.y + 24.f});
    objectiveSigilCore.setFillColor(withAlpha(kObjectiveSigilCore, 255.f * panelAlphaFactor));
    objectiveSigilCore.setOutlineColor(withAlpha(sf::Color(134, 82, 42, 220), 220.f * panelAlphaFactor));
    objectiveSigilRing.setPosition({panelPos.x + 24.f, panelPos.y + 24.f});
    objectiveSigilRing.setSectorOffset(std::fmod(elapsed * 0.12f, 1.f));
    objectiveSigilRing.setRotation(sf::degrees(std::sin(elapsed * 1.1f) * 7.f));
    objectiveSigilRing.setColor(withAlpha(sf::Color(
        246,
        183,
        104,
        255
    ), (120.f + pulse * 52.f) * panelAlphaFactor));
    objectiveSigilOrbitRing.setPosition({panelPos.x + 24.f, panelPos.y + 24.f});
    objectiveSigilOrbitRing.setSectorOffset(1.f - std::fmod(elapsed * 0.18f, 1.f));
    objectiveSigilOrbitRing.setRotation(sf::degrees(-elapsed * 28.f));
    objectiveSigilOrbitRing.setColor(withAlpha(sf::Color(
        112,
        168,
        232,
        255
    ), (78.f + pulse * 28.f) * panelAlphaFactor));

    objectiveTitleText.setString(snapshot.campaignTitle);
    objectiveChapterText.setString(snapshot.chapterTitle);
    objectiveNarrativeText.setString(snapshot.narrative);
    objectiveTaskText.setString(snapshot.objective);
    objectiveProgressText.setString(snapshot.progressText);
    objectiveRewardText.setString(snapshot.rewardText);
    objectiveTitleText.setFillColor(withAlpha(sf::Color(244, 231, 214), panelAlpha));
    objectiveChapterText.setFillColor(withAlpha(sf::Color(255, 208, 138), panelAlpha));
    objectiveNarrativeText.setFillColor(withAlpha(sf::Color(199, 207, 220), panelAlpha));
    objectiveTaskText.setFillColor(withAlpha(sf::Color(241, 233, 219), panelAlpha));
    objectiveProgressText.setFillColor(withAlpha(sf::Color(236, 221, 193), panelAlpha));
    objectiveRewardText.setFillColor(withAlpha(sf::Color(158, 194, 229), panelAlpha));
    objectiveTitleText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * panelAlphaFactor));
    objectiveChapterText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * panelAlphaFactor));
    objectiveNarrativeText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * panelAlphaFactor));
    objectiveTaskText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * panelAlphaFactor));
    objectiveProgressText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * panelAlphaFactor));
    objectiveRewardText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * panelAlphaFactor));

    objectiveTitleText.setPosition({panelPos.x + 46.f, panelPos.y + 10.f});
    objectiveChapterText.setPosition({panelPos.x + 14.f, panelPos.y + 34.f});
    objectiveNarrativeText.setPosition({panelPos.x + 14.f, panelPos.y + 55.f});
    objectiveTaskText.setPosition({panelPos.x + 14.f, panelPos.y + 75.f});

    objectiveDivider.setPosition({panelPos.x + 14.f, panelPos.y + 93.f});

    objectiveProgressBar.setPosition({panelPos.x + 14.f, panelPos.y + 101.f});
    objectiveProgressBar.setRatio(std::clamp(snapshot.progressRatio, 0.f, 1.f));
    objectiveProgressBar.setBackgroundColor(withAlpha(kObjectiveProgressBack, 230.f * panelAlphaFactor));
    objectiveProgressBar.setFrameColor(withAlpha(sf::Color(106, 125, 151, 200), 200.f * panelAlphaFactor));
    objectiveProgressBar.setColor(withAlpha(sf::Color(
        kObjectiveProgressFill.r,
        kObjectiveProgressFill.g,
        kObjectiveProgressFill.b,
        255
    ), (214.f + pulse * 34.f) * panelAlphaFactor));
    objectiveProgressGlow.setPosition({
        objectiveProgressBar.getPosition().x + 3.f,
        objectiveProgressBar.getPosition().y + 2.f
    });
    objectiveProgressGlow.setFillColor(withAlpha(kObjectiveProgressGlow, 190.f * panelAlphaFactor));

    const float progressWidth = objectiveProgressBar.getSize().x * std::clamp(snapshot.progressRatio, 0.f, 1.f);
    objectiveProgressGlow.setSize({std::max(0.f, progressWidth - 6.f), objectiveProgressGlow.getSize().y});

    objectiveProgressText.setPosition({
        panelPos.x + BASE_OBJECTIVE_PANEL_SIZE.x - objectiveProgressText.getLocalBounds().size.x - 14.f,
        panelPos.y + 97.f
    });
    objectiveRewardText.setPosition({panelPos.x + 14.f, panelPos.y + 117.f});
    objectiveDivider.setFillColor(withAlpha(sf::Color(70, 84, 103, 255), 255.f * panelAlphaFactor));

    const float toastElapsed = objectiveToastClock.getElapsedTime().asSeconds();
    if (objectiveToastVisible_ && toastElapsed > 4.f)
    {
        objectiveToastVisible_ = false;
    }

    if (objectiveToastVisible_)
    {
        float alphaFactor = 1.f;
        if (toastElapsed < 0.25f)
        {
            alphaFactor = std::clamp(toastElapsed / 0.25f, 0.f, 1.f);
        }
        else if (toastElapsed > 3.2f)
        {
            alphaFactor = std::clamp(1.f - (toastElapsed - 3.2f) / 0.8f, 0.f, 1.f);
        }

        const sf::Vector2f toastPos = {
            screenViewPos.x + screenViewSize.x * 0.5f - BASE_OBJECTIVE_TOAST_SIZE.x * 0.5f,
            targetPanelPos.y + BASE_OBJECTIVE_PANEL_SIZE.y + 10.f
        };
        const std::uint8_t alpha = static_cast<std::uint8_t>(std::clamp(alphaFactor * 255.f, 0.f, 255.f));

        objectiveToastShadow.setPosition({toastPos.x + 6.f, toastPos.y + 8.f});
        objectiveToastBack.setPosition(toastPos);
        objectiveToastAccent.setPosition(toastPos);

        objectiveToastShadow.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(96.f * alphaFactor)));
        objectiveToastBack.setFillColor(sf::Color(kObjectiveToastFill.r, kObjectiveToastFill.g, kObjectiveToastFill.b, alpha));
        objectiveToastBack.setOutlineColor(sf::Color(
            kObjectiveToastBorder.r,
            kObjectiveToastBorder.g,
            kObjectiveToastBorder.b,
            alpha
        ));
        objectiveToastAccent.setFillColor(sf::Color(
            kObjectiveAccent.r,
            kObjectiveAccent.g,
            kObjectiveAccent.b,
            alpha
        ));

        objectiveToastTitleText.setString("Objective Updated");
        objectiveToastBodyText.setString(snapshot.chapterTitle + "  |  " + snapshot.objective);
        objectiveToastTitleText.setFillColor(sf::Color(255, 223, 172, alpha));
        objectiveToastBodyText.setFillColor(sf::Color(235, 232, 224, alpha));
        objectiveToastTitleText.setPosition({toastPos.x + 18.f, toastPos.y + 10.f});
        objectiveToastBodyText.setPosition({toastPos.x + 18.f, toastPos.y + 34.f});
    }
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
    const float elapsed = uiAnimationClock.getElapsedTime().asSeconds();
    const float deltaTime = std::min(inventoryPanelAnimationClock_.restart().asSeconds(), 0.05f);
    const float pulse = 0.85f + std::sin(elapsed * 3.f) * 0.08f;

    if (inventoryPanelVisible_ && inventoryPanelVisibilityClock_.getElapsedTime().asSeconds() >= kHudPanelAutoHideSeconds)
    {
        inventoryPanelVisible_ = false;
    }

    if (inventoryPanelVisible_)
    {
        inventoryPanelReveal_ = std::min(1.f, inventoryPanelReveal_ + deltaTime * 5.4f);
    }
    else
    {
        inventoryPanelReveal_ = std::max(0.f, inventoryPanelReveal_ - deltaTime * 5.4f);
    }

    const unsigned int inventoryRows = player->getInventory().empty()
        ? 1u
        : static_cast<unsigned int>((player->getInventory().size() + BASE_INVENTORY_COLUMNS - 1) / BASE_INVENTORY_COLUMNS);

    const float panelWidth = BASE_INVENTORY_PANEL_SIZE.x;
    const float gridHeight = player->getInventory().empty()
        ? 56.f
        : inventoryRows * BASE_INVENTORY_SLOT_SIZE.y + (inventoryRows - 1) * BASE_INVENTORY_SLOT_GAP.y;
    const float panelHeight = 168.f + gridHeight;

    const sf::Vector2f finalPanelPos = {
        screenViewPos.x + screenViewSize.x - panelWidth - BASE_INVENTORY_PANEL_OFFSET.x,
        screenViewPos.y + BASE_INVENTORY_PANEL_OFFSET.y
    };
    const float revealEase = cubicEaseOut(inventoryPanelReveal_);
    const float hintAlphaFactor = 1.f - revealEase;
    const float panelAlphaFactor = revealEase;
    const float contentReveal = cubicEaseOut(inverseLerpClamped(revealEase, 0.22f, 1.f));
    const float contentAlphaFactor = contentReveal;
    const float panelAlpha = 255.f * panelAlphaFactor;
    const float contentAlpha = 255.f * contentAlphaFactor;
    const float currentPanelHeight = lerp(kInventoryHintHeight, panelHeight, revealEase);
    const sf::Vector2f panelPos = finalPanelPos;

    inventoryHintShadow.setPosition({finalPanelPos.x + 6.f, finalPanelPos.y + 7.f});
    inventoryHintShadow.setSize({panelWidth, kInventoryHintHeight});
    inventoryHintShadow.setFillColor(withAlpha(sf::Color(0, 0, 0, 92), 92.f * hintAlphaFactor));

    inventoryHintBack.setPosition(finalPanelPos);
    inventoryHintBack.setSize({panelWidth, kInventoryHintHeight});
    inventoryHintBack.setFillColor(withAlpha(kInventoryHintFill, 228.f * hintAlphaFactor));
    inventoryHintBack.setOutlineThickness(2.f);
    inventoryHintBack.setOutlineColor(withAlpha(kInventoryHintBorder, 220.f * hintAlphaFactor));

    inventoryHintAccent.setPosition(finalPanelPos);
    inventoryHintAccent.setSize({panelWidth, 6.f});
    inventoryHintAccent.setFillColor(withAlpha(kInventoryHintAccent, 255.f * hintAlphaFactor));

    inventoryHintText.setString("Press I to open inventory");
    setTextOriginToMiddle(inventoryHintText);
    inventoryHintText.setPosition({
        finalPanelPos.x + panelWidth / 2.f,
        finalPanelPos.y + kInventoryHintHeight / 2.f - 2.f
    });
    inventoryHintText.setFillColor(withAlpha(sf::Color(242, 229, 212, 255), 255.f * hintAlphaFactor));
    inventoryHintText.setOutlineColor(withAlpha(sf::Color(0, 0, 0, 180), 180.f * hintAlphaFactor));

    inventoryPanelShadow.setSize({panelWidth, currentPanelHeight});
    inventoryPanelShadow.setPosition({panelPos.x + 8.f, panelPos.y + 10.f});
    inventoryPanelShadow.setFillColor(withAlpha(sf::Color(0, 0, 0, 115), 115.f * panelAlphaFactor));

    inventoryPanelBack.setSize({panelWidth, currentPanelHeight});
    inventoryPanelBack.setPosition(panelPos);
    inventoryPanelBack.setFillColor(withAlpha(sf::Color(9, 15, 26, 230), 230.f * panelAlphaFactor));
    inventoryPanelBack.setOutlineColor(withAlpha(sf::Color(61, 81, 110, 255), panelAlpha));

    inventoryHeaderAccent.setSize({panelWidth, 8.f});
    inventoryHeaderAccent.setPosition(panelPos);
    inventoryHeaderAccent.setFillColor(withAlpha(sf::Color(255, 198, 87, 255), (220.f + 35.f * pulse) * panelAlphaFactor));

    const float collapsedContentY = panelPos.y + kInventoryHintHeight - 8.f;
    const auto revealY = [&](float finalY, float collapsedOffset) {
        return lerp(collapsedContentY - collapsedOffset, finalY, contentReveal);
    };

    goldChip.setSize({panelWidth - 36.f, 34.f});
    goldChip.setPosition({panelPos.x + 18.f, revealY(panelPos.y + 48.f, 0.f)});
    goldChip.setFillColor(withAlpha(sf::Color(35, 24, 11, 230), 230.f * contentAlphaFactor));
    goldChip.setOutlineColor(withAlpha(sf::Color(126, 93, 31, 255), 255.f * contentAlphaFactor));

    weaponChip.setSize({panelWidth - 36.f, 40.f});
    weaponChip.setPosition({panelPos.x + 18.f, revealY(panelPos.y + 88.f, 8.f)});
    weaponChip.setFillColor(withAlpha(sf::Color(28, 19, 37, 232), 232.f * contentAlphaFactor));
    weaponChip.setOutlineColor(withAlpha(getInventoryQualityColor(player->getCurrentWeaponQuality()), 255.f * contentAlphaFactor));

    inventoryDivider.setSize({panelWidth - 36.f, 2.f});
    inventoryDivider.setPosition({panelPos.x + 18.f, revealY(panelPos.y + 132.f, 12.f)});
    inventoryDivider.setFillColor(withAlpha(sf::Color(52, 65, 89, 255), 255.f * contentAlphaFactor));

    inventoryTitleText.setPosition({panelPos.x + 18.f, revealY(panelPos.y + 16.f, 8.f)});
    inventoryTitleText.setFillColor(withAlpha(sf::Color(245, 239, 227), contentAlpha));

    inventoryGoldText.setString(std::to_string(player->getGold()) + " gold");
    inventoryGoldText.setPosition({panelPos.x + 58.f, revealY(panelPos.y + 52.f, 0.f)});
    inventoryGoldText.setFillColor(withAlpha(sf::Color(255, 219, 120), contentAlpha));

    inventoryWeaponText.setString(player->getCurrentWeaponName());
    inventoryWeaponText.setPosition({panelPos.x + 30.f, revealY(panelPos.y + 94.f, 8.f)});
    inventoryWeaponText.setFillColor(withAlpha(getInventoryQualityColor(player->getCurrentWeaponQuality()), contentAlpha));

    inventoryWeaponHintText.setString("A / S - switch");
    inventoryWeaponHintText.setPosition({panelPos.x + 30.f, revealY(panelPos.y + 114.f, 10.f)});
    inventoryWeaponHintText.setFillColor(withAlpha(sf::Color(162, 171, 194), contentAlpha));

    goldCoinGlow.setScale({pulse, pulse});
    goldCoinGlow.setPosition({panelPos.x + 36.f, revealY(panelPos.y + 65.f, 2.f)});
    goldCoinGlow.setFillColor(withAlpha(sf::Color(255, 210, 106, 60), 60.f * contentAlphaFactor));
    goldCoinOuter.setPosition({panelPos.x + 36.f, revealY(panelPos.y + 65.f, 2.f)});
    goldCoinOuter.setFillColor(withAlpha(sf::Color(255, 200, 70), contentAlpha));
    goldCoinOuter.setOutlineColor(withAlpha(sf::Color(163, 102, 19), 255.f * contentAlphaFactor));
    goldCoinInner.setPosition({panelPos.x + 36.f, revealY(panelPos.y + 65.f, 2.f)});
    goldCoinInner.setFillColor(withAlpha(sf::Color(255, 233, 145), contentAlpha));
    goldCoinShine.setPosition({panelPos.x + 34.f, revealY(panelPos.y + 63.f, 2.f)});
    goldCoinShine.setRotation(sf::degrees(-24.f + std::sin(elapsed * 5.f) * 5.f));
    goldCoinShine.setFillColor(withAlpha(sf::Color(255, 250, 226, 220), 220.f * contentAlphaFactor));

    const sf::Vector2f gridStart = {panelPos.x + 18.f, panelPos.y + 146.f};
    const float collapsedGridY = panelPos.y + kInventoryHintHeight + 2.f;
    for (size_t index = 0; index < inventorySlots.size(); ++index)
    {
        const unsigned int row = static_cast<unsigned int>(index / BASE_INVENTORY_COLUMNS);
        const unsigned int column = static_cast<unsigned int>(index % BASE_INVENTORY_COLUMNS);
        const sf::Vector2f finalSlotPos = {
            gridStart.x + column * (BASE_INVENTORY_SLOT_SIZE.x + BASE_INVENTORY_SLOT_GAP.x),
            gridStart.y + row * (BASE_INVENTORY_SLOT_SIZE.y + BASE_INVENTORY_SLOT_GAP.y)
        };
        const sf::Vector2f slotPos = {
            finalSlotPos.x,
            lerp(collapsedGridY + static_cast<float>(row) * 10.f, finalSlotPos.y, contentReveal)
        };

        InventorySlotVisual& slot = inventorySlots[index];
        slot.shadow.setPosition({slotPos.x + 4.f, slotPos.y + 5.f});
        slot.shadow.setFillColor(withAlpha(sf::Color(0, 0, 0, 100), 100.f * contentAlphaFactor));
        slot.background.setPosition(slotPos);
        slot.background.setFillColor(withAlpha(sf::Color(18, 25, 39, 225), 225.f * contentAlphaFactor));
        slot.background.setOutlineColor(withAlpha(getInventoryQualityColor(player->getInventory()[index].quality), 255.f * contentAlphaFactor));
        slot.accent.setPosition({slotPos.x, slotPos.y + BASE_INVENTORY_SLOT_SIZE.y - slot.accent.getSize().y});
        slot.accent.setFillColor(withAlpha(getInventoryQualityColor(player->getInventory()[index].quality), 255.f * contentAlphaFactor));

        if(slot.icon)
        {
            slot.icon->setPosition({
                slotPos.x + BASE_INVENTORY_SLOT_SIZE.x / 2.f,
                slotPos.y + BASE_INVENTORY_SLOT_SIZE.y / 2.f - 2.f
            });
            slot.icon->setColor(withAlpha(sf::Color::White, contentAlpha));
        }
    }

    inventoryEmptyText.setString("No relics yet");
    setTextOriginToMiddle(inventoryEmptyText);
    inventoryEmptyText.setPosition({panelPos.x + panelWidth / 2.f, gridStart.y + gridHeight / 2.f});
    inventoryEmptyText.setFillColor(withAlpha(sf::Color(173, 181, 201), contentAlpha));
}

PlayerUI::PlayerUI(Player &p, GameCamera &c, GameData &d)
    : camera(&c)
    , data(&d)
    , player(&p)
    , hpTextInfo(*d.gameFont)
    , hpText(*d.gameFont)
    , energyTextInfo(*d.gameFont)
    , energyText(*d.gameFont)
    , statsTitleText(*d.gameFont)
    , statsWeaponText(*d.gameFont)
    , statsHintText(*d.gameFont)
    , objectiveTitleText(*d.gameFont)
    , objectiveChapterText(*d.gameFont)
    , objectiveNarrativeText(*d.gameFont)
    , objectiveTaskText(*d.gameFont)
    , objectiveProgressText(*d.gameFont)
    , objectiveRewardText(*d.gameFont)
    , objectiveHintText(*d.gameFont)
    , objectiveToastTitleText(*d.gameFont)
    , objectiveToastBodyText(*d.gameFont)
    , objectiveProgressBar({BASE_OBJECTIVE_PANEL_SIZE.x - 28.f, 8.f})
    , objectiveSigilRing(20.f, 0.68f, 72u)
    , objectiveSigilOrbitRing(25.f, 0.86f, 72u)
    , inventoryTitleText(*d.gameFont)
    , inventoryGoldText(*d.gameFont)
    , inventoryWeaponText(*d.gameFont)
    , inventoryWeaponHintText(*d.gameFont)
    , inventoryEmptyText(*d.gameFont)
    , inventoryHintText(*d.gameFont)
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

    statsPanelShadow.setFillColor(sf::Color(0, 0, 0, 116));

    statsPanelBack.setFillColor(kStatsPanelInset);
    statsPanelBack.setOutlineThickness(2.f);
    statsPanelBack.setOutlineColor(sf::Color(96, 64, 52, 255));

    statsHeaderAccent.setFillColor(kStatsPanelAccent);
    statsSideSigil.setFillColor(sf::Color(92, 38, 45, 218));
    statsDivider.setFillColor(sf::Color(78, 56, 46, 255));
    statsHintShadow.setFillColor(sf::Color(0, 0, 0, 92));
    statsHintBack.setFillColor(kStatsHintFill);
    statsHintBack.setOutlineThickness(2.f);
    statsHintBack.setOutlineColor(kStatsHintBorder);
    statsHintAccent.setFillColor(kStatsHintAccent);

    statsTitleText.setCharacterSize(18);
    statsTitleText.setFillColor(sf::Color(245, 238, 226));
    statsTitleText.setOutlineThickness(1.5f);
    statsTitleText.setOutlineColor(sf::Color(0, 0, 0, 180));
    statsTitleText.setString("Covenant stats");

    statsWeaponText.setCharacterSize(14);
    statsWeaponText.setFillColor(sf::Color(220, 203, 188));
    statsWeaponText.setOutlineThickness(1.2f);
    statsWeaponText.setOutlineColor(sf::Color(0, 0, 0, 180));

    styleHudText(statsHintText, 12, sf::Color(242, 229, 212));
    statsHintText.setString("Press P to show stats");

    objectivePanelShadow.setSize(BASE_OBJECTIVE_PANEL_SIZE);
    objectivePanelShadow.setFillColor(sf::Color(0, 0, 0, 116));

    objectivePanelBack.setSize(BASE_OBJECTIVE_PANEL_SIZE);
    objectivePanelBack.setFillColor(kObjectivePanelFill);
    objectivePanelBack.setOutlineThickness(2.f);
    objectivePanelBack.setOutlineColor(kObjectivePanelBorder);

    objectiveHeaderAccent.setSize({BASE_OBJECTIVE_PANEL_SIZE.x, 7.f});
    objectiveHeaderAccent.setFillColor(kObjectiveAccent);
    objectiveHintShadow.setFillColor(sf::Color(0, 0, 0, 92));
    objectiveHintBack.setFillColor(kObjectiveHintFill);
    objectiveHintBack.setOutlineThickness(2.f);
    objectiveHintBack.setOutlineColor(kObjectiveHintBorder);
    objectiveHintAccent.setFillColor(kObjectiveHintAccent);

    objectiveDivider.setSize({BASE_OBJECTIVE_PANEL_SIZE.x - 28.f, 2.f});
    objectiveDivider.setFillColor(sf::Color(70, 84, 103, 255));

    objectiveProgressBar.setShowBackgroundAndFrame(true);
    objectiveProgressBar.setBackgroundColor(kObjectiveProgressBack);
    objectiveProgressBar.setFrameThickness(1.f);
    objectiveProgressBar.setFrameColor(sf::Color(106, 125, 151, 200));
    objectiveProgressBar.setColor(kObjectiveProgressFill);
    objectiveProgressBar.setRatio(0.f);

    objectiveProgressGlow.setSize({0.f, 3.f});
    objectiveProgressGlow.setFillColor(kObjectiveProgressGlow);

    objectiveSigilGlow.setRadius(14.f);
    objectiveSigilGlow.setOrigin({objectiveSigilGlow.getRadius(), objectiveSigilGlow.getRadius()});
    objectiveSigilGlow.setFillColor(kObjectiveSigilGlow);

    objectiveSigilCore.setRadius(7.f);
    objectiveSigilCore.setOrigin({objectiveSigilCore.getRadius(), objectiveSigilCore.getRadius()});
    objectiveSigilCore.setFillColor(kObjectiveSigilCore);
    objectiveSigilCore.setOutlineThickness(2.f);
    objectiveSigilCore.setOutlineColor(sf::Color(134, 82, 42, 220));

    objectiveSigilRing.setOrigin({objectiveSigilRing.getRadius(), objectiveSigilRing.getRadius()});
    objectiveSigilRing.setColor(sf::Color(246, 183, 104, 176));
    objectiveSigilRing.setSectorSize(0.82f);

    objectiveSigilOrbitRing.setOrigin({objectiveSigilOrbitRing.getRadius(), objectiveSigilOrbitRing.getRadius()});
    objectiveSigilOrbitRing.setColor(sf::Color(112, 168, 232, 116));
    objectiveSigilOrbitRing.setSectorSize(0.38f);

    styleHudText(objectiveTitleText, 16, sf::Color(244, 231, 214));
    objectiveTitleText.setString("Restore the Heart Lantern");

    styleHudText(objectiveChapterText, 14, sf::Color(255, 208, 138));
    styleHudText(objectiveNarrativeText, 11, sf::Color(199, 207, 220));
    styleHudText(objectiveTaskText, 13, sf::Color(241, 233, 219));
    styleHudText(objectiveProgressText, 12, sf::Color(236, 221, 193));
    styleHudText(objectiveRewardText, 12, sf::Color(158, 194, 229));
    styleHudText(objectiveHintText, 13, sf::Color(242, 229, 212));
    objectiveHintText.setString("Press O to show current objective");

    objectiveToastShadow.setSize(BASE_OBJECTIVE_TOAST_SIZE);
    objectiveToastBack.setSize(BASE_OBJECTIVE_TOAST_SIZE);
    objectiveToastBack.setOutlineThickness(2.f);
    objectiveToastAccent.setSize({BASE_OBJECTIVE_TOAST_SIZE.x, 5.f});

    styleHudText(objectiveToastTitleText, 16, sf::Color(255, 223, 172));
    styleHudText(objectiveToastBodyText, 14, sf::Color(235, 232, 224));

    inventoryPanelShadow.setFillColor(sf::Color(0, 0, 0, 115));

    inventoryPanelBack.setFillColor(sf::Color(9, 15, 26, 230));
    inventoryPanelBack.setOutlineThickness(2.f);
    inventoryPanelBack.setOutlineColor(sf::Color(61, 81, 110, 255));

    inventoryHeaderAccent.setFillColor(sf::Color(255, 198, 87));
    inventoryDivider.setFillColor(sf::Color(52, 65, 89, 255));
    inventoryHintShadow.setFillColor(sf::Color(0, 0, 0, 92));
    inventoryHintBack.setFillColor(kInventoryHintFill);
    inventoryHintBack.setOutlineThickness(2.f);
    inventoryHintBack.setOutlineColor(kInventoryHintBorder);
    inventoryHintAccent.setFillColor(kInventoryHintAccent);

    goldChip.setFillColor(sf::Color(35, 24, 11, 230));
    goldChip.setOutlineThickness(1.f);
    goldChip.setOutlineColor(sf::Color(126, 93, 31, 255));

    weaponChip.setFillColor(sf::Color(28, 19, 37, 232));
    weaponChip.setOutlineThickness(1.f);
    weaponChip.setOutlineColor(sf::Color(118, 83, 122, 255));

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

    inventoryWeaponText.setCharacterSize(18);
    inventoryWeaponText.setFillColor(sf::Color(229, 225, 240));

    inventoryWeaponHintText.setCharacterSize(14);
    inventoryWeaponHintText.setFillColor(sf::Color(162, 171, 194));

    inventoryEmptyText.setCharacterSize(17);
    inventoryEmptyText.setFillColor(sf::Color(173, 181, 201));

    styleHudText(inventoryHintText, 13, sf::Color(242, 229, 212));
    inventoryHintText.setString("Press I to open inventory");
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

    if (statsHintBack.getFillColor().a > 0)
    {
        window.draw(statsHintShadow);
        window.draw(statsHintBack);
        window.draw(statsHintAccent);
        window.draw(statsHintText);
    }

    if (statsPanelReveal_ > 0.001f)
    {
        window.draw(statsPanelShadow);
        window.draw(statsPanelBack);
        window.draw(statsHeaderAccent);
        window.draw(statsSideSigil);
        window.draw(statsDivider);
        window.draw(statsTitleText);
        window.draw(statsWeaponText);
        for (auto& line : statLines)
        {
            if (line.plate.getFillColor().a == 0)
            {
                continue;
            }

            window.draw(line.plate);
            window.draw(line.accent);
            window.draw(line.label);
            window.draw(line.value);
        }
    }

    if (objectiveHintBack.getFillColor().a > 0)
    {
        window.draw(objectiveHintShadow);
        window.draw(objectiveHintBack);
        window.draw(objectiveHintAccent);
        window.draw(objectiveHintText);
    }

    // Objective panel
    if (objectivePanelReveal_ > 0.001f)
    {
        window.draw(objectivePanelShadow);
        window.draw(objectivePanelBack);
        window.draw(objectiveHeaderAccent);
        window.draw(objectiveSigilGlow);
        window.draw(objectiveSigilOrbitRing);
        window.draw(objectiveSigilRing);
        window.draw(objectiveSigilCore);
        window.draw(objectiveTitleText);
        window.draw(objectiveChapterText);
        window.draw(objectiveNarrativeText);
        window.draw(objectiveTaskText);
        window.draw(objectiveDivider);
        window.draw(objectiveProgressBar);
        window.draw(objectiveProgressGlow);
        window.draw(objectiveProgressText);
        window.draw(objectiveRewardText);
    }

    if (objectiveToastVisible_)
    {
        window.draw(objectiveToastShadow);
        window.draw(objectiveToastBack);
        window.draw(objectiveToastAccent);
        window.draw(objectiveToastTitleText);
        window.draw(objectiveToastBodyText);
    }

    if (inventoryHintBack.getFillColor().a > 0)
    {
        window.draw(inventoryHintShadow);
        window.draw(inventoryHintBack);
        window.draw(inventoryHintAccent);
        window.draw(inventoryHintText);
    }

    if (inventoryPanelReveal_ > 0.001f)
    {
        window.draw(inventoryPanelShadow);
        window.draw(inventoryPanelBack);
        window.draw(inventoryHeaderAccent);
        window.draw(goldChip);
        window.draw(weaponChip);
        window.draw(goldCoinGlow);
        window.draw(goldCoinOuter);
        window.draw(goldCoinInner);
        window.draw(goldCoinShine);
        window.draw(inventoryDivider);
        window.draw(inventoryTitleText);
        window.draw(inventoryGoldText);
        window.draw(inventoryWeaponText);
        window.draw(inventoryWeaponHintText);

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
}

void PlayerUI::update()
{
    updateCooldownRects();
    updateHP();
    updateEnergy();
    updateStatsPanel();
    updateObjectivePanel();
    updateInventoryPanel();
}

bool PlayerUI::handleEvent(const sf::Event& event)
{
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == sf::Keyboard::Scancode::O && !objectiveToggleKeyDown_)
        {
            objectiveToggleKeyDown_ = true;
            objectivePanelExpanded_ = !objectivePanelExpanded_;
            if (objectivePanelExpanded_)
            {
                objectivePanelVisibilityClock_.restart();
            }
            return true;
        }

        if (keyPressed->scancode == sf::Keyboard::Scancode::P && !statsToggleKeyDown_)
        {
            statsToggleKeyDown_ = true;
            statsPanelVisible_ = !statsPanelVisible_;
            if (statsPanelVisible_)
            {
                statsPanelVisibilityClock_.restart();
            }
            return true;
        }

        if (keyPressed->scancode == sf::Keyboard::Scancode::I && !inventoryToggleKeyDown_)
        {
            inventoryToggleKeyDown_ = true;
            inventoryPanelVisible_ = !inventoryPanelVisible_;
            if (inventoryPanelVisible_)
            {
                inventoryPanelVisibilityClock_.restart();
            }
            return true;
        }
    }

    if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>())
    {
        if (keyReleased->scancode == sf::Keyboard::Scancode::O)
        {
            objectiveToggleKeyDown_ = false;
        }

        if (keyReleased->scancode == sf::Keyboard::Scancode::P)
        {
            statsToggleKeyDown_ = false;
        }

        if (keyReleased->scancode == sf::Keyboard::Scancode::I)
        {
            inventoryToggleKeyDown_ = false;
        }
    }

    return false;
}
