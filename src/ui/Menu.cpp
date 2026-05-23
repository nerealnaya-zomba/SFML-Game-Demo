#include <Menu.h>
#include <CampaignProgress.h>
#include <GameData.h>
#include <Localization.h>

#include <TGUI/Renderers/ListBoxRenderer.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string_view>
#include <utility>

namespace
{
struct MenuTheme
{
    sf::Color panelShadow;
    sf::Color panelFrame;
    sf::Color panelBorder;
    sf::Color panelInset;
    sf::Color panelInsetBorder;
    sf::Color titleBand;
    sf::Color titleText;
    sf::Color subtitleText;
    sf::Color footerText;
    sf::Color divider;
    sf::Color ornament;
    sf::Color sigil;
    sf::Color sigilSoft;

    sf::Color primaryButtonBackground;
    sf::Color primaryButtonHover;
    sf::Color primaryButtonDown;
    sf::Color primaryButtonBorder;

    sf::Color secondaryButtonBackground;
    sf::Color secondaryButtonHover;
    sf::Color secondaryButtonDown;
    sf::Color secondaryButtonBorder;

    sf::Color dangerButtonBackground;
    sf::Color dangerButtonHover;
    sf::Color dangerButtonDown;
    sf::Color dangerButtonBorder;

    sf::Color buttonText;
    sf::Color buttonTextHover;
    sf::Color buttonTextDisabled;

    sf::Color selectorBackground;
    sf::Color selectorBackgroundDisabled;
    sf::Color selectorBorder;
    sf::Color selectorText;
    sf::Color selectorTextDisabled;
    sf::Color selectorArrowBackground;
    sf::Color selectorArrowBackgroundHover;
    sf::Color selectorArrowColor;
    sf::Color listBackground;
    sf::Color listHoverBackground;
    sf::Color listSelectedBackground;
    sf::Color listSelectedHoverBackground;
    sf::Color listText;
    sf::Color listHoverText;
    sf::Color listSelectedText;

    sf::Color childBackground;
    sf::Color childBorder;
    sf::Color childTitleBar;
    sf::Color childTitleText;
    sf::Color infoCardFill;
    sf::Color infoCardBorder;
    sf::Color infoCardGlow;
};

constexpr float kMainPanelWidth = 720.f;
constexpr float kMainPanelHeight = 590.f;
constexpr float kPausePanelWidth = 620.f;
constexpr float kPausePanelHeight = 570.f;

std::string getLevelTitle(const std::string& levelName)
{
    const CampaignLevelInfo& levelInfo = CampaignProgress::getLevelInfo(levelName);
    return levelInfo.levelName == "unknown" ? levelName : levelInfo.title;
}

std::string getLevelTitle(const MenuState& state, const std::string& levelName)
{
    if (levelName.empty())
    {
        return Localization::isRussian() ? Localization::tr("menu.none") : "None";
    }

    const auto displayNameIt = state.levelDisplayNames.find(levelName);
    if (displayNameIt != state.levelDisplayNames.end() && !displayNameIt->second.empty())
    {
        return displayNameIt->second;
    }

    return getLevelTitle(levelName);
}

std::string getLevelFlavor(const std::string& levelName)
{
    const CampaignLevelInfo& levelInfo = CampaignProgress::getLevelInfo(levelName);
    return levelInfo.levelName == "unknown"
        ? (Localization::isRussian() ? "Для этих врат еще нет предзнаменования." : "No omen is written for this gate yet.")
        : levelInfo.flavor;
}

std::string getLevelFlavor(const MenuState& state, const std::string& levelName)
{
    const CampaignLevelInfo& levelInfo = CampaignProgress::getLevelInfo(levelName);
    if (levelInfo.levelName != "unknown")
    {
        return levelInfo.flavor;
    }

    const auto displayNameIt = state.levelDisplayNames.find(levelName);
    if (displayNameIt != state.levelDisplayNames.end() && !displayNameIt->second.empty())
    {
        return Localization::isRussian() ? "Пользовательский путь из редактора уровней." : "A custom route saved in the level editor.";
    }

    return Localization::isRussian() ? "Для этих врат еще нет предзнаменования." : "No omen is written for this gate yet.";
}

std::string getAshDensityLabel(int particleCount)
{
    if (particleCount <= 120)
    {
        return Localization::isRussian() ? Localization::tr("settings.low") : "Low";
    }

    if (particleCount >= 260)
    {
        return Localization::isRussian() ? Localization::tr("settings.high") : "High";
    }

    return Localization::isRussian() ? Localization::tr("settings.medium") : "Medium";
}

int normalizeAshDensityCount(int particleCount)
{
    if (particleCount <= 140)
    {
        return 120;
    }

    if (particleCount >= 230)
    {
        return 260;
    }

    return 200;
}

int getNextAshDensityCount(int particleCount)
{
    switch (normalizeAshDensityCount(particleCount))
    {
        case 120:
            return 200;
        case 200:
            return 260;
        default:
            return 120;
    }
}

std::string buildAshDensityText(int particleCount)
{
    const std::string prefix = Localization::isRussian()
        ? Localization::tr("settings.ash_density")
        : "Ash density";
    return prefix + ": " + getAshDensityLabel(particleCount) + " (" + std::to_string(particleCount) + ")";
}

sf::Color brighten(const sf::Color& color, int amount, std::uint8_t alpha = 255)
{
    return sf::Color(
        static_cast<std::uint8_t>(std::clamp(static_cast<int>(color.r) + amount, 0, 255)),
        static_cast<std::uint8_t>(std::clamp(static_cast<int>(color.g) + amount, 0, 255)),
        static_cast<std::uint8_t>(std::clamp(static_cast<int>(color.b) + amount, 0, 255)),
        alpha
    );
}

MenuTheme getMenuTheme(MenuMode mode)
{
    MenuTheme theme{};

    if (mode == MenuMode::Pause)
    {
        theme.panelShadow = sf::Color(0, 0, 0, 170);
        theme.panelFrame = sf::Color(12, 15, 21, 222);
        theme.panelBorder = sf::Color(121, 136, 152, 210);
        theme.panelInset = sf::Color(8, 11, 15, 200);
        theme.panelInsetBorder = sf::Color(49, 61, 74, 220);
        theme.titleBand = sf::Color(18, 26, 34, 220);
        theme.titleText = sf::Color(224, 215, 204, 255);
        theme.subtitleText = sf::Color(171, 180, 192, 245);
        theme.footerText = sf::Color(122, 131, 145, 220);
        theme.divider = sf::Color(109, 92, 84, 160);
        theme.ornament = sf::Color(86, 97, 110, 118);
        theme.sigil = sf::Color(102, 118, 133, 60);
        theme.sigilSoft = sf::Color(189, 202, 215, 26);

        theme.primaryButtonBackground = sf::Color(42, 32, 31, 235);
        theme.primaryButtonHover = sf::Color(72, 45, 40, 245);
        theme.primaryButtonDown = sf::Color(94, 58, 48, 250);
        theme.primaryButtonBorder = sf::Color(170, 126, 96, 255);

        theme.secondaryButtonBackground = sf::Color(19, 24, 31, 235);
        theme.secondaryButtonHover = sf::Color(30, 38, 46, 245);
        theme.secondaryButtonDown = sf::Color(40, 49, 57, 250);
        theme.secondaryButtonBorder = sf::Color(92, 112, 128, 255);

        theme.dangerButtonBackground = sf::Color(45, 15, 18, 235);
        theme.dangerButtonHover = sf::Color(73, 22, 27, 245);
        theme.dangerButtonDown = sf::Color(96, 28, 35, 250);
        theme.dangerButtonBorder = sf::Color(161, 79, 80, 255);

        theme.buttonText = sf::Color(228, 220, 210, 255);
        theme.buttonTextHover = sf::Color(255, 244, 230, 255);
        theme.buttonTextDisabled = sf::Color(112, 114, 118, 255);

        theme.selectorBackground = sf::Color(16, 20, 27, 238);
        theme.selectorBackgroundDisabled = sf::Color(13, 15, 20, 220);
        theme.selectorBorder = sf::Color(94, 112, 125, 255);
        theme.selectorText = sf::Color(221, 214, 206, 255);
        theme.selectorTextDisabled = sf::Color(110, 115, 120, 255);
        theme.selectorArrowBackground = sf::Color(28, 36, 43, 255);
        theme.selectorArrowBackgroundHover = sf::Color(42, 52, 61, 255);
        theme.selectorArrowColor = sf::Color(214, 199, 177, 255);
        theme.listBackground = sf::Color(12, 17, 22, 245);
        theme.listHoverBackground = sf::Color(28, 37, 44, 245);
        theme.listSelectedBackground = sf::Color(63, 44, 38, 245);
        theme.listSelectedHoverBackground = sf::Color(88, 60, 50, 245);
        theme.listText = sf::Color(214, 206, 196, 255);
        theme.listHoverText = sf::Color(246, 239, 228, 255);
        theme.listSelectedText = sf::Color(255, 236, 216, 255);

        theme.childBackground = sf::Color(9, 12, 17, 242);
        theme.childBorder = sf::Color(100, 115, 129, 255);
        theme.childTitleBar = sf::Color(21, 29, 37, 250);
        theme.childTitleText = sf::Color(228, 218, 205, 255);
        theme.infoCardFill = sf::Color(14, 19, 26, 214);
        theme.infoCardBorder = sf::Color(78, 96, 114, 210);
        theme.infoCardGlow = sf::Color(120, 144, 168, 54);
        return theme;
    }

    theme.panelShadow = sf::Color(0, 0, 0, 185);
    theme.panelFrame = sf::Color(20, 9, 10, 220);
    theme.panelBorder = sf::Color(162, 118, 83, 220);
    theme.panelInset = sf::Color(10, 6, 8, 198);
    theme.panelInsetBorder = sf::Color(80, 39, 32, 220);
    theme.titleBand = sf::Color(52, 10, 12, 220);
    theme.titleText = sf::Color(182, 42, 31, 255);
    theme.subtitleText = sf::Color(216, 192, 181, 245);
    theme.footerText = sf::Color(136, 118, 112, 220);
    theme.divider = sf::Color(150, 72, 53, 175);
    theme.ornament = sf::Color(130, 77, 57, 120);
    theme.sigil = sf::Color(134, 48, 34, 72);
    theme.sigilSoft = sf::Color(229, 186, 136, 30);

    theme.primaryButtonBackground = sf::Color(70, 18, 17, 238);
    theme.primaryButtonHover = sf::Color(101, 29, 24, 246);
    theme.primaryButtonDown = sf::Color(127, 39, 29, 250);
    theme.primaryButtonBorder = sf::Color(194, 147, 101, 255);

    theme.secondaryButtonBackground = sf::Color(28, 18, 19, 235);
    theme.secondaryButtonHover = sf::Color(46, 25, 24, 244);
    theme.secondaryButtonDown = sf::Color(60, 31, 28, 250);
    theme.secondaryButtonBorder = sf::Color(124, 84, 66, 255);

    theme.dangerButtonBackground = sf::Color(56, 12, 15, 236);
    theme.dangerButtonHover = sf::Color(92, 20, 24, 246);
    theme.dangerButtonDown = sf::Color(118, 28, 31, 250);
    theme.dangerButtonBorder = sf::Color(188, 82, 76, 255);

    theme.buttonText = sf::Color(238, 225, 205, 255);
    theme.buttonTextHover = sf::Color(255, 243, 224, 255);
    theme.buttonTextDisabled = sf::Color(116, 103, 98, 255);

    theme.selectorBackground = sf::Color(16, 11, 13, 240);
    theme.selectorBackgroundDisabled = sf::Color(12, 10, 11, 222);
    theme.selectorBorder = sf::Color(153, 111, 82, 255);
    theme.selectorText = sf::Color(236, 223, 205, 255);
    theme.selectorTextDisabled = sf::Color(119, 107, 103, 255);
    theme.selectorArrowBackground = sf::Color(43, 17, 18, 255);
    theme.selectorArrowBackgroundHover = sf::Color(72, 28, 25, 255);
    theme.selectorArrowColor = sf::Color(240, 214, 174, 255);
    theme.listBackground = sf::Color(12, 8, 10, 247);
    theme.listHoverBackground = sf::Color(46, 20, 21, 247);
    theme.listSelectedBackground = sf::Color(85, 31, 24, 247);
    theme.listSelectedHoverBackground = sf::Color(111, 39, 28, 247);
    theme.listText = sf::Color(230, 219, 206, 255);
    theme.listHoverText = sf::Color(255, 245, 233, 255);
    theme.listSelectedText = sf::Color(255, 238, 215, 255);

    theme.childBackground = sf::Color(8, 6, 8, 244);
    theme.childBorder = sf::Color(136, 97, 72, 255);
    theme.childTitleBar = sf::Color(48, 12, 14, 252);
    theme.childTitleText = sf::Color(240, 226, 206, 255);
    theme.infoCardFill = sf::Color(18, 10, 11, 214);
    theme.infoCardBorder = sf::Color(150, 105, 77, 214);
    theme.infoCardGlow = sf::Color(206, 114, 78, 58);
    return theme;
}

float getPanelWidth(MenuMode mode)
{
    return (mode == MenuMode::Main) ? kMainPanelWidth : kPausePanelWidth;
}

float getPanelHeight(MenuMode mode)
{
    return (mode == MenuMode::Main) ? kMainPanelHeight : kPausePanelHeight;
}
}

