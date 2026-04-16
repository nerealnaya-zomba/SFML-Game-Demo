#include <ChooseDestinationMenu.h>
#include <GameCamera.h>
#include <GameData.h>
#include <GameLevel.h>
#include <Player.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <utility>

namespace
{
constexpr float kPi = 3.14159265f;

std::string removeExtension(const std::string& fileName)
{
    const std::size_t dotPos = fileName.find_last_of('.');
    if (dotPos == std::string::npos)
    {
        return fileName;
    }

    return fileName.substr(0, dotPos);
}

int extractTrailingNumber(const std::string& value)
{
    std::size_t index = value.size();
    while (index > 0 && std::isdigit(static_cast<unsigned char>(value[index - 1])))
    {
        --index;
    }

    if (index == value.size())
    {
        return -1;
    }

    return std::stoi(value.substr(index));
}

std::string toRoman(int value)
{
    if (value <= 0)
    {
        return "?";
    }

    const std::pair<int, const char*> numerals[] = {
        {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"},
        {90, "XC"}, {50, "L"}, {40, "XL"}, {10, "X"}, {9, "IX"},
        {5, "V"}, {4, "IV"}, {1, "I"}
    };

    std::string result;
    int remaining = value;
    for (const auto& [number, numeral] : numerals)
    {
        while (remaining >= number)
        {
            result += numeral;
            remaining -= number;
        }
    }

    return result;
}

std::string titleCase(std::string text)
{
    bool upperNext = true;
    for (char& character : text)
    {
        if (character == '_' || character == '-')
        {
            character = ' ';
            upperNext = true;
            continue;
        }

        if (upperNext)
        {
            character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
            upperNext = false;
        }
        else
        {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }

        if (character == ' ')
        {
            upperNext = true;
        }
    }

    return text;
}

std::string formatLevelDisplayName(const std::string& rawLevelName)
{
    const std::string baseName = removeExtension(rawLevelName);
    const int levelIndex = extractTrailingNumber(baseName);

    if (baseName.rfind("level", 0) == 0 && levelIndex > 0)
    {
        return "Level " + toRoman(levelIndex);
    }

    return titleCase(baseName);
}

std::string formatLevelBadge(const std::string& rawLevelName)
{
    const int levelIndex = extractTrailingNumber(removeExtension(rawLevelName));
    if (levelIndex > 0)
    {
        return toRoman(levelIndex);
    }

    const std::string pretty = formatLevelDisplayName(rawLevelName);
    return pretty.substr(0, std::min<std::size_t>(pretty.size(), 8));
}

std::string scanToLabel(sf::Keyboard::Scan scan)
{
    switch (scan)
    {
    case sf::Keyboard::Scan::Q:
        return "Q";
    case sf::Keyboard::Scan::W:
        return "W";
    case sf::Keyboard::Scan::A:
        return "A";
    case sf::Keyboard::Scan::D:
        return "D";
    case sf::Keyboard::Scan::E:
        return "E";
    case sf::Keyboard::Scan::R:
        return "R";
    case sf::Keyboard::Scan::Enter:
        return "Enter";
    case sf::Keyboard::Scan::Escape:
        return "Esc";
    case sf::Keyboard::Scan::Left:
        return "Left";
    case sf::Keyboard::Scan::Right:
        return "Right";
    default:
        return "Key";
    }
}

void configureText(sf::Text& text, unsigned int size, sf::Color color, float letterSpacing = 1.f)
{
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setLetterSpacing(letterSpacing);
}
}

void ChooseDestinationMenu::handleEvents(const sf::Event& ev)
{
    if (!isOpened)
    {
        return;
    }

    handleMoveEvents(ev);
    handleActivateEvent(ev);
    handleCloseEvent(ev);
}

void ChooseDestinationMenu::handleMoveEvents(const sf::Event& ev)
{
    if (const auto* keyPressed = ev.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == moveLeftKey || keyPressed->scancode == sf::Keyboard::Scan::Left)
        {
            moveLevelItLeft();
        }
        else if (keyPressed->scancode == moveRightKey || keyPressed->scancode == sf::Keyboard::Scan::Right)
        {
            moveLevelItRight();
        }
    }
}

