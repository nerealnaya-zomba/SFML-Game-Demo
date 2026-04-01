#include <Menu.h>

#include <TGUI/Renderers/ListBoxRenderer.hpp>

#include <algorithm>
#include <cmath>
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
};

constexpr float kMainPanelWidth = 680.f;
constexpr float kMainPanelHeight = 640.f;
constexpr float kPausePanelWidth = 560.f;
constexpr float kPausePanelHeight = 520.f;

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

Menu::Menu(sf::RenderWindow& window)
    : window_m(&window)
    , background(static_cast<int>(window.getSize().x), static_cast<int>(window.getSize().y))
{
    gui.setWindow(window);
    setupMainWidgets();

    const sf::Vector2u windowSize = window.getSize();
    exitDialogue = std::make_unique<AskDialogue>(
        sf::Vector2f(windowSize.x / 2.f, windowSize.y / 2.f),
        sf::Vector2f(350.f, 150.f),
        "Quit the game?",
        *window_m
    );
    exitDialogue->setOnYesClick([this]() { callbacks_.onExitGame(); });
    exitDialogue->setOnNoClick([this]() { exitDialogue->close(); });

    openMainMenu();
}

Menu::~Menu() = default;

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
    titleLabel = tgui::Label::create("SATIRO");
    titleLabel->setTextSize(52);
    gui.add(titleLabel);

    subtitleLabel = tgui::Label::create("Stand before the ash and choose your path");
    subtitleLabel->setTextSize(19);
    gui.add(subtitleLabel);

    footerLabel = tgui::Label::create("Esc closes the pause veil");
    footerLabel->setTextSize(17);
    gui.add(footerLabel);

    initializeLevelSelector();

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
    levelSelector->setDefaultText("Select a level");
    levelSelector->addItem(state_.selectedLevelName);
    levelSelector->setSelectedItem(state_.selectedLevelName);
    levelSelector->onItemSelect([this](const tgui::String& item) {
        state_.selectedLevelName = item.toStdString();
        refreshMenuContext();
    });
    gui.add(levelSelector);
}

void Menu::initializeSettingsWindow()
{
    settingsWindow = tgui::ChildWindow::create();
    settingsWindow->setTitle("Ritual Settings");
    settingsWindow->setClientSize({420.f, 250.f});
    settingsWindow->setPosition({
        window_m->getSize().x / 2.f - 210.f,
        window_m->getSize().y / 2.f - 160.f
    });
    settingsWindow->setVisible(false);

    auto settingsLabel = tgui::Label::create("Tune the veil of this session");
    settingsLabel->setWidgetName("settingsLabel");
    settingsLabel->setTextSize(20);
    settingsLabel->setPosition({0.f, 6.f});
    settingsLabel->setSize({388.f, 26.f});
    settingsLabel->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
    settingsWindow->add(settingsLabel);

    vsyncToggleButton = tgui::Button::create();
    vsyncToggleButton->setSize({360.f, 42.f});
    vsyncToggleButton->setPosition({14.f, 56.f});
    vsyncToggleButton->setTextSize(22);
    vsyncToggleButton->onClick([this]() { toggleVsync(); });
    settingsWindow->add(vsyncToggleButton);

    particleToggleButton = tgui::Button::create();
    particleToggleButton->setSize({360.f, 42.f});
    particleToggleButton->setPosition({14.f, 106.f});
    particleToggleButton->setTextSize(22);
    particleToggleButton->onClick([this]() { toggleMenuParticles(); });
    settingsWindow->add(particleToggleButton);

    auto closeButton = tgui::Button::create();
    closeButton->setWidgetName("settingsCloseButton");
    closeButton->setSize({360.f, 42.f});
    closeButton->setPosition({14.f, 156.f});
    closeButton->setText("Close");
    closeButton->setTextSize(22);
    closeButton->onClick([this]() { settingsWindow->setVisible(false); });
    settingsWindow->add(closeButton);

    gui.add(settingsWindow);
}