Menu::Menu(sf::RenderWindow& window, GameData& gameData)
    : window_m(&window)
    , gameData_m(&gameData)
    , background(static_cast<int>(window.getSize().x), static_cast<int>(window.getSize().y))
{
    syncPreferencesFromGameData();

    gui.setWindow(window);
    setupMainWidgets();

    const sf::Vector2u windowSize = window.getSize();
    exitDialogue = std::make_unique<AskDialogue>(
        sf::Vector2f(windowSize.x / 2.f, windowSize.y / 2.f),
        sf::Vector2f(350.f, 150.f),
        Localization::isRussian() ? Localization::tr("dialog.quit") : "Quit the game?",
        *window_m
    );
    exitDialogue->setOnYesClick([this]() { callbacks_.onExitGame(); });
    exitDialogue->setOnNoClick([this]() { exitDialogue->close(); });

    resetProgressDialogue = std::make_unique<AskDialogue>(
        sf::Vector2f(windowSize.x / 2.f, windowSize.y / 2.f),
        sf::Vector2f(460.f, 180.f),
        Localization::isRussian()
            ? Localization::tr("dialog.reset_progress")
            : "Reset all saved progress?\nGold, relics, weapons and unlocked gates will be lost.",
        *window_m
    );
    resetProgressDialogue->setOnYesClick([this]() {
        if (callbacks_.onResetProgress())
        {
            closePopups();
            openMainMenu();
            return;
        }

        resetProgressDialogue->close();
        syncPopupInteractivity();
    });
    resetProgressDialogue->setOnNoClick([this]() {
        resetProgressDialogue->close();
        syncPopupInteractivity();
    });

    openMainMenu();
}

Menu::~Menu() = default;

void Menu::syncPreferencesFromGameData()
{
    if (!gameData_m)
    {
        return;
    }

    fullscreenEnabled = gameData_m->isFullscreenEnabled();
    vsyncEnabled = gameData_m->isVsyncEnabled();
    menuParticleCount = normalizeAshDensityCount(gameData_m->getMenuParticleCount());
    background.setParticleCount(menuParticleCount);
    window_m->setVerticalSyncEnabled(vsyncEnabled);
    window_m->setFramerateLimit(vsyncEnabled ? 0u : WINDOW_FPS);
}

void Menu::attachWindow(sf::RenderWindow& window)
{
    window_m = &window;
    gui.setWindow(window);
    if (exitDialogue)
    {
        exitDialogue->attachWindow(window);
    }
    if (resetProgressDialogue)
    {
        resetProgressDialogue->attachWindow(window);
    }
    applyModeLayout();
}

