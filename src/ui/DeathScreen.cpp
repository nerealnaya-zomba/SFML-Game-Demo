#include <DeathScreen.h>

#include <Localization.h>
#include <Mounting.h>

#include <algorithm>
#include <cmath>

namespace
{
constexpr float kTitleFadeDelay = 0.18f;
constexpr float kTitleFadeDuration = 0.82f;
constexpr float kOptionsFadeDelay = 0.92f;
constexpr float kOptionsFadeDuration = 0.34f;
constexpr float kHintFadeDelay = 1.18f;
constexpr float kHintFadeDuration = 0.22f;
constexpr float kInputUnlockDelay = 1.12f;

float clamp01(float value)
{
    return std::clamp(value, 0.f, 1.f);
}

float smoothstep(float value)
{
    const float clamped = clamp01(value);
    return clamped * clamped * (3.f - 2.f * clamped);
}

std::uint8_t toAlpha(float value)
{
    return static_cast<std::uint8_t>(std::clamp(value, 0.f, 255.f));
}

void styleOptionText(sf::Text& text)
{
    text.setCharacterSize(34);
    text.setOutlineThickness(2.f);
    text.setOutlineColor(sf::Color(6, 4, 4, 0));
}
}

DeathScreen::DeathScreen(sf::RenderWindow& window, sf::Font& font)
    : window_(&window)
    , titleText_(font)
    , descriptionText_(font)
    , restartOptionText_(font)
    , mainMenuOptionText_(font)
    , hintText_(font)
{
    Localization::setText(titleText_, Localization::isRussian() ? Localization::tr("death.title") : "YOU DIED");
    titleText_.setCharacterSize(132);
    titleText_.setOutlineThickness(4.f);
    titleText_.setOutlineColor(sf::Color(14, 0, 0, 0));

    Localization::setText(descriptionText_, Localization::isRussian() ? Localization::tr("death.description") : "Choose what comes next");
    descriptionText_.setCharacterSize(22);
    descriptionText_.setOutlineThickness(1.f);
    descriptionText_.setOutlineColor(sf::Color(6, 6, 6, 0));

    Localization::setText(restartOptionText_, Localization::isRussian() ? Localization::tr("death.restart") : "Restart Level");
    Localization::setText(mainMenuOptionText_, Localization::isRussian() ? Localization::tr("death.main_menu") : "Return To Main Menu");
    styleOptionText(restartOptionText_);
    styleOptionText(mainMenuOptionText_);

    Localization::setText(hintText_, Localization::isRussian() ? Localization::tr("death.hint") : "Arrows / W S - choose    Enter - confirm");
    hintText_.setCharacterSize(18);
    hintText_.setOutlineThickness(1.f);
    hintText_.setOutlineColor(sf::Color(0, 0, 0, 0));

    titleBand_.setFillColor(sf::Color(20, 4, 4, 0));
    dividerLine_.setFillColor(sf::Color(130, 30, 22, 0));
    optionHighlight_.setFillColor(sf::Color(105, 27, 18, 0));
    optionHighlight_.setOutlineThickness(1.5f);
    optionHighlight_.setOutlineColor(sf::Color(206, 149, 118, 0));

    refreshLayout();
    refreshVisuals(0.f);
}

void DeathScreen::open()
{
    isOpen_ = true;
    selectedOption_ = 0;
    Localization::setText(titleText_, Localization::isRussian() ? Localization::tr("death.title") : "YOU DIED");
    Localization::setText(descriptionText_, Localization::isRussian() ? Localization::tr("death.description") : "Choose what comes next");
    Localization::setText(restartOptionText_, Localization::isRussian() ? Localization::tr("death.restart") : "Restart Level");
    Localization::setText(mainMenuOptionText_, Localization::isRussian() ? Localization::tr("death.main_menu") : "Return To Main Menu");
    Localization::setText(hintText_, Localization::isRussian() ? Localization::tr("death.hint") : "Arrows / W S - choose    Enter - confirm");
    animationClock_.restart();
    refreshLayout();
    refreshVisuals(0.f);
}