void ChooseDestinationMenu::handleActivateEvent(const sf::Event& ev)
{
    if (const auto* keyPressed = ev.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == selectKey)
        {
            currentSelectedElementToDesiredDestination();
            player->setPortalDestination(desiredDestination);
        }
    }
}

void ChooseDestinationMenu::handleCloseEvent(const sf::Event& ev)
{
    if (const auto* keyPressed = ev.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == closeKey || keyPressed->scancode == sf::Keyboard::Scan::Escape)
        {
            close();
        }
    }
}

ChooseDestinationMenu::ChooseDestinationMenu(
    GameData& d,
    GameCamera& c,
    GameLevelManager& lm,
    Player& p,
    sf::Keyboard::Scan mvLeftKey,
    sf::Keyboard::Scan mvRightKey,
    sf::Keyboard::Scan slctKey,
    sf::Keyboard::Scan clsKey
)
    : data(&d)
    , camera(&c)
    , manager(&lm)
    , player(&p)
    , moveLeftKey(mvLeftKey)
    , moveRightKey(mvRightKey)
    , selectKey(slctKey)
    , closeKey(clsKey)
    , titleText(*d.gameFont)
    , subtitleText(*d.gameFont)
    , displayingLevelName(*d.gameFont)
    , destinationStateText(*d.gameFont)
    , destinationDescriptionText(*d.gameFont)
    , legendText(*d.gameFont)
    , levelIt(levels.end())
{
    if (!d.gameFont)
    {
        throw std::runtime_error("Font not loaded");
    }

    configureText(titleText, 38, sf::Color(236, 224, 205, 255), 1.08f);
    titleText.setStyle(sf::Text::Bold);
    titleText.setString("CHOOSE DESTINATION");

    configureText(subtitleText, 18, sf::Color(182, 149, 118, 235), 1.16f);
    subtitleText.setString("Farm grave-gold, forge relics, then reopen deeper hunting grounds.");

    configureText(displayingLevelName, 34, sf::Color(244, 230, 210, 255), 1.02f);
    displayingLevelName.setStyle(sf::Text::Bold);

    configureText(destinationStateText, 20, sf::Color(196, 173, 153, 245), 1.02f);
    configureText(destinationDescriptionText, 18, sf::Color(154, 138, 128, 232), 1.01f);
    destinationDescriptionText.setString("Choose where the red gate will answer your call.");

    configureText(legendText, 17, sf::Color(148, 132, 123, 224), 1.02f);

    updateDisplayedTexts();
}

void ChooseDestinationMenu::open()
{
    isOpened = true;
    syncUnlockedStates();
    checkWherePlayer();

    for (auto& level : levels)
    {
        level.leveldestination.isSelected = false;
    }

    if (levels.empty())
    {
        levelIt = levels.end();
        updateDisplayedTexts();
        return;
    }

    auto preferredIt = std::find_if(levels.begin(), levels.end(), [](const LevelDestinationRect& level) {
        return level.leveldestination.isChoosed;
    });

    if (preferredIt == levels.end())
    {
        preferredIt = std::find_if(levels.begin(), levels.end(), [](const LevelDestinationRect& level) {
            return level.leveldestination.isPlayerThere;
        });
    }

    if (preferredIt == levels.end())
    {
        preferredIt = levels.begin();
    }

    levelIt = preferredIt;
    levelIt->leveldestination.isSelected = true;
    updateDisplayedTexts();
}

void ChooseDestinationMenu::close()
{
    isOpened = false;
    for (auto& level : levels)
    {
        level.leveldestination.isSelected = false;
    }
}

void ChooseDestinationMenu::addLevelInVector(const GameLevel& level, sf::Texture& icon)
{
    addLevelInVector(level, static_cast<const sf::Texture&>(icon));
}

void ChooseDestinationMenu::addLevelInVector(const GameLevel& level, const sf::Texture& icon)
{
    LevelDestinationRect destination(icon);
    destination.leveldestination.level = &level;
    destination.leveldestination.isOpened = player->isLevelUnlocked(level.levelName);
    destination.leveldestination.isVisible = true;
    applyIconScale(destination.icon);

    levels.push_back(destination);
    levelIt = levels.end();
    initializeIsChoosed();
    updateDisplayedTexts();
}