void Menu::styleButton(const tgui::Button::Ptr& button, ButtonStyleRole role) const
{
    const MenuTheme theme = getMenuTheme(mode_);
    const sf::Color* idle = &theme.secondaryButtonBackground;
    const sf::Color* hover = &theme.secondaryButtonHover;
    const sf::Color* down = &theme.secondaryButtonDown;
    const sf::Color* border = &theme.secondaryButtonBorder;

    if (role == ButtonStyleRole::Primary)
    {
        idle = &theme.primaryButtonBackground;
        hover = &theme.primaryButtonHover;
        down = &theme.primaryButtonDown;
        border = &theme.primaryButtonBorder;
    }
    else if (role == ButtonStyleRole::Danger)
    {
        idle = &theme.dangerButtonBackground;
        hover = &theme.dangerButtonHover;
        down = &theme.dangerButtonDown;
        border = &theme.dangerButtonBorder;
    }

    auto* renderer = button->getRenderer();
    renderer->setBorders({2.f});
    renderer->setRoundedBorderRadius(8.f);
    renderer->setBackgroundColor(*idle);
    renderer->setBackgroundColorHover(*hover);
    renderer->setBackgroundColorDown(*down);
    renderer->setBackgroundColorDownHover(*down);
    renderer->setBackgroundColorDisabled(sf::Color(idle->r / 2, idle->g / 2, idle->b / 2, 200));
    renderer->setBorderColor(*border);
    renderer->setBorderColorHover(sf::Color(
        std::min(border->r + 26, 255),
        std::min(border->g + 26, 255),
        std::min(border->b + 26, 255),
        border->a
    ));
    renderer->setBorderColorDown(*border);
    renderer->setBorderColorDownHover(*border);
    renderer->setBorderColorDisabled(sf::Color(border->r / 2, border->g / 2, border->b / 2, 180));
    renderer->setTextColor(theme.buttonText);
    renderer->setTextColorHover(theme.buttonTextHover);
    renderer->setTextColorDown(theme.buttonTextHover);
    renderer->setTextColorDownHover(theme.buttonTextHover);
    renderer->setTextColorDisabled(theme.buttonTextDisabled);
}

void Menu::styleLabel(const tgui::Label::Ptr& label, bool isTitle) const
{
    const MenuTheme theme = getMenuTheme(mode_);
    auto* renderer = label->getRenderer();
    renderer->setBackgroundColor(tgui::Color::Transparent);
    renderer->setTextColor(isTitle ? theme.titleText : theme.subtitleText);
    renderer->setTextOutlineColor(isTitle ? sf::Color(14, 2, 2, 190) : sf::Color(0, 0, 0, 150));
    renderer->setTextOutlineThickness(isTitle ? 2.f : 1.f);
    label->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
    label->setVerticalAlignment(tgui::VerticalAlignment::Center);
}

void Menu::styleInfoLabel(const tgui::Label::Ptr& label) const
{
    const MenuTheme theme = getMenuTheme(mode_);
    auto* renderer = label->getRenderer();
    renderer->setBackgroundColor(tgui::Color::Transparent);
    renderer->setTextColor(theme.subtitleText);
    renderer->setTextOutlineColor(sf::Color(0, 0, 0, 150));
    renderer->setTextOutlineThickness(1.f);
    label->setHorizontalAlignment(tgui::HorizontalAlignment::Left);
    label->setVerticalAlignment(tgui::VerticalAlignment::Top);
}

void Menu::styleSelector() const
{
    const MenuTheme theme = getMenuTheme(mode_);
    auto* renderer = levelSelector->getRenderer();
    renderer->setBorders({2.f});
    renderer->setPadding({10.f, 7.f, 10.f, 7.f});
    renderer->setRoundedBorderRadius(8.f);
    renderer->setBackgroundColor(theme.selectorBackground);
    renderer->setBackgroundColorDisabled(theme.selectorBackgroundDisabled);
    renderer->setBorderColor(theme.selectorBorder);
    renderer->setTextColor(theme.selectorText);
    renderer->setTextColorDisabled(theme.selectorTextDisabled);
    renderer->setArrowBackgroundColor(theme.selectorArrowBackground);
    renderer->setArrowBackgroundColorHover(theme.selectorArrowBackgroundHover);
    renderer->setArrowBackgroundColorDisabled(theme.selectorBackgroundDisabled);
    renderer->setArrowColor(theme.selectorArrowColor);
    renderer->setDefaultTextColor(theme.footerText);

    tgui::ListBoxRenderer listRenderer(renderer->getListBox());
    listRenderer.setBorders({2.f});
    listRenderer.setPadding({8.f, 5.f, 8.f, 5.f});
    listRenderer.setBackgroundColor(theme.listBackground);
    listRenderer.setBackgroundColorHover(theme.listHoverBackground);
    listRenderer.setSelectedBackgroundColor(theme.listSelectedBackground);
    listRenderer.setSelectedBackgroundColorHover(theme.listSelectedHoverBackground);
    listRenderer.setTextColor(theme.listText);
    listRenderer.setTextColorHover(theme.listHoverText);
    listRenderer.setSelectedTextColor(theme.listSelectedText);
    listRenderer.setSelectedTextColorHover(theme.listSelectedText);
    listRenderer.setBorderColor(theme.selectorBorder);
}

void Menu::styleChildWindow(const tgui::ChildWindow::Ptr& childWindow) const
{
    const MenuTheme theme = getMenuTheme(mode_);
    auto* renderer = childWindow->getRenderer();
    renderer->setBorders({2.f});
    renderer->setClientPadding({16.f, 14.f, 16.f, 16.f});
    renderer->setTitleBarHeight(34.f);
    renderer->setDistanceToSide(12.f);
    renderer->setBorderBelowTitleBar(2.f);
    renderer->setBackgroundColor(theme.childBackground);
    renderer->setBorderColor(theme.childBorder);
    renderer->setBorderColorFocused(theme.panelBorder);
    renderer->setTitleBarColor(theme.childTitleBar);
    renderer->setTitleColor(theme.childTitleText);
}

tgui::Button::Ptr Menu::createMenuButton(const tgui::String& text, float y)
{
    auto button = tgui::Button::create();
    button->setSize(360.f, 56.f);
    button->setPosition({
        (window_m->getSize().x / 2.f) - 180.f,
        y
    });
    button->setText(text);
    button->setTextSize(BASE_MENU_BUTTONS_CHARACTER_SIZE);
    gui.add(button);
    return button;
}

void Menu::setupMainWidgets()
{
    titleLabel = tgui::Label::create("DARK GATE");
    titleLabel->setTextSize(52);
    gui.add(titleLabel);

    subtitleLabel = tgui::Label::create("Enter the first gate");
    subtitleLabel->setTextSize(19);
    gui.add(subtitleLabel);

    footerLabel = tgui::Label::create("Esc closes the pause veil");
    footerLabel->setTextSize(17);
    gui.add(footerLabel);

    selectionInfoLabel = tgui::Label::create();
    selectionInfoLabel->setTextSize(17);
    gui.add(selectionInfoLabel);

    systemInfoLabel = tgui::Label::create();
    systemInfoLabel->setTextSize(17);
    gui.add(systemInfoLabel);

    shortcutInfoLabel = tgui::Label::create();
    shortcutInfoLabel->setTextSize(16);
    gui.add(shortcutInfoLabel);

    initializeLevelSelector();

    levelPrevButton = tgui::Button::create("<");
    levelPrevButton->setTextSize(22);
    levelPrevButton->onClick([this]() { selectAdjacentLevel(-1); });
    gui.add(levelPrevButton);

    levelNextButton = tgui::Button::create(">");
    levelNextButton->setTextSize(22);
    levelNextButton->onClick([this]() { selectAdjacentLevel(1); });
    gui.add(levelNextButton);

    randomLevelButton = tgui::Button::create("Hidden");
    randomLevelButton->setVisible(false);
    gui.add(randomLevelButton);

    continueButton = createMenuButton(BASE_PLAY_BUTTON_TEXT, 0.f);
    continueButton->onClick([this]() { resumeButtonOnClick(); });

    startLevelButton = createMenuButton(BASE_START_BUTTON_TEXT, 0.f);
    startLevelButton->onClick([this]() { startSelectedLevelOnClick(); });

    restartLevelButton = createMenuButton(BASE_RESTART_BUTTON_TEXT, 0.f);
    restartLevelButton->onClick([this]() { restartCurrentLevelOnClick(); });

    mainMenuButton = createMenuButton(BASE_RETURN_TO_MAIN_MENU_BUTTON_TEXT, 0.f);
    mainMenuButton->onClick([this]() { returnToMainMenuOnClick(); });

    settingsButton = createMenuButton(BASE_SETTINGS_BUTTON_TEXT, 0.f);
    settingsButton->onClick([this]() { settingsButtonOnClick(); });

    controlsButton = createMenuButton(BASE_CONTROLS_BUTTON_TEXT, 0.f);
    controlsButton->onClick([this]() { controlsButtonOnClick(); });

    exitButton = createMenuButton(BASE_EXIT_BUTTON_TEXT, 0.f);
    exitButton->onClick([this]() { exitButtonOnClick(); });

    initializeSettingsWindow();
    initializeControlsWindow();
    applyModeLayout();
    refreshMenuContext();
}