void DeathScreen::close()
{
    isOpen_ = false;
}

void DeathScreen::update()
{
    if (!isOpen_)
    {
        return;
    }

    refreshLayout();
    refreshVisuals(animationClock_.getElapsedTime().asSeconds());
}

void DeathScreen::draw()
{
    if (!isOpen_ || !window_)
    {
        return;
    }

    const sf::View previousView = window_->getView();
    window_->setView(window_->getDefaultView());

    window_->draw(titleBand_);
    window_->draw(titleText_);
    window_->draw(descriptionText_);
    window_->draw(dividerLine_);
    window_->draw(optionHighlight_);
    window_->draw(restartOptionText_);
    window_->draw(mainMenuOptionText_);
    window_->draw(hintText_);

    window_->setView(previousView);
}

std::optional<DeathScreenAction> DeathScreen::handleEvent(const sf::Event& event)
{
    if (!isOpen_)
    {
        return std::nullopt;
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        const float elapsedSeconds = animationClock_.getElapsedTime().asSeconds();

        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
        {
            return DeathScreenAction::ReturnToMainMenu;
        }

        if (elapsedSeconds < kInputUnlockDelay)
        {
            return std::nullopt;
        }

        if (
            keyPressed->scancode == sf::Keyboard::Scancode::Up ||
            keyPressed->scancode == sf::Keyboard::Scancode::Left ||
            keyPressed->scancode == sf::Keyboard::Scancode::W ||
            keyPressed->scancode == sf::Keyboard::Scancode::A
        )
        {
            moveSelection(-1);
            return std::nullopt;
        }

        if (
            keyPressed->scancode == sf::Keyboard::Scancode::Down ||
            keyPressed->scancode == sf::Keyboard::Scancode::Right ||
            keyPressed->scancode == sf::Keyboard::Scancode::S ||
            keyPressed->scancode == sf::Keyboard::Scancode::D
        )
        {
            moveSelection(1);
            return std::nullopt;
        }

        if (
            keyPressed->scancode == sf::Keyboard::Scancode::Enter ||
            keyPressed->scancode == sf::Keyboard::Scancode::Space
        )
        {
            return selectedOption_ == 0
                ? std::optional<DeathScreenAction>(DeathScreenAction::RestartLevel)
                : std::optional<DeathScreenAction>(DeathScreenAction::ReturnToMainMenu);
        }
    }

    return std::nullopt;
}

bool DeathScreen::isOpen() const
{
    return isOpen_;
}

void DeathScreen::refreshLayout()
{
    if (!window_)
    {
        return;
    }

    const sf::View& defaultView = window_->getDefaultView();
    const sf::Vector2f viewCenter = defaultView.getCenter();
    const sf::Vector2f viewSize = defaultView.getSize();

    const sf::Vector2f titlePosition = {
        viewCenter.x,
        viewCenter.y - viewSize.y * 0.15f
    };
    const sf::Vector2f descriptionPosition = {
        viewCenter.x,
        titlePosition.y + 92.f
    };
    const sf::Vector2f restartPosition = {
        viewCenter.x,
        viewCenter.y + viewSize.y * 0.07f
    };
    const sf::Vector2f menuPosition = {
        viewCenter.x,
        restartPosition.y + 60.f
    };
    const sf::Vector2f hintPosition = {
        viewCenter.x,
        menuPosition.y + 110.f
    };

    setTextOriginToMiddle(titleText_);
    titleText_.setPosition(titlePosition);

    setTextOriginToMiddle(descriptionText_);
    descriptionText_.setPosition(descriptionPosition);

    setTextOriginToMiddle(restartOptionText_);
    restartOptionText_.setPosition(restartPosition);

    setTextOriginToMiddle(mainMenuOptionText_);
    mainMenuOptionText_.setPosition(menuPosition);

    setTextOriginToMiddle(hintText_);
    hintText_.setPosition(hintPosition);

    titleBand_.setSize({viewSize.x * 0.54f, 208.f});
    setRectangleOriginToMiddle(titleBand_);
    titleBand_.setPosition({titlePosition.x, titlePosition.y + 14.f});

    dividerLine_.setSize({viewSize.x * 0.24f, 2.f});
    setRectangleOriginToMiddle(dividerLine_);
    dividerLine_.setPosition({descriptionPosition.x, descriptionPosition.y + 32.f});

    const sf::Text& selectedText = (selectedOption_ == 0) ? restartOptionText_ : mainMenuOptionText_;
    const sf::FloatRect selectedBounds = selectedText.getGlobalBounds();
    const float highlightWidth = std::max(
        restartOptionText_.getGlobalBounds().size.x,
        mainMenuOptionText_.getGlobalBounds().size.x
    ) + 120.f;

    optionHighlight_.setSize({highlightWidth, selectedBounds.size.y + 26.f});
    setRectangleOriginToMiddle(optionHighlight_);
    optionHighlight_.setPosition({
        selectedText.getPosition().x,
        selectedText.getPosition().y + selectedBounds.size.y * 0.1f
    });
}