void ChooseDestinationMenu::drawLevelDestinations(sf::RenderWindow& window)
{
    drawLevelDestinationsBackground(window);
    drawLevelDestinationsLevels(window);
}

void ChooseDestinationMenu::drawLevelDestinationsBackground(sf::RenderWindow& window)
{
    window.draw(overlayVeil);

    const float pulse = 0.9f + std::sin(animationTime * 1.6f) * 0.05f;

    sf::CircleShape eclipse(135.f);
    eclipse.setOrigin({eclipse.getRadius(), eclipse.getRadius()});
    eclipse.setScale({1.9f, 1.1f});
    eclipse.setPosition({panelPosition.x + panelSize.x * 0.5f, panelPosition.y + 54.f});
    eclipse.setFillColor(sf::Color(148, 47, 35, 40));
    window.draw(eclipse);

    sf::RectangleShape sigilBeam({14.f, panelSize.y * 0.54f});
    sigilBeam.setOrigin({sigilBeam.getSize().x / 2.f, 0.f});
    sigilBeam.setPosition({previewPosition.x + previewSize.x * 0.5f, previewPosition.y + 8.f});
    sigilBeam.setScale({pulse, 1.f});
    sigilBeam.setFillColor(sf::Color(176, 76, 56, 80));
    window.draw(sigilBeam);

    window.draw(panelShadow);
    window.draw(panelFrame);
    window.draw(panelInset);
    window.draw(titleBand);
    window.draw(footerBand);
    window.draw(previewFrame);
    window.draw(previewInset);

    if (levelIt != levels.end())
    {
        sf::Sprite preview(levelIt->icon.getTexture());
        const sf::Vector2f textureSize = static_cast<sf::Vector2f>(preview.getTexture().getSize());
        const float previewScale = std::min(
            (previewSize.x - 18.f) / textureSize.x,
            (previewSize.y - 18.f) / textureSize.y
        );
        preview.setScale({previewScale, previewScale});
        preview.setColor(levelIt->leveldestination.isOpened
            ? sf::Color(228, 218, 210, 255)
            : sf::Color(126, 118, 116, 210));
        setSpriteOriginToMiddle(preview);
        preview.setPosition({
            previewPosition.x + previewSize.x / 2.f,
            previewPosition.y + previewSize.y / 2.f
        });
        window.draw(preview);

        previewVeil.setFillColor(levelIt->leveldestination.isOpened
            ? sf::Color(32, 10, 12, 76)
            : sf::Color(8, 8, 12, 132));
    }

    window.draw(previewVeil);

    sf::CircleShape outerRune(84.f);
    outerRune.setOrigin({outerRune.getRadius(), outerRune.getRadius()});
    outerRune.setPosition({
        panelPosition.x + panelSize.x - 116.f,
        previewPosition.y + 82.f
    });
    outerRune.setScale({1.18f, 0.64f});
    outerRune.setFillColor(sf::Color::Transparent);
    outerRune.setOutlineThickness(2.f);
    outerRune.setOutlineColor(sf::Color(148, 90, 70, 58));
    window.draw(outerRune);

    sf::CircleShape innerRune(52.f);
    innerRune.setOrigin({innerRune.getRadius(), innerRune.getRadius()});
    innerRune.setPosition(outerRune.getPosition());
    innerRune.setScale({1.f, 0.52f});
    innerRune.setFillColor(sf::Color::Transparent);
    innerRune.setOutlineThickness(1.5f);
    innerRune.setOutlineColor(sf::Color(241, 203, 153, 30));
    window.draw(innerRune);
}

void ChooseDestinationMenu::drawLevelDestinationsLevels(sf::RenderWindow& window)
{
    for (auto& level : levels)
    {
        level.draw(window);

        sf::Text badge(*data->gameFont);
        configureText(badge, 16, sf::Color(238, 223, 205, 240), 1.02f);
        badge.setStyle(sf::Text::Bold);
        badge.setString(formatLevelBadge(level.leveldestination.level->levelName));

        const sf::Vector2f cardPosition = level.selectionRect.getPosition();
        const sf::Vector2f cardSize = level.selectionRect.getSize();
        badge.setPosition({
            cardPosition.x + cardSize.x * 0.5f,
            cardPosition.y + cardSize.y - 16.f
        });
        setTextOriginToMiddle(badge);
        window.draw(badge);
    }
}