void Menu::initializeLevelSelector()
{
    levelSelector = tgui::ComboBox::create();
    levelSelector->setSize({400.f, 42.f});
    levelSelector->setTextSize(20);
    levelSelector->setDefaultText("Level");
    levelSelector->onItemSelect([this](const tgui::String& item) {
        const std::string label = item.toStdString();
        const auto selectedLevelIt = selectorLabelToLevelId_.find(label);
        if (selectedLevelIt != selectorLabelToLevelId_.end())
        {
            state_.selectedLevelName = selectedLevelIt->second;
        }
        refreshMenuContext();
    });
    gui.add(levelSelector);
}

void Menu::initializeSettingsWindow()
{
    settingsWindow = tgui::ChildWindow::create();
    settingsWindow->setTitle(Localization::isRussian() ? Localization::tr("settings.title") : "Settings");
    settingsWindow->setClientSize({420.f, 400.f});
    settingsWindow->setPosition({
        window_m->getSize().x / 2.f - 210.f,
        window_m->getSize().y / 2.f - 210.f
    });
    settingsWindow->setResizable(false);
    settingsWindow->setTitleButtons(tgui::ChildWindow::TitleButton::None);
    settingsWindow->setVisible(false);

    auto settingsLabel = tgui::Label::create(Localization::isRussian()
        ? Localization::tr("settings.description")
        : "Settings for the menu, language and video.");
    settingsLabel->setWidgetName("settingsLabel");
    settingsLabel->setTextSize(18);
    settingsLabel->setPosition({0.f, 6.f});
    settingsLabel->setSize({388.f, 44.f});
    settingsLabel->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
    settingsWindow->add(settingsLabel);

    fullscreenToggleButton = tgui::Button::create();
    fullscreenToggleButton->setSize({360.f, 42.f});
    fullscreenToggleButton->setPosition({14.f, 70.f});
    fullscreenToggleButton->setTextSize(22);
    fullscreenToggleButton->onClick([this]() { toggleFullscreen(); });
    settingsWindow->add(fullscreenToggleButton);

    vsyncToggleButton = tgui::Button::create();
    vsyncToggleButton->setSize({360.f, 42.f});
    vsyncToggleButton->setPosition({14.f, 120.f});
    vsyncToggleButton->setTextSize(22);
    vsyncToggleButton->onClick([this]() { toggleVsync(); });
    settingsWindow->add(vsyncToggleButton);

    particleToggleButton = tgui::Button::create();
    particleToggleButton->setSize({360.f, 42.f});
    particleToggleButton->setPosition({14.f, 170.f});
    particleToggleButton->setTextSize(22);
    particleToggleButton->onClick([this]() { toggleMenuParticles(); });
    settingsWindow->add(particleToggleButton);

    languageToggleButton = tgui::Button::create();
    languageToggleButton->setSize({360.f, 42.f});
    languageToggleButton->setPosition({14.f, 220.f});
    languageToggleButton->setTextSize(22);
    languageToggleButton->onClick([this]() { toggleLanguage(); });
    settingsWindow->add(languageToggleButton);

    resetProgressButton = tgui::Button::create();
    resetProgressButton->setSize({360.f, 42.f});
    resetProgressButton->setPosition({14.f, 270.f});
    resetProgressButton->setText(Localization::isRussian() ? Localization::tr("settings.reset_progress") : "Reset saved progress");
    resetProgressButton->setTextSize(22);
    resetProgressButton->onClick([this]() { resetProgressButtonOnClick(); });
    settingsWindow->add(resetProgressButton);

    auto closeButton = tgui::Button::create();
    closeButton->setWidgetName("settingsCloseButton");
    closeButton->setSize({360.f, 42.f});
    closeButton->setPosition({14.f, 320.f});
    closeButton->setText(Localization::isRussian() ? Localization::tr("settings.close") : "Close");
    closeButton->setTextSize(22);
    closeButton->onClick([this]() {
        settingsWindow->setVisible(false);
        syncPopupInteractivity();
    });
    settingsWindow->add(closeButton);

    gui.add(settingsWindow);
}

void Menu::initializeControlsWindow()
{
    controlsWindow = tgui::ChildWindow::create();
    controlsWindow->setTitle(Localization::isRussian() ? Localization::tr("controls.title") : "Controls");
    controlsWindow->setClientSize({470.f, 388.f});
    controlsWindow->setPosition({
        window_m->getSize().x / 2.f - 235.f,
        window_m->getSize().y / 2.f - 205.f
    });
    controlsWindow->setResizable(false);
    controlsWindow->setTitleButtons(tgui::ChildWindow::TitleButton::None);
    controlsWindow->setVisible(false);

    auto controlsLabel = tgui::Label::create(
        Localization::isRussian() ? Localization::tr("controls.body") :
        "Movement\n"
        "Left / Right - Move\n"
        "Z - Jump\n"
        "C - Dash\n\n"
        "Combat\n"
        "X - Shoot\n"
        "R - Open portal\n"
        "E - Destination menu\n"
        "Q / W - Change destination\n"
        "Enter - Confirm\n\n"
        "Menus\n"
        "Escape - Pause\n"
        "F1 - Ritual Console"
    );
    controlsLabel->setWidgetName("controlsLabel");
    controlsLabel->setTextSize(19);
    controlsLabel->setPosition({10.f, 10.f});
    controlsLabel->setSize({420.f, 250.f});
    controlsWindow->add(controlsLabel);

    auto closeButton = tgui::Button::create();
    closeButton->setWidgetName("controlsCloseButton");
    closeButton->setSize({404.f, 42.f});
    closeButton->setPosition({18.f, 302.f});
    closeButton->setText(Localization::isRussian() ? Localization::tr("settings.close") : "Close");
    closeButton->setTextSize(22);
    closeButton->onClick([this]() {
        controlsWindow->setVisible(false);
        syncPopupInteractivity();
    });
    controlsWindow->add(closeButton);

    gui.add(controlsWindow);
}

void Menu::connectTGUIFont(tgui::Font& font)
{
    gui.setFont(font);
    exitDialogue->connectTGUIFont(font);
    resetProgressDialogue->connectTGUIFont(font);
}

void Menu::setCallbacks(MenuCallbacks callbacks)
{
    callbacks_ = std::move(callbacks);
}

void Menu::setState(const MenuState& state)
{
    state_ = state;
    refreshLevelSelector();
    refreshMenuContext();
}

void Menu::setAvailableLevels(const std::vector<std::string>& levels)
{
    state_.availableLevels = levels;
    refreshLevelSelector();
    refreshMenuContext();
}

void Menu::setCurrentLevelName(const std::string& levelName)
{
    state_.currentLevelName = levelName;
    refreshMenuContext();
}

void Menu::setSelectedLevelName(const std::string& levelName)
{
    state_.selectedLevelName = levelName;
    refreshLevelSelector();
    refreshMenuContext();
}

void Menu::setCanContinue(bool enabled)
{
    state_.canContinue = enabled;
    refreshMenuContext();
}

void Menu::setCanRestartLevel(bool enabled)
{
    state_.canRestartLevel = enabled;
    refreshMenuContext();
}

std::string Menu::buildLevelSelectorLabel(const std::string& levelName) const
{
    const auto displayNameIt = state_.levelDisplayNames.find(levelName);
    if (displayNameIt == state_.levelDisplayNames.end() || displayNameIt->second.empty())
    {
        return getLevelTitle(levelName);
    }

    return displayNameIt->second;
}

void Menu::refreshLevelSelector()
{
    levelSelector->removeAllItems();
    selectorLabelToLevelId_.clear();
    levelIdToSelectorLabel_.clear();

    for (const auto& levelName : state_.availableLevels)
    {
        const std::string selectorLabel = buildLevelSelectorLabel(levelName);
        selectorLabelToLevelId_[selectorLabel] = levelName;
        levelIdToSelectorLabel_[levelName] = selectorLabel;
        levelSelector->addItem(selectorLabel);
    }

    if (state_.availableLevels.empty())
    {
        state_.selectedLevelName.clear();
        const std::string noLevelsText = Localization::isRussian()
            ? Localization::tr("menu.no_levels_available")
            : "No levels available";
        levelSelector->addItem(noLevelsText);
        levelSelector->setSelectedItem(noLevelsText);
        levelSelector->setEnabled(false);
        return;
    }

    levelSelector->setEnabled(true);

    if (state_.selectedLevelName.empty())
    {
        state_.selectedLevelName = !state_.currentLevelName.empty()
            ? state_.currentLevelName
            : state_.availableLevels.front();
    }

    const auto selectedLabelIt = levelIdToSelectorLabel_.find(state_.selectedLevelName);
    if (selectedLabelIt == levelIdToSelectorLabel_.end() || !levelSelector->setSelectedItem(selectedLabelIt->second))
    {
        state_.selectedLevelName = state_.availableLevels.front();
        const auto fallbackLabelIt = levelIdToSelectorLabel_.find(state_.selectedLevelName);
        if (fallbackLabelIt != levelIdToSelectorLabel_.end())
        {
            levelSelector->setSelectedItem(fallbackLabelIt->second);
        }
    }
}