void Menu::initializeControlsWindow()
{
    controlsWindow = tgui::ChildWindow::create();
    controlsWindow->setTitle("Traveler's Codex");
    controlsWindow->setClientSize({470.f, 388.f});
    controlsWindow->setPosition({
        window_m->getSize().x / 2.f - 235.f,
        window_m->getSize().y / 2.f - 205.f
    });
    controlsWindow->setVisible(false);

    auto controlsLabel = tgui::Label::create(
        "Movement\n"
        "Left / Right - Move\n"
        "Z - Jump\n"
        "C - Dash\n\n"
        "Combat\n"
        "X - Shoot\n"
        "R - Open portal\n"
        "E - Destination menu\n"
        "Enter - Confirm\n\n"
        "Menus\n"
        "Escape - Pause"
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
    closeButton->setText("Close");
    closeButton->setTextSize(22);
    closeButton->onClick([this]() { controlsWindow->setVisible(false); });
    controlsWindow->add(closeButton);

    gui.add(controlsWindow);
}

void Menu::connectTGUIFont(tgui::Font& font)
{
    gui.setFont(font);
    exitDialogue->connectTGUIFont(font);
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

void Menu::refreshLevelSelector()
{
    levelSelector->removeAllItems();

    for (const auto& levelName : state_.availableLevels)
    {
        levelSelector->addItem(levelName);
    }

    if (state_.availableLevels.empty())
    {
        state_.selectedLevelName.clear();
        levelSelector->addItem("No levels available");
        levelSelector->setSelectedItem("No levels available");
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

    if (!levelSelector->setSelectedItem(state_.selectedLevelName))
    {
        state_.selectedLevelName = state_.availableLevels.front();
        levelSelector->setSelectedItem(state_.selectedLevelName);
    }
}

void Menu::refreshMenuContext()
{
    if (vsyncToggleButton)
    {
        vsyncToggleButton->setText(vsyncEnabled ? "VSync: ON" : "VSync: OFF");
    }

    if (particleToggleButton)
    {
        particleToggleButton->setText(std::string("Ash density: ") + std::to_string(menuParticleCount));
    }

    if (mode_ == MenuMode::Pause)
    {
        titleLabel->setText("PAUSED");
        if (state_.currentLevelName.empty())
        {
            subtitleLabel->setText("The world waits behind the veil");
        }
        else
        {
            subtitleLabel->setText("The world waits on: " + state_.currentLevelName);
        }

        footerLabel->setText("Press Esc again to return to the world");
    }
    else
    {
        titleLabel->setText("SATIRO");

        if (state_.availableLevels.empty())
        {
            subtitleLabel->setText("No playable levels were found in the ash");
        }
        else if (state_.currentLevelName.empty())
        {
            subtitleLabel->setText("Choose where the journey begins");
        }
        else
        {
            subtitleLabel->setText(
                std::string("Current level: ") + state_.currentLevelName +
                "\nSelected start: " + state_.selectedLevelName
            );
        }

        footerLabel->setText(state_.canContinue
            ? "Resume the fallen path or choose a fresh descent"
            : "A crimson path opens only when you choose a level");
    }

    continueButton->setText(mode_ == MenuMode::Pause ? BASE_RESUME_BUTTON_TEXT : BASE_PLAY_BUTTON_TEXT);
    continueButton->setEnabled(state_.canContinue);
    startLevelButton->setEnabled(!state_.availableLevels.empty());
    restartLevelButton->setEnabled(state_.canRestartLevel);

    if (levelSelector->isVisible() && !state_.selectedLevelName.empty() && levelSelector->getSelectedItem().empty())
    {
        levelSelector->setSelectedItem(state_.selectedLevelName);
    }
}

void Menu::applyModeLayout()
{
    const bool isMainMenu = mode_ == MenuMode::Main;
    const float panelWidth = getPanelWidth(mode_);
    const float panelHeight = getPanelHeight(mode_);
    const float centerX = window_m->getSize().x * 0.5f;
    const float panelTop = isMainMenu ? window_m->getSize().y * 0.14f : window_m->getSize().y * 0.18f;

    titleLabel->setSize({panelWidth - 70.f, 64.f});
    titleLabel->setPosition({centerX - (panelWidth - 70.f) / 2.f, panelTop + 18.f});
    titleLabel->setTextSize(isMainMenu ? 56 : 42);

    subtitleLabel->setSize({panelWidth - 100.f, isMainMenu ? 74.f : 58.f});
    subtitleLabel->setPosition({centerX - (panelWidth - 100.f) / 2.f, panelTop + 90.f});
    subtitleLabel->setTextSize(isMainMenu ? 19 : 18);

    footerLabel->setSize({panelWidth - 120.f, 26.f});
    footerLabel->setPosition({centerX - (panelWidth - 120.f) / 2.f, panelTop + panelHeight - 52.f});

    continueButton->setSize(isMainMenu ? 392.f : 344.f, isMainMenu ? 56.f : 52.f);
    startLevelButton->setSize(392.f, 56.f);
    restartLevelButton->setSize(isMainMenu ? 392.f : 344.f, isMainMenu ? 56.f : 52.f);
    mainMenuButton->setSize(344.f, 52.f);
    settingsButton->setSize(isMainMenu ? 392.f : 344.f, isMainMenu ? 56.f : 52.f);
    controlsButton->setSize(isMainMenu ? 392.f : 344.f, isMainMenu ? 56.f : 52.f);
    exitButton->setSize(isMainMenu ? 392.f : 344.f, isMainMenu ? 56.f : 52.f);

    if (isMainMenu)
    {
        levelSelector->setPosition({centerX - 200.f, panelTop + 176.f});
        levelSelector->setSize({400.f, 42.f});

        continueButton->setPosition({centerX - 196.f, panelTop + 252.f});
        startLevelButton->setPosition({centerX - 196.f, panelTop + 316.f});
        restartLevelButton->setPosition({centerX - 196.f, panelTop + 380.f});
        settingsButton->setPosition({centerX - 196.f, panelTop + 444.f});
        controlsButton->setPosition({centerX - 196.f, panelTop + 508.f});
        exitButton->setPosition({centerX - 196.f, panelTop + 572.f});
    }
    else
    {
        continueButton->setPosition({centerX - 172.f, panelTop + 164.f});
        restartLevelButton->setPosition({centerX - 172.f, panelTop + 224.f});
        mainMenuButton->setPosition({centerX - 172.f, panelTop + 284.f});
        settingsButton->setPosition({centerX - 172.f, panelTop + 344.f});
        controlsButton->setPosition({centerX - 172.f, panelTop + 404.f});
        exitButton->setPosition({centerX - 172.f, panelTop + 464.f});
    }

    levelSelector->setVisible(isMainMenu);
    startLevelButton->setVisible(isMainMenu);
    mainMenuButton->setVisible(!isMainMenu);

    applyModeTheme();
    updateDecorativeLayout();
    refreshMenuContext();
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
    styleButton(vsyncToggleButton, ButtonStyleRole::Secondary);
    styleButton(particleToggleButton, ButtonStyleRole::Secondary);

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

void Menu::updateDecorativeLayout()
{
    const MenuTheme theme = getMenuTheme(mode_);
    const float panelWidth = getPanelWidth(mode_);
    const float panelHeight = getPanelHeight(mode_);
    const float centerX = window_m->getSize().x * 0.5f;
    const float centerY = (mode_ == MenuMode::Main)
        ? window_m->getSize().y * 0.14f + panelHeight * 0.5f
        : window_m->getSize().y * 0.18f + panelHeight * 0.5f;
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

    titleBand_.setSize({panelWidth - 84.f, mode_ == MenuMode::Main ? 112.f : 98.f});
    titleBand_.setOrigin({titleBand_.getSize().x / 2.f, titleBand_.getSize().y / 2.f});
    titleBand_.setPosition({centerX, centerY - panelHeight * 0.35f});
    titleBand_.setFillColor(theme.titleBand);
    titleBand_.setOutlineThickness(1.5f);
    titleBand_.setOutlineColor(theme.panelBorder);

    dividerLine_.setSize({panelWidth - 154.f, 2.f});
    dividerLine_.setOrigin({dividerLine_.getSize().x / 2.f, dividerLine_.getSize().y / 2.f});
    dividerLine_.setPosition({centerX, centerY - panelHeight * 0.2f});
    dividerLine_.setFillColor(theme.divider);

    footerBand_.setSize({panelWidth - 110.f, 36.f});
    footerBand_.setOrigin({footerBand_.getSize().x / 2.f, footerBand_.getSize().y / 2.f});
    footerBand_.setPosition({centerX, centerY + panelHeight * 0.42f});
    footerBand_.setFillColor(sf::Color(theme.titleBand.r, theme.titleBand.g, theme.titleBand.b, 140));

    for (std::size_t i = 0; i < ornamentLines_.size(); ++i)
    {
        auto& line = ornamentLines_[i];
        line.setSize({2.f, panelHeight - 160.f});
        line.setOrigin({line.getSize().x / 2.f, line.getSize().y / 2.f});
        line.setPosition({
            centerX + (i == 0 ? -(panelWidth * 0.38f) : panelWidth * 0.38f),
            centerY + std::sin(menuVisualTime_ * 0.8f + static_cast<float>(i)) * 5.f
        });
        line.setFillColor(theme.ornament);
    }

    sigilOuterRing_.setRadius(mode_ == MenuMode::Main ? 142.f : 104.f);
    sigilOuterRing_.setPointCount(mode_ == MenuMode::Main ? 8u : 20u);
    sigilOuterRing_.setOrigin({sigilOuterRing_.getRadius(), sigilOuterRing_.getRadius()});
    sigilOuterRing_.setPosition({centerX, centerY + (mode_ == MenuMode::Main ? 12.f : -6.f)});
    sigilOuterRing_.setScale({1.02f + pulse * 0.03f, 0.96f + pulse * 0.03f});
    sigilOuterRing_.setFillColor(sf::Color::Transparent);
    sigilOuterRing_.setOutlineThickness(mode_ == MenuMode::Main ? 2.f : 1.5f);
    sigilOuterRing_.setOutlineColor(theme.sigil);
    sigilOuterRing_.setRotation(sf::degrees(menuVisualTime_ * (mode_ == MenuMode::Main ? 7.f : 1.4f)));

    sigilInnerRing_.setRadius(mode_ == MenuMode::Main ? 84.f : 64.f);
    sigilInnerRing_.setPointCount(mode_ == MenuMode::Main ? 6u : 16u);
    sigilInnerRing_.setOrigin({sigilInnerRing_.getRadius(), sigilInnerRing_.getRadius()});
    sigilInnerRing_.setPosition(sigilOuterRing_.getPosition());
    sigilInnerRing_.setScale({0.96f + pulse * 0.04f, 0.94f + pulse * 0.04f});
    sigilInnerRing_.setFillColor(sf::Color::Transparent);
    sigilInnerRing_.setOutlineThickness(1.5f);
    sigilInnerRing_.setOutlineColor(theme.sigilSoft);
    sigilInnerRing_.setRotation(sf::degrees(-menuVisualTime_ * (mode_ == MenuMode::Main ? 10.f : 2.2f)));

    sigilVerticalBar_.setSize({2.f, mode_ == MenuMode::Main ? 166.f : 122.f});
    sigilVerticalBar_.setOrigin({sigilVerticalBar_.getSize().x / 2.f, sigilVerticalBar_.getSize().y / 2.f});
    sigilVerticalBar_.setPosition(sigilOuterRing_.getPosition());
    sigilVerticalBar_.setRotation(sf::degrees(menuVisualTime_ * (mode_ == MenuMode::Main ? 7.f : 1.4f)));
    sigilVerticalBar_.setFillColor(theme.sigilSoft);

    sigilHorizontalBar_.setSize({mode_ == MenuMode::Main ? 166.f : 122.f, 2.f});
    sigilHorizontalBar_.setOrigin({sigilHorizontalBar_.getSize().x / 2.f, sigilHorizontalBar_.getSize().y / 2.f});
    sigilHorizontalBar_.setPosition(sigilOuterRing_.getPosition());
    sigilHorizontalBar_.setRotation(sf::degrees(menuVisualTime_ * (mode_ == MenuMode::Main ? 7.f : 1.4f)));
    sigilHorizontalBar_.setFillColor(theme.sigilSoft);
}

void Menu::drawDecorativeLayout(sf::RenderWindow& window)
{
    window.draw(panelShadow_);
    window.draw(panelFrame_);
    window.draw(sigilOuterRing_);
    window.draw(sigilInnerRing_);
    window.draw(sigilVerticalBar_);
    window.draw(sigilHorizontalBar_);
    window.draw(panelInset_);
    window.draw(titleBand_);
    window.draw(dividerLine_);
    for (auto& line : ornamentLines_)
    {
        window.draw(line);
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
}

void Menu::resumeButtonOnClick()
{
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
}

void Menu::controlsButtonOnClick()
{
    if (settingsWindow)
    {
        settingsWindow->setVisible(false);
    }
    controlsWindow->setVisible(true);
}

void Menu::toggleVsync()
{
    vsyncEnabled = !vsyncEnabled;
    window_m->setVerticalSyncEnabled(vsyncEnabled);
    window_m->setFramerateLimit(vsyncEnabled ? 0u : WINDOW_FPS);
    refreshMenuContext();
}

void Menu::toggleMenuParticles()
{
    menuParticleCount = (menuParticleCount <= 120) ? 260 : 120;
    background.setParticleCount(menuParticleCount);
    refreshMenuContext();
}

void Menu::exitButtonOnClick()
{
    exitDialogue->open();
}

void Menu::openMainMenu()
{
    mode_ = MenuMode::Main;
    isOpen_ = true;
    closePopups();
    menuAnimationClock.restart();
    menuVisualTime_ = 0.f;
    applyModeLayout();
}

void Menu::openPauseMenu()
{
    mode_ = MenuMode::Pause;
    isOpen_ = true;
    closePopups();
    menuAnimationClock.restart();
    menuVisualTime_ = 0.f;
    applyModeLayout();
}

void Menu::close()
{
    isOpen_ = false;
    closePopups();
    menuAnimationClock.restart();
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
    refreshMenuContext();
    background.update(deltaTime);
    updateDecorativeLayout();

    background.draw(window);
    drawDecorativeLayout(window);
    gui.draw();
    exitDialogue->draw(window);
    window.display();
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
                return;
            }
            if (settingsWindow->isVisible())
            {
                settingsWindow->setVisible(false);
                return;
            }
            if (controlsWindow->isVisible())
            {
                controlsWindow->setVisible(false);
                return;
            }
            if (isPauseMenu())
            {
                close();
                return;
            }
        }
    }

    gui.handleEvent(ev);
    exitDialogue->handleEvent(ev);
}