void ChooseDestinationMenu::update()
{
    if (!isOpened)
    {
        return;
    }

    animationTime += 1.f / static_cast<float>(WINDOW_FPS);
    syncUnlockedStates();
    positioningLevelDestinations();
    checkWherePlayer();
    updateDisplayedTexts();
}

void ChooseDestinationMenu::draw(sf::RenderWindow& w)
{
    if (!isOpened)
    {
        return;
    }

    drawLevelDestinations(w);
    drawLevelDestinationsText(w);
}

bool ChooseDestinationMenu::getIsOpened()
{
    return isOpened;
}

std::optional<std::string> ChooseDestinationMenu::getSelectedLevel()
{
    return desiredDestination;
}

void ChooseDestinationMenu::LevelDestinationRect::draw(sf::RenderWindow& w) const
{
    if (!leveldestination.isVisible)
    {
        return;
    }

    const sf::Vector2f cardPosition = selectionRect.getPosition();
    const sf::Vector2f cardSize = selectionRect.getSize();

    sf::RectangleShape shadow({cardSize.x + 10.f, cardSize.y + 10.f});
    shadow.setPosition({cardPosition.x - 5.f, cardPosition.y + 6.f});
    shadow.setFillColor(sf::Color(0, 0, 0, 86));
    w.draw(shadow);

    sf::RectangleShape card(cardSize);
    card.setPosition(cardPosition);
    card.setFillColor(sf::Color(10, 8, 11, 228));
    card.setOutlineThickness(1.f);
    card.setOutlineColor(sf::Color(84, 57, 48, 205));
    w.draw(card);

    w.draw(icon);

    sf::RectangleShape bottomShade({cardSize.x, cardSize.y * 0.34f});
    bottomShade.setPosition({cardPosition.x, cardPosition.y + cardSize.y - bottomShade.getSize().y});
    bottomShade.setFillColor(sf::Color(5, 4, 5, 138));
    w.draw(bottomShade);

    sf::RectangleShape veil(cardSize);
    veil.setPosition(cardPosition);
    if (leveldestination.isOpened)
    {
        veil.setFillColor(leveldestination.isSelected ? sf::Color(60, 22, 18, 40) : sf::Color(6, 5, 7, 92));
    }
    else
    {
        veil.setFillColor(leveldestination.isSelected ? sf::Color(20, 17, 21, 126) : sf::Color(10, 10, 14, 168));
    }
    w.draw(veil);

    if (leveldestination.isChoosed)
    {
        w.draw(currentLevelMarkRect);
    }

    if (leveldestination.isSelected)
    {
        w.draw(selectionRect);
    }

    if (leveldestination.isPlayerThere)
    {
        sf::CircleShape currentGroundMark(7.f);
        currentGroundMark.setOrigin({currentGroundMark.getRadius(), currentGroundMark.getRadius()});
        currentGroundMark.setPosition({cardPosition.x + 14.f, cardPosition.y + 14.f});
        currentGroundMark.setFillColor(sf::Color(196, 218, 228, 245));
        currentGroundMark.setOutlineThickness(2.f);
        currentGroundMark.setOutlineColor(sf::Color(86, 115, 125, 210));
        w.draw(currentGroundMark);
    }

    if (leveldestination.isChoosed)
    {
        sf::CircleShape attunedMark(7.f);
        attunedMark.setOrigin({attunedMark.getRadius(), attunedMark.getRadius()});
        attunedMark.setPosition({cardPosition.x + cardSize.x - 14.f, cardPosition.y + 14.f});
        attunedMark.setFillColor(sf::Color(234, 126, 92, 245));
        attunedMark.setOutlineThickness(2.f);
        attunedMark.setOutlineColor(sf::Color(123, 39, 30, 225));
        w.draw(attunedMark);
    }

    if (!leveldestination.isOpened)
    {
        sf::RectangleShape chainBar({cardSize.x - 24.f, 5.f});
        chainBar.setOrigin({chainBar.getSize().x / 2.f, chainBar.getSize().y / 2.f});
        chainBar.setPosition({cardPosition.x + cardSize.x / 2.f, cardPosition.y + cardSize.y / 2.f});
        chainBar.setFillColor(sf::Color(182, 162, 132, 210));
        chainBar.setRotation(sf::degrees(-18.f));
        w.draw(chainBar);

        sf::RectangleShape chainBarMirror(chainBar);
        chainBarMirror.setRotation(sf::degrees(18.f));
        w.draw(chainBarMirror);
    }
}

