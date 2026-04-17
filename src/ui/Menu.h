#pragma once

#include <AskDialogue.h>
#include <Defines.h>
#include <MenuBackground.h>
#include <NotificationFeed.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

inline const tgui::String BASE_PLAY_BUTTON_TEXT = "Continue";
inline const tgui::String BASE_RESUME_BUTTON_TEXT = "Resume";
inline const tgui::String BASE_START_BUTTON_TEXT = "Start Level";
inline const tgui::String BASE_RESTART_BUTTON_TEXT = "Restart Current Level";
inline const tgui::String BASE_RETURN_TO_MAIN_MENU_BUTTON_TEXT = "Main Menu";
inline const tgui::String BASE_CONTROLS_BUTTON_TEXT = "Controls";
inline const tgui::String BASE_EXIT_BUTTON_TEXT = "Exit";
inline const tgui::String BASE_SETTINGS_BUTTON_TEXT = "Settings";
inline constexpr unsigned int BASE_MENU_BUTTONS_CHARACTER_SIZE = 25;

class GameData;

enum class MenuMode
{
    Main,
    Pause
};

struct MenuState
{
    std::vector<std::string> availableLevels{};
    std::map<std::string, std::string> levelDisplayNames{};
    std::string currentLevelName{};
    std::string selectedLevelName{};
    bool canContinue = false;
    bool canRestartLevel = false;
};

struct MenuCallbacks
{
    std::function<void()> onResumeGame = []() {};
    std::function<bool(const std::string&)> onStartSelectedLevel = [](const std::string&) { return false; };
    std::function<bool()> onRestartCurrentLevel = []() { return false; };
    std::function<bool()> onResetProgress = []() { return false; };
    std::function<bool(bool)> onSetFullscreen = [](bool) { return false; };
    std::function<void(std::string, std::string, NotificationTone)> onNotify =
        [](std::string, std::string, NotificationTone) {};
    std::function<void()> onReturnToMainMenu = []() {};
    std::function<void()> onExitGame = []() {};
};

class Menu
{
public:
    Menu(sf::RenderWindow& window, GameData& gameData);
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
    void attachWindow(sf::RenderWindow& window);

    bool isOpen() const;
    bool isMainMenu() const;
    bool isPauseMenu() const;
    const std::string& getSelectedLevelName() const;

private:
    enum class ButtonStyleRole
    {
        Primary,
        Secondary,
        Danger
    };

    MenuCallbacks callbacks_{};
    MenuState state_{};
    MenuMode mode_ = MenuMode::Main;
    bool isOpen_ = true;

    std::unique_ptr<AskDialogue> exitDialogue;
    std::unique_ptr<AskDialogue> resetProgressDialogue;

    tgui::Gui gui;
    tgui::Button::Ptr continueButton;
    tgui::Button::Ptr startLevelButton;
    tgui::Button::Ptr restartLevelButton;
    tgui::Button::Ptr mainMenuButton;
    tgui::Button::Ptr settingsButton;
    tgui::Button::Ptr controlsButton;
    tgui::Button::Ptr exitButton;
    tgui::Button::Ptr levelPrevButton;
    tgui::Button::Ptr levelNextButton;
    tgui::Button::Ptr randomLevelButton;
    tgui::ComboBox::Ptr levelSelector;
    tgui::Label::Ptr titleLabel;
    tgui::Label::Ptr subtitleLabel;
    tgui::Label::Ptr footerLabel;
    tgui::Label::Ptr selectionInfoLabel;
    tgui::Label::Ptr systemInfoLabel;
    tgui::Label::Ptr shortcutInfoLabel;
    tgui::ChildWindow::Ptr settingsWindow;
    tgui::ChildWindow::Ptr controlsWindow;
    tgui::Button::Ptr fullscreenToggleButton;
    tgui::Button::Ptr vsyncToggleButton;
    tgui::Button::Ptr particleToggleButton;
    tgui::Button::Ptr resetProgressButton;
    std::map<std::string, std::string> selectorLabelToLevelId_;
    std::map<std::string, std::string> levelIdToSelectorLabel_;

    sf::RenderWindow* window_m = nullptr;
    GameData* gameData_m = nullptr;
    MenuBackground background;
    sf::Clock menuAnimationClock;
    float menuVisualTime_ = 0.f;

    sf::RectangleShape panelShadow_;
    sf::RectangleShape panelFrame_;
    sf::RectangleShape panelInset_;
    sf::RectangleShape titleBand_;
    sf::RectangleShape dividerLine_;
    sf::RectangleShape verticalDivider_;
    sf::RectangleShape footerBand_;
    sf::RectangleShape selectorBand_;
    std::array<sf::RectangleShape, 3> infoCards_;
    std::array<sf::RectangleShape, 3> infoCardGlows_;
    std::array<sf::RectangleShape, 2> ornamentLines_;
    sf::CircleShape sigilOuterRing_;
    sf::CircleShape sigilInnerRing_;
    sf::RectangleShape sigilVerticalBar_;
    sf::RectangleShape sigilHorizontalBar_;
    std::array<sf::CircleShape, 2> emberNodes_;

    bool vsyncEnabled = false;
    bool fullscreenEnabled = false;
    int menuParticleCount = 200;
    float menuReveal_ = 1.f;

    void setupMainWidgets();
    tgui::Button::Ptr createMenuButton(const tgui::String& text, float y);
    void styleButton(const tgui::Button::Ptr& button, ButtonStyleRole role) const;
    void styleLabel(const tgui::Label::Ptr& label, bool isTitle) const;
    void styleInfoLabel(const tgui::Label::Ptr& label) const;
    void styleSelector() const;
    void styleChildWindow(const tgui::ChildWindow::Ptr& childWindow) const;
    void syncPreferencesFromGameData();
    void initializeLevelSelector();
    void initializeSettingsWindow();
    void initializeControlsWindow();
    std::string buildLevelSelectorLabel(const std::string& levelName) const;
    void refreshLevelSelector();
    void refreshMenuContext();
    void applyModeLayout();
    void applyModeTheme();
    void updateWidgetLayout();
    bool isBlockingPopupOpen() const;
    void syncPopupInteractivity();
    void updateDecorativeLayout();
    void drawDecorativeLayout(sf::RenderWindow& window);
    void closePopups();
    void selectAdjacentLevel(int direction);
    void selectRandomLevel();
    std::string buildSelectionInfoText() const;
    std::string buildSystemInfoText() const;
    std::string buildShortcutInfoText() const;

    void resumeButtonOnClick();
    void startSelectedLevelOnClick();
    void restartCurrentLevelOnClick();
    void returnToMainMenuOnClick();
    void settingsButtonOnClick();
    void controlsButtonOnClick();
    void toggleFullscreen();
    void toggleVsync();
    void toggleMenuParticles();
    void resetProgressButtonOnClick();
    void exitButtonOnClick();
};