void Menu::selectAdjacentLevel(int direction)
{
    if (state_.availableLevels.empty() || direction == 0)
    {
        return;
    }

    auto currentIt = std::find(state_.availableLevels.begin(), state_.availableLevels.end(), state_.selectedLevelName);
    std::size_t currentIndex = 0u;
    if (currentIt != state_.availableLevels.end())
    {
        currentIndex = static_cast<std::size_t>(std::distance(state_.availableLevels.begin(), currentIt));
    }

    const std::size_t count = state_.availableLevels.size();
    const int normalizedDirection = direction > 0 ? 1 : -1;
    const std::size_t nextIndex = static_cast<std::size_t>(
        (static_cast<int>(currentIndex) + normalizedDirection + static_cast<int>(count)) % static_cast<int>(count)
    );

    state_.selectedLevelName = state_.availableLevels[nextIndex];
    if (levelSelector && levelSelector->isVisible())
    {
        const auto selectedLabelIt = levelIdToSelectorLabel_.find(state_.selectedLevelName);
        if (selectedLabelIt != levelIdToSelectorLabel_.end())
        {
            levelSelector->setSelectedItem(selectedLabelIt->second);
        }
    }
    refreshMenuContext();
}

void Menu::selectRandomLevel()
{
}

std::string Menu::buildSelectionInfoText() const
{
    if (mode_ == MenuMode::Pause)
    {
        return (Localization::isRussian() ? Localization::tr("menu.current_level") : std::string("Current Level")) +
            "\n" + getLevelTitle(state_, state_.currentLevelName);
    }

    return (Localization::isRussian() ? Localization::tr("menu.starting_level") : std::string("Starting Level")) +
        "\n" + getLevelTitle(state_, "level1.json");
}

std::string Menu::buildSystemInfoText() const
{
    return "";
}

std::string Menu::buildShortcutInfoText() const
{
    return "";
}

void Menu::refreshMenuContext()
{
    if (fullscreenToggleButton)
    {
        fullscreenToggleButton->setText(fullscreenEnabled
            ? (Localization::isRussian() ? Localization::tr("settings.fullscreen_on") : "Fullscreen: ON")
            : (Localization::isRussian() ? Localization::tr("settings.fullscreen_off") : "Fullscreen: OFF"));
    }

    if (vsyncToggleButton)
    {
        vsyncToggleButton->setText(vsyncEnabled
            ? (Localization::isRussian() ? Localization::tr("settings.vsync_on") : "VSync: ON")
            : (Localization::isRussian() ? Localization::tr("settings.vsync_off") : "VSync: OFF"));
    }

    if (particleToggleButton)
    {
        particleToggleButton->setText(buildAshDensityText(menuParticleCount));
    }

    if (languageToggleButton)
    {
        const std::string prefix = Localization::isRussian() ? Localization::tr("settings.language") : "Language";
        languageToggleButton->setText(prefix + ": " + Localization::languageDisplayName(Localization::getLanguage()));
    }

    if (settingsWindow)
    {
        settingsWindow->setTitle(Localization::isRussian() ? Localization::tr("settings.title") : "Settings");
        if (auto settingsLabel = settingsWindow->get<tgui::Label>("settingsLabel"))
        {
            settingsLabel->setText(Localization::isRussian()
                ? Localization::tr("settings.description")
                : "Settings for the menu, language and video.");
        }
        if (resetProgressButton)
        {
            resetProgressButton->setText(Localization::isRussian() ? Localization::tr("settings.reset_progress") : "Reset saved progress");
        }
        if (auto closeButton = settingsWindow->get<tgui::Button>("settingsCloseButton"))
        {
            closeButton->setText(Localization::isRussian() ? Localization::tr("settings.close") : "Close");
        }
    }

    if (controlsWindow)
    {
        controlsWindow->setTitle(Localization::isRussian() ? Localization::tr("controls.title") : "Controls");
        if (auto controlsLabel = controlsWindow->get<tgui::Label>("controlsLabel"))
        {
            controlsLabel->setText(Localization::isRussian()
                ? Localization::tr("controls.body")
                : "Movement\n"
                  "Left / Right - Move\n"
                  "Z - Jump\n"
                  "C - Dash\n\n"
                  "Combat\n"
                  "X - Shoot\n"
                  "R - Open portal\n"
                  "E - Destination menu\n"
                  "Q / W - Change destination\n"
                  "Enter - Confirm\n\n"
                  "Menus\n"
                  "Escape - Pause\n"
                  "F1 - Ritual Console");
        }
        if (auto closeButton = controlsWindow->get<tgui::Button>("controlsCloseButton"))
        {
            closeButton->setText(Localization::isRussian() ? Localization::tr("settings.close") : "Close");
        }
    }

    if (mode_ == MenuMode::Pause)
    {
        titleLabel->setText(Localization::isRussian() ? Localization::tr("menu.paused") : "PAUSED");
        subtitleLabel->setText(state_.currentLevelName.empty()
            ? (Localization::isRussian() ? Localization::tr("menu.game_paused") : "Game paused")
            : getLevelTitle(state_, state_.currentLevelName));
        footerLabel->setText("");
    }
    else
    {
        titleLabel->setText("DARK GATE");

        if (state_.availableLevels.empty())
        {
            subtitleLabel->setText(Localization::isRussian() ? Localization::tr("menu.no_levels") : "No playable levels found");
        }
        else
        {
            subtitleLabel->setText(Localization::isRussian() ? Localization::tr("menu.enter_first_gate") : "Enter the first gate");
        }

        footerLabel->setText("");
    }

    continueButton->setText(mode_ == MenuMode::Pause
        ? (Localization::isRussian() ? Localization::tr("menu.resume") : std::string("Resume"))
        : (Localization::isRussian() ? Localization::tr("menu.start") : std::string("Start")));
    mainMenuButton->setText(Localization::isRussian() ? Localization::tr("menu.main_menu") : std::string("Main Menu"));
    settingsButton->setText(Localization::isRussian() ? Localization::tr("menu.settings") : std::string("Settings"));
    controlsButton->setText(Localization::isRussian() ? Localization::tr("menu.controls") : std::string("Controls"));
    exitButton->setText(Localization::isRussian() ? Localization::tr("menu.exit") : std::string("Exit"));
    if (selectionInfoLabel)
    {
        selectionInfoLabel->setText(buildSelectionInfoText());
        selectionInfoLabel->setVisible(true);
    }
    if (systemInfoLabel)
    {
        systemInfoLabel->setText(buildSystemInfoText());
        systemInfoLabel->setVisible(false);
    }
    if (shortcutInfoLabel)
    {
        shortcutInfoLabel->setText(buildShortcutInfoText());
        shortcutInfoLabel->setVisible(false);
    }

    if (levelSelector->isVisible() && !state_.selectedLevelName.empty() && levelSelector->getSelectedItem().empty())
    {
        const auto selectedLabelIt = levelIdToSelectorLabel_.find(state_.selectedLevelName);
        if (selectedLabelIt != levelIdToSelectorLabel_.end())
        {
            levelSelector->setSelectedItem(selectedLabelIt->second);
        }
    }

    syncPopupInteractivity();
}

void Menu::applyModeLayout()
{
    refreshMenuContext();
    updateWidgetLayout();
    applyModeTheme();
    updateDecorativeLayout();
}

