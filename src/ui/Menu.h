#pragma once

#include <AskDialogue.h>
#include <Defines.h>
#include <MenuBackground.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

inline const tgui::String BASE_PLAY_BUTTON_TEXT = "Continue";
inline const tgui::String BASE_RESUME_BUTTON_TEXT = "Resume";
inline const tgui::String BASE_START_BUTTON_TEXT = "Start Selected Level";
inline const tgui::String BASE_RESTART_BUTTON_TEXT = "Restart Current Level";
inline const tgui::String BASE_RETURN_TO_MAIN_MENU_BUTTON_TEXT = "Main Menu";
inline const tgui::String BASE_CONTROLS_BUTTON_TEXT = "Controls";
inline const tgui::String BASE_EXIT_BUTTON_TEXT = "Exit";
inline const tgui::String BASE_SETTINGS_BUTTON_TEXT = "Settings";
inline constexpr unsigned int BASE_MENU_BUTTONS_CHARACTER_SIZE = 25;

inline const sf::Color BASE_IDLE_COLOR(70, 130, 180);
inline const sf::Color BASE_HOVER_COLOR(100, 150, 200);
inline const sf::Color BASE_CLICK_COLOR(40, 90, 140);
inline const sf::Color BASE_TEXT_IDLE_COLOR(0, 0, 0);
inline const sf::Color BASE_TEXT_HOVER_COLOR(255, 255, 255);
inline const sf::Color BASE_TEXT_CLICK_COLOR(127, 127, 127);

enum class MenuMode
{
    Main,
    Pause
};

struct MenuState
{
    std::vector<std::string> availableLevels{};
    std::string currentLevelName{};
    std::string selectedLevelName{"level1.json"};
    bool canContinue = false;
    bool canRestartLevel = false;
};

struct MenuCallbacks
{
    std::function<void()> onResumeGame = []() {};
    std::function<bool(const std::string&)> onStartSelectedLevel = [](const std::string&) { return false; };
    std::function<bool()> onRestartCurrentLevel = []() { return false; };
    std::function<void()> onReturnToMainMenu = []() {};
    std::function<void()> onExitGame = []() {};
};

class Menu
{
public:
    explicit Menu(sf::RenderWindow& window);
    ~Menu();

    void menuDraw(sf::RenderWindow& window);
    void menuHandleEvents(const sf::Event& ev);

    void connectTGUIFont(tgui::Font& font);
    void setCallbacks(MenuCallbacks callbacks);
    void setState(const MenuState& state);
    void setAvailableLevels(const std::vector<std::string>& levels);
    void setCurrentLevelName(const std::string& levelName);
    void setSelectedLevelName(const std::string& levelName);
    void setCanContinue(bool enabled);
    void setCanRestartLevel(bool enabled);

    void openMainMenu();
    void openPauseMenu();
    void close();

    bool isOpen() const;
    bool isMainMenu() const;
    bool isPauseMenu() const;
    const std::string& getSelectedLevelName() const;

private:
    MenuCallbacks callbacks_{};
    MenuState state_{};
    MenuMode mode_ = MenuMode::Main;
    bool isOpen_ = true;

    std::unique_ptr<AskDialogue> exitDialogue;

    tgui::Gui gui;
    tgui::Button::Ptr continueButton;
    tgui::Button::Ptr startLevelButton;
    tgui::Button::Ptr restartLevelButton;
    tgui::Button::Ptr mainMenuButton;
    tgui::Button::Ptr settingsButton;
    tgui::Button::Ptr controlsButton;
    tgui::Button::Ptr exitButton;
    tgui::ComboBox::Ptr levelSelector;
    tgui::Label::Ptr titleLabel;
    tgui::Label::Ptr subtitleLabel;
    tgui::ChildWindow::Ptr settingsWindow;
    tgui::ChildWindow::Ptr controlsWindow;
    tgui::Button::Ptr vsyncToggleButton;
    tgui::Button::Ptr particleToggleButton;

    sf::RenderWindow* window_m = nullptr;
    MenuBackground background;
    sf::Clock menuAnimationClock;

    bool vsyncEnabled = false;
    int menuParticleCount = 200;

    void setupMainWidgets();
    tgui::Button::Ptr createMenuButton(const tgui::String& text, float y);
    void styleButton(const tgui::Button::Ptr& button) const;
    void initializeLevelSelector();
    void initializeSettingsWindow();
    void initializeControlsWindow();
    void refreshLevelSelector();
    void refreshMenuContext();
    void applyModeLayout();
    void closePopups();

    void resumeButtonOnClick();
    void startSelectedLevelOnClick();
    void restartCurrentLevelOnClick();
    void returnToMainMenuOnClick();
    void settingsButtonOnClick();
    void controlsButtonOnClick();
    void toggleVsync();
    void toggleMenuParticles();
    void exitButtonOnClick();
};
