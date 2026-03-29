#include <Menu.h>

#include <utility>

namespace
{
constexpr sf::Vector2f kMenuButtonSize{280.f, 54.f};
constexpr float kMenuButtonSpacing = 66.f;
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

void Menu::styleButton(const tgui::Button::Ptr& button) const
{
    button->getRenderer()->setBackgroundColor(BASE_IDLE_COLOR);
    button->getRenderer()->setBackgroundColorHover(BASE_HOVER_COLOR);
    button->getRenderer()->setBackgroundColorDown(BASE_CLICK_COLOR);
    button->getRenderer()->setTextColor(BASE_TEXT_IDLE_COLOR);
    button->getRenderer()->setTextColorDown(BASE_TEXT_CLICK_COLOR);
    button->getRenderer()->setTextColorHover(BASE_TEXT_HOVER_COLOR);
}

tgui::Button::Ptr Menu::createMenuButton(const tgui::String& text, float y)
{
    auto button = tgui::Button::create();
    button->setSize(kMenuButtonSize.x, kMenuButtonSize.y);
    button->setPosition({
        (window_m->getSize().x / 2.f) - (kMenuButtonSize.x / 2.f),
        y
    });
    button->setText(text);
    button->setTextSize(BASE_MENU_BUTTONS_CHARACTER_SIZE);
    styleButton(button);
    gui.add(button);
    return button;
}

void Menu::setupMainWidgets()
{
    const float menuTop = window_m->getSize().y * 0.32f;

    titleLabel = tgui::Label::create("Satiro Control Center");
    titleLabel->setTextSize(34);
    titleLabel->setPosition({window_m->getSize().x / 2.f - 165.f, 42.f});
    gui.add(titleLabel);

    subtitleLabel = tgui::Label::create("Choose what to do next");
    subtitleLabel->setTextSize(18);
    subtitleLabel->setPosition({window_m->getSize().x / 2.f - 150.f, 90.f});
    gui.add(subtitleLabel);

    initializeLevelSelector();

    continueButton = createMenuButton(BASE_PLAY_BUTTON_TEXT, menuTop);
    continueButton->onClick([this]() { resumeButtonOnClick(); });

    startLevelButton = createMenuButton(BASE_START_BUTTON_TEXT, menuTop + kMenuButtonSpacing);
    startLevelButton->onClick([this]() { startSelectedLevelOnClick(); });

    restartLevelButton = createMenuButton(BASE_RESTART_BUTTON_TEXT, menuTop + kMenuButtonSpacing * 2.f);
    restartLevelButton->onClick([this]() { restartCurrentLevelOnClick(); });

    mainMenuButton = createMenuButton(BASE_RETURN_TO_MAIN_MENU_BUTTON_TEXT, menuTop + kMenuButtonSpacing);
    mainMenuButton->onClick([this]() { returnToMainMenuOnClick(); });

    settingsButton = createMenuButton(BASE_SETTINGS_BUTTON_TEXT, menuTop + kMenuButtonSpacing * 3.f);
    settingsButton->onClick([this]() { settingsButtonOnClick(); });

    controlsButton = createMenuButton(BASE_CONTROLS_BUTTON_TEXT, menuTop + kMenuButtonSpacing * 4.f);
    controlsButton->onClick([this]() { controlsButtonOnClick(); });

    exitButton = createMenuButton(BASE_EXIT_BUTTON_TEXT, menuTop + kMenuButtonSpacing * 5.f);
    exitButton->onClick([this]() { exitButtonOnClick(); });

    initializeSettingsWindow();
    initializeControlsWindow();
    applyModeLayout();
    refreshMenuContext();
}

void Menu::initializeLevelSelector()
{
    levelSelector = tgui::ComboBox::create();
    levelSelector->setSize({320.f, 36.f});
    levelSelector->setPosition({
        (window_m->getSize().x / 2.f) - 160.f,
        window_m->getSize().y * 0.24f
    });
    levelSelector->setTextSize(20);
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
    settingsWindow->setTitle("Settings");
    settingsWindow->setClientSize({380.f, 220.f});
    settingsWindow->setPosition({
        window_m->getSize().x / 2.f - 190.f,
        window_m->getSize().y / 2.f - 140.f
    });
    settingsWindow->setVisible(false);

    auto settingsLabel = tgui::Label::create("Realtime options for this session");
    settingsLabel->setTextSize(20);
    settingsLabel->setPosition({18.f, 16.f});
    settingsWindow->add(settingsLabel);

    vsyncToggleButton = tgui::Button::create();
    vsyncToggleButton->setSize({340.f, 40.f});
    vsyncToggleButton->setPosition({18.f, 60.f});
    styleButton(vsyncToggleButton);
    vsyncToggleButton->onClick([this]() { toggleVsync(); });
    settingsWindow->add(vsyncToggleButton);

    particleToggleButton = tgui::Button::create();
    particleToggleButton->setSize({340.f, 40.f});
    particleToggleButton->setPosition({18.f, 108.f});
    styleButton(particleToggleButton);
    particleToggleButton->onClick([this]() { toggleMenuParticles(); });
    settingsWindow->add(particleToggleButton);

    auto closeButton = tgui::Button::create();
    closeButton->setSize({340.f, 40.f});
    closeButton->setPosition({18.f, 156.f});
    closeButton->setText("Close");
    styleButton(closeButton);
    closeButton->onClick([this]() { settingsWindow->setVisible(false); });
    settingsWindow->add(closeButton);

    gui.add(settingsWindow);
}

void Menu::initializeControlsWindow()
{
    controlsWindow = tgui::ChildWindow::create();
    controlsWindow->setTitle("Controls");
    controlsWindow->setClientSize({420.f, 260.f});
    controlsWindow->setPosition({
        window_m->getSize().x / 2.f - 210.f,
        window_m->getSize().y / 2.f - 160.f
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
    controlsLabel->setTextSize(20);
    controlsLabel->setPosition({18.f, 16.f});
    controlsWindow->add(controlsLabel);

    auto closeButton = tgui::Button::create();
    closeButton->setSize({380.f, 40.f});
    closeButton->setPosition({18.f, 200.f});
    closeButton->setText("Close");
    styleButton(closeButton);
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
        particleToggleButton->setText(std::string("Menu particles: ") + std::to_string(menuParticleCount));
    }

    if (mode_ == MenuMode::Pause)
    {
        titleLabel->setText("Pause");
        if (state_.currentLevelName.empty())
        {
            subtitleLabel->setText("Game paused");
        }
        else
        {
            subtitleLabel->setText("Paused on: " + state_.currentLevelName);
        }
    }
    else
    {
        titleLabel->setText("Satiro Control Center");

        if (state_.availableLevels.empty())
        {
            subtitleLabel->setText("No playable levels were found");
        }
        else if (state_.currentLevelName.empty())
        {
            subtitleLabel->setText("Choose a level to start");
        }
        else
        {
            subtitleLabel->setText(
                std::string("Current level: ") + state_.currentLevelName +
                "\nSelected start: " + state_.selectedLevelName
            );
        }
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

    levelSelector->setVisible(isMainMenu);
    startLevelButton->setVisible(isMainMenu);
    mainMenuButton->setVisible(!isMainMenu);

    refreshMenuContext();
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
    applyModeLayout();
}

void Menu::openPauseMenu()
{
    mode_ = MenuMode::Pause;
    isOpen_ = true;
    closePopups();
    menuAnimationClock.restart();
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

    refreshMenuContext();
    background.update(deltaTime);
    background.draw(window);
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