void Menu::updateWidgetLayout()
{
    background.setSize(static_cast<int>(window_m->getSize().x), static_cast<int>(window_m->getSize().y));

    const bool isMainMenu = mode_ == MenuMode::Main;
    const float panelWidth = getPanelWidth(mode_);
    const float panelHeight = getPanelHeight(mode_);
    const float centerX = window_m->getSize().x * 0.5f;
    const float panelTopBase = isMainMenu ? window_m->getSize().y * 0.12f : window_m->getSize().y * 0.16f;
    const float reveal = 1.f - std::pow(1.f - std::clamp(menuReveal_, 0.f, 1.f), 3.f);
    const float revealOffsetY = (1.f - reveal) * 34.f;
    const float panelTop = panelTopBase + revealOffsetY;

    const float titleWidth = panelWidth - 92.f;
    const float subtitleWidth = panelWidth - 116.f;
    const float footerWidth = panelWidth - 132.f;

    titleLabel->setSize({titleWidth, isMainMenu ? 68.f : 62.f});
    titleLabel->setPosition({centerX - titleWidth / 2.f, panelTop + 16.f});
    titleLabel->setTextSize(isMainMenu ? 50 : 38);

    subtitleLabel->setSize({subtitleWidth, isMainMenu ? 36.f : 38.f});
    subtitleLabel->setPosition({centerX - subtitleWidth / 2.f, panelTop + (isMainMenu ? 88.f : 84.f)});
    subtitleLabel->setTextSize(isMainMenu ? 18 : 17);

    footerLabel->setSize({footerWidth, 22.f});
    footerLabel->setPosition({centerX - footerWidth / 2.f, panelTop + panelHeight - (isMainMenu ? 34.f : 40.f)});
    footerLabel->setTextSize(isMainMenu ? 14 : 12);

    const float panelLeft = centerX - panelWidth / 2.f;

    if (isMainMenu)
    {
        const float contentWidth = 460.f;
        const float contentLeft = centerX - contentWidth / 2.f;
        const float selectorTop = panelTop + 240.f;

        selectionInfoLabel->setMaximumTextWidth(contentWidth);
        selectionInfoLabel->setSize({contentWidth, 66.f});
        selectionInfoLabel->setPosition({contentLeft, panelTop + 154.f});
        selectionInfoLabel->setTextSize(18);

        levelPrevButton->setVisible(false);
        levelNextButton->setVisible(false);
        randomLevelButton->setVisible(false);
        levelSelector->setVisible(false);
        startLevelButton->setVisible(false);
        restartLevelButton->setVisible(false);
        mainMenuButton->setVisible(false);

        continueButton->setSize({contentWidth, 52.f});
        startLevelButton->setSize({contentWidth, 52.f});
        restartLevelButton->setSize({contentWidth, 52.f});
        settingsButton->setSize({contentWidth, 52.f});
        controlsButton->setSize({contentWidth, 52.f});
        exitButton->setSize({contentWidth, 52.f});
        continueButton->setTextSize(26);
        startLevelButton->setTextSize(26);
        restartLevelButton->setTextSize(26);
        settingsButton->setTextSize(26);
        controlsButton->setTextSize(26);
        exitButton->setTextSize(26);

        continueButton->setPosition({contentLeft, selectorTop});
        startLevelButton->setPosition({contentLeft, selectorTop + 60.f});
        restartLevelButton->setPosition({contentLeft, selectorTop + 120.f});
        settingsButton->setPosition({contentLeft, selectorTop + 70.f});
        controlsButton->setPosition({contentLeft, selectorTop + 130.f});
        exitButton->setPosition({contentLeft, selectorTop + 190.f});
    }
    else
    {
        const float contentWidth = 440.f;
        const float contentLeft = centerX - contentWidth / 2.f;
        const float infoTop = panelTop + 150.f;

        selectionInfoLabel->setMaximumTextWidth(contentWidth);
        selectionInfoLabel->setSize({contentWidth, 66.f});
        selectionInfoLabel->setPosition({contentLeft, infoTop});
        selectionInfoLabel->setTextSize(18);

        levelPrevButton->setVisible(false);
        levelNextButton->setVisible(false);
        randomLevelButton->setVisible(false);
        levelSelector->setVisible(false);
        startLevelButton->setVisible(false);
        restartLevelButton->setVisible(false);
        mainMenuButton->setVisible(true);

        continueButton->setSize({contentWidth, 52.f});
        restartLevelButton->setSize({contentWidth, 52.f});
        mainMenuButton->setSize({contentWidth, 52.f});
        settingsButton->setSize({contentWidth, 52.f});
        controlsButton->setSize({contentWidth, 52.f});
        exitButton->setSize({contentWidth, 52.f});
        continueButton->setTextSize(26);
        restartLevelButton->setTextSize(26);
        mainMenuButton->setTextSize(26);
        settingsButton->setTextSize(26);
        controlsButton->setTextSize(26);
        exitButton->setTextSize(26);

        continueButton->setPosition({contentLeft, panelTop + 232.f});
        restartLevelButton->setPosition({contentLeft, panelTop + 292.f});
        mainMenuButton->setPosition({contentLeft, panelTop + 292.f});
        settingsButton->setPosition({contentLeft, panelTop + 352.f});
        controlsButton->setPosition({contentLeft, panelTop + 412.f});
        exitButton->setPosition({contentLeft, panelTop + 472.f});
    }

    if (settingsWindow)
    {
        const tgui::Vector2f settingsSize = settingsWindow->getSize();
        settingsWindow->setPosition({
            window_m->getSize().x * 0.5f - settingsSize.x * 0.5f,
            window_m->getSize().y * 0.5f - settingsSize.y * 0.5f
        });
    }

    if (controlsWindow)
    {
        const tgui::Vector2f controlsSize = controlsWindow->getSize();
        controlsWindow->setPosition({
            window_m->getSize().x * 0.5f - controlsSize.x * 0.5f,
            window_m->getSize().y * 0.5f - controlsSize.y * 0.5f
        });
    }
}

void Menu::applyModeTheme()
{
    const MenuTheme theme = getMenuTheme(mode_);

    background.setTheme(mode_ == MenuMode::Main ? MenuBackgroundTheme::Main : MenuBackgroundTheme::Pause);
    background.setParticleCount(menuParticleCount);

    styleLabel(titleLabel, true);
    styleLabel(subtitleLabel, false);
    styleLabel(footerLabel, false);
    footerLabel->getRenderer()->setTextColor(theme.footerText);

    styleButton(continueButton, ButtonStyleRole::Primary);
    styleButton(startLevelButton, ButtonStyleRole::Primary);
    styleButton(restartLevelButton, ButtonStyleRole::Secondary);
    styleButton(mainMenuButton, ButtonStyleRole::Secondary);
    styleButton(settingsButton, ButtonStyleRole::Secondary);
    styleButton(controlsButton, ButtonStyleRole::Secondary);
    styleButton(exitButton, ButtonStyleRole::Danger);
    styleButton(levelPrevButton, ButtonStyleRole::Secondary);
    styleButton(levelNextButton, ButtonStyleRole::Secondary);
    if (randomLevelButton)
    {
        randomLevelButton->setVisible(false);
        randomLevelButton->setEnabled(false);
    }
    styleButton(fullscreenToggleButton, ButtonStyleRole::Secondary);
    styleButton(vsyncToggleButton, ButtonStyleRole::Secondary);
    styleButton(particleToggleButton, ButtonStyleRole::Secondary);
    styleButton(languageToggleButton, ButtonStyleRole::Secondary);
    styleButton(resetProgressButton, ButtonStyleRole::Danger);
    styleInfoLabel(selectionInfoLabel);
    styleInfoLabel(systemInfoLabel);
    styleInfoLabel(shortcutInfoLabel);

    if (settingsWindow)
    {
        styleChildWindow(settingsWindow);
        if (auto settingsLabel = settingsWindow->get<tgui::Label>("settingsLabel"))
        {
            settingsLabel->getRenderer()->setTextColor(theme.subtitleText);
            settingsLabel->getRenderer()->setTextOutlineColor(sf::Color(0, 0, 0, 150));
            settingsLabel->getRenderer()->setTextOutlineThickness(1.f);
        }
        if (auto closeButton = settingsWindow->get<tgui::Button>("settingsCloseButton"))
        {
            styleButton(closeButton, ButtonStyleRole::Secondary);
        }
    }

    if (controlsWindow)
    {
        styleChildWindow(controlsWindow);
        if (auto controlsLabel = controlsWindow->get<tgui::Label>("controlsLabel"))
        {
            controlsLabel->getRenderer()->setTextColor(theme.subtitleText);
            controlsLabel->getRenderer()->setTextOutlineColor(sf::Color(0, 0, 0, 150));
            controlsLabel->getRenderer()->setTextOutlineThickness(1.f);
        }
        if (auto closeButton = controlsWindow->get<tgui::Button>("controlsCloseButton"))
        {
            styleButton(closeButton, ButtonStyleRole::Secondary);
        }
    }

    styleSelector();
}

bool Menu::isBlockingPopupOpen() const
{
    return (exitDialogue && exitDialogue->isOpen())
        || (resetProgressDialogue && resetProgressDialogue->isOpen())
        || (settingsWindow && settingsWindow->isVisible())
        || (controlsWindow && controlsWindow->isVisible());
}