void ChooseDestinationMenu::currentSelectedElementToDesiredDestination()
{
    if (levels.empty() || levelIt == levels.end())
    {
        desiredDestination = std::nullopt;
        return;
    }

    if (!levelIt->leveldestination.isOpened)
    {
        updateDisplayedTexts();
        return;
    }

    desiredDestination = std::make_optional<std::string>(levelIt->leveldestination.level->levelName);
    for (auto& level : levels)
    {
        level.leveldestination.isChoosed = false;
    }
    levelIt->leveldestination.isChoosed = true;
    updateDisplayedTexts();
}

std::string ChooseDestinationMenu::getCurrentLevelName()
{
    return manager->getIteratorReference()->second->levelName;
}

void ChooseDestinationMenu::mountSelectionRect(sf::RectangleShape& sr, const sf::FloatRect& bounds)
{
    sr.setPosition(bounds.position);
    sr.setSize(bounds.size);
    sr.setFillColor(sf::Color::Transparent);
    sr.setOutlineColor(BASE_SELECTION_COLOR);
    sr.setOutlineThickness(BASE_SELECTION_SIZE);
}

void ChooseDestinationMenu::mountCurrentLevelMarkRect(sf::RectangleShape& sr, const sf::FloatRect& bounds)
{
    sr.setPosition({bounds.position.x + 4.f, bounds.position.y + 4.f});
    sr.setSize({bounds.size.x - 8.f, bounds.size.y - 8.f});
    sr.setFillColor(sf::Color::Transparent);
    sr.setOutlineColor(BASE_LEVELMARK_COLOR);
    sr.setOutlineThickness(BASE_LEVELMARK_SIZE);
}

void ChooseDestinationMenu::setDisplayingLevelNameString(const std::string& str)
{
    displayingLevelName.setString(str);
}

void ChooseDestinationMenu::applyIconScale(sf::Sprite& icon)
{
    const sf::Vector2f iconSize = static_cast<sf::Vector2f>(icon.getTexture().getSize());
    const sf::Vector2f scale = {
        BASE_DESTINATION_ICON_SIZE.x / iconSize.x,
        BASE_DESTINATION_ICON_SIZE.y / iconSize.y
    };
    icon.setScale(scale);
}

void ChooseDestinationMenu::initializeIsChoosed()
{
    checkWherePlayer();
    for (auto& level : levels)
    {
        level.leveldestination.isChoosed = level.leveldestination.isPlayerThere;
    }

    auto choosedIt = std::find_if(levels.begin(), levels.end(), [](const LevelDestinationRect& level) {
        return level.leveldestination.isChoosed;
    });
    if (choosedIt != levels.end())
    {
        desiredDestination = choosedIt->leveldestination.level->levelName;
    }
}

void ChooseDestinationMenu::moveLevelItLeft()
{
    if (levels.empty() || levelIt == levels.end())
    {
        return;
    }

    levelIt->leveldestination.isSelected = false;
    if (levelIt == levels.begin())
    {
        levelIt = levels.end() - 1;
    }
    else
    {
        --levelIt;
    }

    levelIt->leveldestination.isSelected = true;
    updateDisplayedTexts();
}

void ChooseDestinationMenu::moveLevelItRight()
{
    if (levels.empty() || levelIt == levels.end())
    {
        return;
    }

    levelIt->leveldestination.isSelected = false;
    if (levelIt == levels.end() - 1)
    {
        levelIt = levels.begin();
    }
    else
    {
        ++levelIt;
    }

    levelIt->leveldestination.isSelected = true;
    updateDisplayedTexts();
}