void DeathScreen::refreshVisuals(float elapsedSeconds)
{
    const float titleProgress = smoothstep((elapsedSeconds - kTitleFadeDelay) / kTitleFadeDuration);
    const float optionsProgress = smoothstep((elapsedSeconds - kOptionsFadeDelay) / kOptionsFadeDuration);
    const float hintProgress = smoothstep((elapsedSeconds - kHintFadeDelay) / kHintFadeDuration);
    const float pulse = 0.5f + 0.5f * std::sin(elapsedSeconds * 3.1f);

    const float titleScale = 1.08f - titleProgress * 0.08f;
    titleText_.setScale({titleScale * 1.08f, titleScale});
    titleText_.setFillColor(sf::Color(160, 28, 20, toAlpha(232.f * titleProgress)));
    titleText_.setOutlineColor(sf::Color(10, 0, 0, toAlpha(175.f * titleProgress)));

    descriptionText_.setFillColor(sf::Color(196, 186, 186, toAlpha(172.f * titleProgress)));
    descriptionText_.setOutlineColor(sf::Color(0, 0, 0, toAlpha(100.f * titleProgress)));

    titleBand_.setFillColor(sf::Color(16, 3, 3, toAlpha(138.f * titleProgress)));
    dividerLine_.setFillColor(sf::Color(128, 26, 18, toAlpha(200.f * titleProgress)));

    const bool restartSelected = selectedOption_ == 0;
    const sf::Color selectedFill(224, 204, 185, toAlpha(255.f * optionsProgress));
    const sf::Color inactiveFill(118, 105, 100, toAlpha(200.f * optionsProgress));
    const sf::Color selectedOutline(20, 9, 6, toAlpha(180.f * optionsProgress));
    const sf::Color inactiveOutline(5, 4, 4, toAlpha(130.f * optionsProgress));

    restartOptionText_.setFillColor(restartSelected ? selectedFill : inactiveFill);
    restartOptionText_.setOutlineColor(restartSelected ? selectedOutline : inactiveOutline);

    mainMenuOptionText_.setFillColor(!restartSelected ? selectedFill : inactiveFill);
    mainMenuOptionText_.setOutlineColor(!restartSelected ? selectedOutline : inactiveOutline);

    optionHighlight_.setFillColor(sf::Color(110, 28, 20, toAlpha((70.f + pulse * 34.f) * optionsProgress)));
    optionHighlight_.setOutlineColor(sf::Color(214, 164, 126, toAlpha((110.f + pulse * 40.f) * optionsProgress)));

    hintText_.setFillColor(sf::Color(130, 126, 122, toAlpha(160.f * hintProgress)));
    hintText_.setOutlineColor(sf::Color(0, 0, 0, toAlpha(95.f * hintProgress)));
}

void DeathScreen::moveSelection(int delta)
{
    selectedOption_ += delta;

    if (selectedOption_ < 0)
    {
        selectedOption_ = 1;
    }
    else if (selectedOption_ > 1)
    {
        selectedOption_ = 0;
    }

    refreshLayout();
    refreshVisuals(animationClock_.getElapsedTime().asSeconds());
}