void Menu::syncPopupInteractivity()
{
    const bool popupOpen = isBlockingPopupOpen();

    continueButton->setEnabled(!popupOpen && (mode_ == MenuMode::Main || state_.canContinue));
    startLevelButton->setEnabled(!popupOpen && !state_.availableLevels.empty());
    restartLevelButton->setEnabled(!popupOpen && state_.canRestartLevel && restartLevelButton->isVisible());
    mainMenuButton->setEnabled(!popupOpen);
    settingsButton->setEnabled(!popupOpen);
    controlsButton->setEnabled(!popupOpen);
    exitButton->setEnabled(!popupOpen);
    levelPrevButton->setEnabled(!popupOpen && !state_.availableLevels.empty() && levelPrevButton->isVisible());
    levelNextButton->setEnabled(!popupOpen && !state_.availableLevels.empty() && levelNextButton->isVisible());
    if (randomLevelButton)
    {
        randomLevelButton->setEnabled(false);
    }

    if (levelSelector)
    {
        levelSelector->setEnabled(!popupOpen && !state_.availableLevels.empty());
    }
}

void Menu::updateDecorativeLayout()
{
    const MenuTheme theme = getMenuTheme(mode_);
    const float panelWidth = getPanelWidth(mode_);
    const float panelHeight = getPanelHeight(mode_);
    const float centerX = window_m->getSize().x * 0.5f;
    const float reveal = 1.f - std::pow(1.f - std::clamp(menuReveal_, 0.f, 1.f), 3.f);
    const float panelTopBase = (mode_ == MenuMode::Main)
        ? window_m->getSize().y * 0.12f
        : window_m->getSize().y * 0.16f;
    const float panelTop = panelTopBase + (1.f - reveal) * 34.f;
    const float centerY = panelTop + panelHeight * 0.5f;
    const float pulse = 0.5f + 0.5f * std::sin(menuVisualTime_ * (mode_ == MenuMode::Main ? 1.3f : 0.65f));

    panelShadow_.setSize({panelWidth + 38.f, panelHeight + 46.f});
    panelShadow_.setOrigin({panelShadow_.getSize().x / 2.f, panelShadow_.getSize().y / 2.f});
    panelShadow_.setPosition({centerX + 10.f, centerY + 16.f});
    panelShadow_.setFillColor(theme.panelShadow);

    panelFrame_.setSize({panelWidth, panelHeight});
    panelFrame_.setOrigin({panelWidth / 2.f, panelHeight / 2.f});
    panelFrame_.setPosition({centerX, centerY});
    panelFrame_.setFillColor(theme.panelFrame);
    panelFrame_.setOutlineThickness(2.f);
    panelFrame_.setOutlineColor(theme.panelBorder);

    panelInset_.setSize({panelWidth - 28.f, panelHeight - 28.f});
    panelInset_.setOrigin({panelInset_.getSize().x / 2.f, panelInset_.getSize().y / 2.f});
    panelInset_.setPosition({centerX, centerY});
    panelInset_.setFillColor(theme.panelInset);
    panelInset_.setOutlineThickness(1.f);
    panelInset_.setOutlineColor(theme.panelInsetBorder);

    const float titleBandHeight = mode_ == MenuMode::Main ? 128.f : 122.f;
    titleBand_.setSize({panelWidth - 84.f, titleBandHeight});
    titleBand_.setOrigin({titleBand_.getSize().x / 2.f, titleBand_.getSize().y / 2.f});
    titleBand_.setPosition({centerX, panelTop + 18.f + titleBandHeight * 0.5f});
    titleBand_.setFillColor(theme.titleBand);
    titleBand_.setOutlineThickness(1.5f);
    titleBand_.setOutlineColor(brighten(theme.panelBorder, static_cast<int>(pulse * 16.f), theme.panelBorder.a));

    dividerLine_.setSize({panelWidth - 154.f, 2.f});
    dividerLine_.setOrigin({dividerLine_.getSize().x / 2.f, dividerLine_.getSize().y / 2.f});
    dividerLine_.setPosition({centerX, panelTop + (mode_ == MenuMode::Main ? 228.f : 224.f)});
    dividerLine_.setFillColor(theme.divider);

    footerBand_.setSize({panelWidth - 126.f, mode_ == MenuMode::Main ? 24.f : 18.f});
    footerBand_.setOrigin({footerBand_.getSize().x / 2.f, footerBand_.getSize().y / 2.f});
    footerBand_.setPosition({centerX, panelTop + panelHeight - 16.f});
    footerBand_.setFillColor(sf::Color(theme.titleBand.r, theme.titleBand.g, theme.titleBand.b, 140));

    verticalDivider_.setSize({0.f, 0.f});
    verticalDivider_.setFillColor(sf::Color::Transparent);

    if (levelSelector && levelSelector->isVisible())
    {
        const tgui::Vector2f selectorPos = levelSelector->getPosition();
        const tgui::Vector2f selectorSize = levelSelector->getSize();
        selectorBand_.setSize({selectorSize.x + 110.f, selectorSize.y + 18.f});
        selectorBand_.setOrigin({selectorBand_.getSize().x / 2.f, selectorBand_.getSize().y / 2.f});
        selectorBand_.setPosition({
            selectorPos.x + selectorSize.x * 0.5f + 24.f,
            selectorPos.y + selectorSize.y * 0.5f + 24.f
        });
        selectorBand_.setFillColor(sf::Color(theme.infoCardGlow.r, theme.infoCardGlow.g, theme.infoCardGlow.b,
            static_cast<std::uint8_t>(34 + pulse * 24.f)));
    }

    const std::array<tgui::Label::Ptr, 3> infoLabels{
        selectionInfoLabel,
        systemInfoLabel,
        shortcutInfoLabel
    };
    for (std::size_t i = 0; i < infoLabels.size(); ++i)
    {
        const auto& label = infoLabels[i];
        if (!label->isVisible())
        {
            infoCardGlows_[i].setSize({0.f, 0.f});
            infoCards_[i].setSize({0.f, 0.f});
            continue;
        }
        const tgui::Vector2f labelPos = label->getPosition();
        const tgui::Vector2f labelSize = label->getSize();

        infoCardGlows_[i].setSize({labelSize.x + 44.f, labelSize.y + 36.f});
        infoCardGlows_[i].setOrigin({infoCardGlows_[i].getSize().x / 2.f, infoCardGlows_[i].getSize().y / 2.f});
        infoCardGlows_[i].setPosition({
            labelPos.x + labelSize.x * 0.5f,
            labelPos.y + labelSize.y * 0.5f + std::sin(menuVisualTime_ * 0.8f + static_cast<float>(i)) * 2.f
        });
        infoCardGlows_[i].setFillColor(sf::Color(theme.infoCardGlow.r, theme.infoCardGlow.g, theme.infoCardGlow.b,
            static_cast<std::uint8_t>(28 + pulse * 18.f)));

        infoCards_[i].setSize({labelSize.x + 28.f, labelSize.y + 20.f});
        infoCards_[i].setOrigin({infoCards_[i].getSize().x / 2.f, infoCards_[i].getSize().y / 2.f});
        infoCards_[i].setPosition({labelPos.x + labelSize.x * 0.5f, labelPos.y + labelSize.y * 0.5f});
        infoCards_[i].setFillColor(theme.infoCardFill);
        infoCards_[i].setOutlineThickness(1.5f);
        infoCards_[i].setOutlineColor(brighten(theme.infoCardBorder, static_cast<int>(pulse * 10.f), theme.infoCardBorder.a));
    }

    verticalDivider_.setSize({0.f, 0.f});
    selectorBand_.setFillColor(sf::Color(theme.infoCardGlow.r, theme.infoCardGlow.g, theme.infoCardGlow.b, 36));
}

void Menu::drawDecorativeLayout(sf::RenderWindow& window)
{
    window.draw(panelShadow_);
    window.draw(panelFrame_);
    window.draw(panelInset_);
    window.draw(titleBand_);
    window.draw(dividerLine_);
    if (mode_ == MenuMode::Main && levelSelector->isVisible())
    {
        window.draw(selectorBand_);
    }
    for (auto& glow : infoCardGlows_)
    {
        window.draw(glow);
    }
    for (auto& card : infoCards_)
    {
        window.draw(card);
    }
    window.draw(footerBand_);
}

void Menu::closePopups()
{
    if (settingsWindow)
    {
        settingsWindow->setVisible(false);
    }
    if (controlsWindow)
    {
        controlsWindow->setVisible(false);
    }
    if (exitDialogue)
    {
        exitDialogue->close();
    }
    if (resetProgressDialogue)
    {
        resetProgressDialogue->close();
    }

    syncPopupInteractivity();
}