void ChooseDestinationMenu::positioningLevelDestinations()
{
    positioningLevelDestinationsBackground();
    positioningLevelDestinationsLevels();
    positioningLevelDestinationsText();
}

void ChooseDestinationMenu::positioningLevelDestinationsBackground()
{
    const sf::Vector2f viewSize = camera->getScreenViewSize();
    const sf::Vector2f viewCenter = camera->getCameraCenterPos();
    const sf::Vector2f viewTopLeft = {
        viewCenter.x - viewSize.x / 2.f,
        viewCenter.y - viewSize.y / 2.f
    };

    overlayVeil.setPosition(viewTopLeft);
    overlayVeil.setSize(viewSize);
    overlayVeil.setFillColor(sf::Color(4, 3, 6, 138));

    panelSize = {
        std::min(viewSize.x - 150.f, 1180.f),
        std::min(viewSize.y - 110.f, 420.f)
    };
    panelPosition = {
        viewCenter.x - panelSize.x / 2.f,
        viewTopLeft.y + BASE_DESTINATION_BACKGROUND_TOPMARGIN
    };

    panelShadow.setPosition({panelPosition.x, panelPosition.y + 12.f});
    panelShadow.setSize(panelSize);
    panelShadow.setFillColor(sf::Color(0, 0, 0, 128));

    panelFrame.setPosition(panelPosition);
    panelFrame.setSize(panelSize);
    panelFrame.setFillColor(sf::Color(16, 9, 11, 226));
    panelFrame.setOutlineThickness(2.f);
    panelFrame.setOutlineColor(sf::Color(137, 96, 71, 220));

    panelInset.setPosition({panelPosition.x + 14.f, panelPosition.y + 50.f});
    panelInset.setSize({panelSize.x - 28.f, panelSize.y - 64.f});
    panelInset.setFillColor(sf::Color(9, 6, 8, 156));
    panelInset.setOutlineThickness(1.f);
    panelInset.setOutlineColor(sf::Color(82, 42, 36, 170));

    titleBand.setPosition({panelPosition.x + 14.f, panelPosition.y + 12.f});
    titleBand.setSize({panelSize.x - 28.f, 34.f});
    titleBand.setFillColor(sf::Color(52, 13, 15, 220));

    footerBand.setPosition({panelPosition.x + 16.f, panelPosition.y + panelSize.y - 62.f});
    footerBand.setSize({panelSize.x - 32.f, 34.f});
    footerBand.setFillColor(sf::Color(14, 10, 11, 185));

    previewSize = {
        std::min(430.f, panelSize.x * 0.39f),
        176.f
    };
    previewPosition = {
        panelPosition.x + 28.f,
        panelPosition.y + 72.f
    };

    previewFrame.setPosition(previewPosition);
    previewFrame.setSize(previewSize);
    previewFrame.setFillColor(sf::Color(8, 6, 9, 220));
    previewFrame.setOutlineThickness(2.f);
    previewFrame.setOutlineColor(sf::Color(160, 108, 75, 215));

    previewInset.setPosition({previewPosition.x + 8.f, previewPosition.y + 8.f});
    previewInset.setSize({previewSize.x - 16.f, previewSize.y - 16.f});
    previewInset.setFillColor(sf::Color(6, 5, 7, 210));

    previewVeil.setPosition({previewPosition.x + 8.f, previewPosition.y + 8.f});
    previewVeil.setSize({previewSize.x - 16.f, previewSize.y - 16.f});
    previewVeil.setFillColor(sf::Color(32, 10, 12, 76));
}