void Menu::resumeButtonOnClick()
{
    if (mode_ == MenuMode::Main)
    {
        startSelectedLevelOnClick();
        return;
    }

    if (!state_.canContinue)
    {
        return;
    }

    closePopups();
    callbacks_.onResumeGame();
    close();
}

void Menu::startSelectedLevelOnClick()
{
    if (state_.selectedLevelName.empty())
    {
        return;
    }

    if (callbacks_.onStartSelectedLevel(state_.selectedLevelName))
    {
        closePopups();
        close();
    }
}

void Menu::restartCurrentLevelOnClick()
{
    if (!state_.canRestartLevel)
    {
        return;
    }

    if (callbacks_.onRestartCurrentLevel())
    {
        closePopups();
        close();
    }
}

void Menu::returnToMainMenuOnClick()
{
    closePopups();
    callbacks_.onReturnToMainMenu();
    openMainMenu();
}

void Menu::settingsButtonOnClick()
{
    if (controlsWindow)
    {
        controlsWindow->setVisible(false);
    }
    settingsWindow->setVisible(true);
    syncPopupInteractivity();
}

void Menu::controlsButtonOnClick()
{
    if (settingsWindow)
    {
        settingsWindow->setVisible(false);
    }
    controlsWindow->setVisible(true);
    syncPopupInteractivity();
}

void Menu::toggleFullscreen()
{
    const bool newFullscreenEnabled = !fullscreenEnabled;
    const bool applied = callbacks_.onSetFullscreen(newFullscreenEnabled);
    if (!applied)
    {
        callbacks_.onNotify(
            Localization::isRussian() ? Localization::tr("settings.error") : "Settings error",
            Localization::isRussian() ? Localization::tr("settings.fullscreen_failed") : "Fullscreen mode could not be applied.",
            NotificationTone::Warning
        );
        refreshMenuContext();
        return;
    }

    fullscreenEnabled = newFullscreenEnabled;
    if (gameData_m)
    {
        gameData_m->setFullscreenEnabled(fullscreenEnabled);
    }
    callbacks_.onNotify(
        Localization::isRussian() ? Localization::tr("settings.saved") : "Settings saved",
        fullscreenEnabled
            ? (Localization::isRussian() ? Localization::tr("settings.fullscreen_enabled") : "Fullscreen enabled.")
            : (Localization::isRussian() ? Localization::tr("settings.fullscreen_disabled") : "Fullscreen disabled."),
        NotificationTone::Info
    );
    refreshMenuContext();
}

void Menu::toggleVsync()
{
    vsyncEnabled = !vsyncEnabled;
    window_m->setVerticalSyncEnabled(vsyncEnabled);
    window_m->setFramerateLimit(vsyncEnabled ? 0u : WINDOW_FPS);
    if (gameData_m)
    {
        gameData_m->setVsyncEnabled(vsyncEnabled);
    }
    callbacks_.onNotify(
        Localization::isRussian() ? Localization::tr("settings.saved") : "Settings saved",
        vsyncEnabled
            ? (Localization::isRussian() ? Localization::tr("settings.vsync_enabled") : "VSync enabled.")
            : (Localization::isRussian() ? Localization::tr("settings.vsync_disabled") : "VSync disabled."),
        NotificationTone::Info
    );
    refreshMenuContext();
}

void Menu::toggleMenuParticles()
{
    menuParticleCount = getNextAshDensityCount(menuParticleCount);
    background.setParticleCount(menuParticleCount);
    if (gameData_m)
    {
        gameData_m->setMenuParticleCount(menuParticleCount);
    }
    callbacks_.onNotify(
        Localization::isRussian() ? Localization::tr("settings.saved") : "Settings saved",
        (Localization::isRussian() ? Localization::tr("settings.ash_saved") : "Menu ash density") +
            std::string(": ") + getAshDensityLabel(menuParticleCount) + ".",
        NotificationTone::Info
    );
    refreshMenuContext();
}

void Menu::toggleLanguage()
{
    const Language next = Localization::nextLanguage(Localization::getLanguage());
    Localization::setLanguage(next);
    if (gameData_m)
    {
        gameData_m->setLanguage(next);
    }
    callbacks_.onNotify(
        Localization::isRussian() ? Localization::tr("settings.saved") : "Settings saved",
        Localization::isRussian() ? Localization::tr("settings.language_saved") : "Language changed.",
        NotificationTone::Info
    );
    refreshLevelSelector();
    refreshMenuContext();
    applyModeTheme();
}

void Menu::resetProgressButtonOnClick()
{
    if (resetProgressDialogue)
    {
        resetProgressDialogue->open();
        syncPopupInteractivity();
    }
}

void Menu::exitButtonOnClick()
{
    exitDialogue->open();
    syncPopupInteractivity();
}

void Menu::openMainMenu()
{
    syncPreferencesFromGameData();
    mode_ = MenuMode::Main;
    isOpen_ = true;
    closePopups();
    menuAnimationClock.restart();
    menuVisualTime_ = 0.f;
    menuReveal_ = 0.f;
    applyModeLayout();
}

void Menu::openPauseMenu()
{
    syncPreferencesFromGameData();
    mode_ = MenuMode::Pause;
    isOpen_ = true;
    closePopups();
    menuAnimationClock.restart();
    menuVisualTime_ = 0.f;
    menuReveal_ = 0.f;
    applyModeLayout();
}

void Menu::close()
{
    isOpen_ = false;
    closePopups();
    menuAnimationClock.restart();
    menuReveal_ = 0.f;
}

bool Menu::isOpen() const
{
    return isOpen_;
}

bool Menu::isMainMenu() const
{
    return isOpen_ && mode_ == MenuMode::Main;
}

bool Menu::isPauseMenu() const
{
    return isOpen_ && mode_ == MenuMode::Pause;
}

const std::string& Menu::getSelectedLevelName() const
{
    return state_.selectedLevelName;
}

void Menu::menuDraw(sf::RenderWindow& window)
{
    float deltaTime = menuAnimationClock.restart().asSeconds();
    if (deltaTime <= 0.f)
    {
        deltaTime = 1.f / 60.f;
    }

    menuVisualTime_ += deltaTime;
    menuReveal_ = std::min(1.f, menuReveal_ + deltaTime * 4.8f);
    refreshMenuContext();
    updateWidgetLayout();
    background.update(deltaTime);
    updateDecorativeLayout();

    background.draw(window);
    drawDecorativeLayout(window);
    gui.draw();
    exitDialogue->draw(window);
    resetProgressDialogue->draw(window);
}

void Menu::menuHandleEvents(const sf::Event& ev)
{
    if (const auto* keyPressed = ev.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
        {
            if (exitDialogue->isOpen())
            {
                exitDialogue->close();
                syncPopupInteractivity();
                return;
            }
            if (resetProgressDialogue->isOpen())
            {
                resetProgressDialogue->close();
                syncPopupInteractivity();
                return;
            }
            if (settingsWindow->isVisible())
            {
                settingsWindow->setVisible(false);
                syncPopupInteractivity();
                return;
            }
            if (controlsWindow->isVisible())
            {
                controlsWindow->setVisible(false);
                syncPopupInteractivity();
                return;
            }
            if (isPauseMenu())
            {
                close();
                return;
            }
        }

        if (!isBlockingPopupOpen())
        {
            const sf::Keyboard::Scancode code = keyPressed->scancode;

            if (code == sf::Keyboard::Scancode::S)
            {
                settingsButtonOnClick();
                return;
            }
            if (code == sf::Keyboard::Scancode::C)
            {
                controlsButtonOnClick();
                return;
            }
            if (code == sf::Keyboard::Scancode::V)
            {
                toggleVsync();
                return;
            }
            if (code == sf::Keyboard::Scancode::P)
            {
                toggleMenuParticles();
                return;
            }

            if (isMainMenu())
            {
                if (code == sf::Keyboard::Scancode::Enter)
                {
                    resumeButtonOnClick();
                    return;
                }
            }
            else if (isPauseMenu())
            {
                if (code == sf::Keyboard::Scancode::Enter)
                {
                    resumeButtonOnClick();
                    return;
                }
                if (code == sf::Keyboard::Scancode::M)
                {
                    returnToMainMenuOnClick();
                    return;
                }
            }
        }
    }

    if (exitDialogue->isOpen())
    {
        exitDialogue->handleEvent(ev);
        return;
    }

    if (resetProgressDialogue->isOpen())
    {
        resetProgressDialogue->handleEvent(ev);
        return;
    }

    if ((settingsWindow && settingsWindow->isVisible()) || (controlsWindow && controlsWindow->isVisible()))
    {
        gui.handleEvent(ev);
        return;
    }

    gui.handleEvent(ev);
}