void ChooseDestinationMenu::positioningLevelDestinationsLevels()
{
    if (levels.empty())
    {
        return;
    }

    const float rowAvailableWidth = panelSize.x - 60.f;
    const float gap = levels.size() > 1 ? 16.f : 0.f;
    const float calculatedWidth =
        (rowAvailableWidth - gap * static_cast<float>(levels.size() - 1)) / static_cast<float>(levels.size());
    const float cardWidth = std::clamp(calculatedWidth, 104.f, 148.f);
    const float cardHeight = std::clamp(cardWidth * 0.58f, 64.f, 86.f);
    const float totalWidth = cardWidth * static_cast<float>(levels.size()) + gap * static_cast<float>(levels.size() - 1);
    const float startX = panelPosition.x + (panelSize.x - totalWidth) / 2.f;
    const float rowY = panelPosition.y + panelSize.y - 156.f;

    for (std::size_t index = 0; index < levels.size(); ++index)
    {
        auto& level = levels[index];
        const sf::Vector2f cardPosition = {
            startX + static_cast<float>(index) * (cardWidth + gap),
            rowY
        };
        const sf::FloatRect bounds(cardPosition, {cardWidth, cardHeight});

        mountSelectionRect(level.selectionRect, bounds);
        mountCurrentLevelMarkRect(level.currentLevelMarkRect, bounds);

        const sf::Vector2f textureSize = static_cast<sf::Vector2f>(level.icon.getTexture().getSize());
        const float previewScale = std::min(
            (cardWidth - 10.f) / textureSize.x,
            (cardHeight - 10.f) / textureSize.y
        );
        level.icon.setScale({previewScale, previewScale});
        setSpriteOriginToMiddle(level.icon);
        level.icon.setPosition({
            cardPosition.x + cardWidth / 2.f,
            cardPosition.y + cardHeight / 2.f - 4.f
        });
        if (!level.leveldestination.isOpened)
        {
            level.icon.setColor(level.leveldestination.isSelected
                ? sf::Color(166, 160, 156, 228)
                : sf::Color(108, 104, 110, 212));
        }
        else
        {
            level.icon.setColor(level.leveldestination.isSelected
                ? sf::Color(255, 255, 255, 255)
                : sf::Color(212, 205, 196, 238));
        }
    }
}

void ChooseDestinationMenu::positioningLevelDestinationsText()
{
    titleText.setPosition({panelPosition.x + 38.f, panelPosition.y + 14.f});
    subtitleText.setPosition({panelPosition.x + 38.f, panelPosition.y + 52.f});

    displayingLevelName.setPosition({previewPosition.x + previewSize.x + 42.f, previewPosition.y + 12.f});
    destinationStateText.setPosition({displayingLevelName.getPosition().x, displayingLevelName.getPosition().y + 52.f});
    destinationDescriptionText.setPosition({
        displayingLevelName.getPosition().x,
        displayingLevelName.getPosition().y + 94.f
    });
    legendText.setPosition({panelPosition.x + 28.f, panelPosition.y + panelSize.y - 58.f});
}

void ChooseDestinationMenu::syncUnlockedStates()
{
    for (auto& level : levels)
    {
        if (!level.leveldestination.level)
        {
            continue;
        }

        level.leveldestination.isOpened = player->isLevelUnlocked(level.leveldestination.level->levelName);
        if (!level.leveldestination.isOpened)
        {
            level.leveldestination.isChoosed = false;
        }
    }

    if (desiredDestination.has_value() && !player->isLevelUnlocked(*desiredDestination))
    {
        desiredDestination = std::nullopt;
    }
}

void ChooseDestinationMenu::checkWherePlayer()
{
    const std::string currentLevelName = getCurrentLevelName();

    for (auto& level : levels)
    {
        level.leveldestination.isPlayerThere = (level.leveldestination.level->levelName == currentLevelName);
    }
}

void ChooseDestinationMenu::updateDisplayedTexts()
{
    if (levels.empty() || levelIt == levels.end())
    {
        setDisplayingLevelNameString("No destinations revealed");
        destinationStateText.setString("The gate has nowhere to answer.");
        destinationDescriptionText.setString("Reclaim the farm and deeper routes will reveal themselves here.");
        legendText.setString("Open a route in the world before you can bind the portal.");
        return;
    }

    const auto& destination = levelIt->leveldestination;
    const CampaignLevelInfo& levelInfo = CampaignProgress::getLevelInfo(destination.level->levelName);
    const bool hasCampaignInfo = levelInfo.levelName != "unknown";
    const std::string displayTitle = destination.level->levelTitle.empty()
        ? (hasCampaignInfo ? levelInfo.title : destination.level->levelName)
        : destination.level->levelTitle;
    const std::string description = hasCampaignInfo
        ? levelInfo.description
        : "A custom route prepared in the level editor.";
    const std::string farmingFocus = hasCampaignInfo
        ? levelInfo.farmingFocus
        : "Best for: testing layouts, encounters and pocket locations.";

    setDisplayingLevelNameString(displayTitle);

    std::string stateText;
    if (!destination.isOpened)
    {
        stateText = "Sealed route  •  the covenant still rejects this gate";
        destinationDescriptionText.setString(description + "\n" + player->getLevelUnlockHint(destination.level->levelName));
    }
    else if (destination.isPlayerThere && destination.isChoosed)
    {
        stateText = "Current ground  •  portal already attuned";
        destinationDescriptionText.setString(description + "\n" + farmingFocus);
    }
    else if (destination.isPlayerThere)
    {
        stateText = "Current ground";
        destinationDescriptionText.setString(description + "\n" + farmingFocus);
    }
    else if (destination.isChoosed)
    {
        stateText = "Portal attuned to this destination";
        destinationDescriptionText.setString(description + "\n" + farmingFocus);
    }
    else
    {
        stateText = "Unbound destination";
        destinationDescriptionText.setString(description + "\n" + farmingFocus);
    }

    destinationStateText.setString(stateText);
    legendText.setString("Ash mark: current ground   •   Ember mark: portal binding   •   Sealed cards: story-locked");
}

void ChooseDestinationMenu::drawLevelDestinationsText(sf::RenderWindow& window)
{
    window.draw(titleText);
    window.draw(subtitleText);
    window.draw(displayingLevelName);
    window.draw(destinationStateText);
    window.draw(destinationDescriptionText);
    window.draw(legendText);
    drawControlHints(window);
}

void ChooseDestinationMenu::drawControlHints(sf::RenderWindow& window)
{
    const std::vector<std::pair<std::string, std::string>> hints = {
        {scanToLabel(moveLeftKey) + " / " + scanToLabel(moveRightKey), "Cycle routes"},
        {scanToLabel(selectKey), "Attune portal"},
        {scanToLabel(closeKey) + " / Esc", "Close menu"}
    };

    float currentX = panelPosition.x + panelSize.x - 34.f;
    const float baseY = panelPosition.y + panelSize.y - 46.f;

    for (auto hintIt = hints.rbegin(); hintIt != hints.rend(); ++hintIt)
    {
        sf::Text actionText(*data->gameFont);
        configureText(actionText, 16, sf::Color(215, 202, 188, 235), 1.01f);
        actionText.setString(hintIt->second);
        const auto actionBounds = actionText.getLocalBounds();

        sf::Text keyText(*data->gameFont);
        configureText(keyText, 16, sf::Color(248, 230, 202, 255), 1.03f);
        keyText.setStyle(sf::Text::Bold);
        keyText.setString(hintIt->first);
        const auto keyBounds = keyText.getLocalBounds();

        const float chipWidth = 34.f + keyBounds.size.x + 18.f + actionBounds.size.x;
        currentX -= chipWidth;

        sf::RectangleShape chip({chipWidth, 24.f});
        chip.setPosition({currentX, baseY});
        chip.setFillColor(sf::Color(21, 15, 17, 218));
        chip.setOutlineThickness(1.f);
        chip.setOutlineColor(sf::Color(98, 65, 52, 215));
        window.draw(chip);

        sf::RectangleShape keyPlate({keyBounds.size.x + 16.f, 18.f});
        keyPlate.setPosition({currentX + 8.f, baseY + 3.f});
        keyPlate.setFillColor(sf::Color(71, 23, 19, 240));
        keyPlate.setOutlineThickness(1.f);
        keyPlate.setOutlineColor(sf::Color(171, 126, 95, 215));
        window.draw(keyPlate);

        keyText.setPosition({
            keyPlate.getPosition().x + keyPlate.getSize().x / 2.f,
            keyPlate.getPosition().y + 8.f
        });
        setTextOriginToMiddle(keyText);
        window.draw(keyText);

        actionText.setPosition({keyPlate.getPosition().x + keyPlate.getSize().x + 10.f, baseY + 3.f});
        window.draw(actionText);

        currentX -= 12.f;
    }
}
