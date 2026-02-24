#pragma once
#include <MainMenu.h>
#include<Defines.h>
#include<AskDialogue.h>
#include<MenuBackground.h>
#include<TGUI/TGUI.hpp>
#include<TGUI/Backend/SFML-Graphics.hpp>
#include<TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>

const tgui::String BASE_EXIT_BUTTON_TEXT = "Exit";
const tgui::String BASE_PLAY_BUTTON_TEXT = "Play";
const tgui::String BASE_SETTINGS_BUTTON_TEXT = "Settings";
const unsigned int BASE_MENU_BUTTONS_CHARACTER_SIZE = 25;

//Button color
    //Base
        // IDLE (обычное состояние)
        const sf::Color BASE_IDLE_COLOR(70, 130, 180);     // Стальной синий
        // HOVER (при наведении)
        const sf::Color BASE_HOVER_COLOR(100, 150, 200);    // Светло-стальной синий
        // CLICK (при нажатии)
        const sf::Color BASE_CLICK_COLOR(40, 90, 140);      // Темно-стальной синий
    //Text
        // IDLE (обычное состояние)
        const sf::Color BASE_TEXT_IDLE_COLOR(0, 0, 0);      // Стальной синий
        // HOVER (при наведении)
        const sf::Color BASE_TEXT_HOVER_COLOR(255, 255, 255);    // Светло-стальной синий
        // CLICK (при нажатии)
        const sf::Color BASE_TEXT_CLICK_COLOR(127, 127, 127);      // Темно-стальной синий


class Menu{
    public:
    //Bools
    bool isMainMenuCalled = true;

    AskDialogue* exitDialogue;
    //TGUI
        //Group
        tgui::Gui gui;
        //Buttons
        tgui::Button::Ptr settingsButton;
        tgui::Button::Ptr exitButton;
        tgui::Button::Ptr playButton;

    //External references
    sf::RenderWindow* window_m;
    sf::RectangleShape* mouseRect_m;

    void rainbowWindowClear(sf::RenderWindow& window, sf::Color& backgroundColor);
    void windowClear(sf::RenderWindow& window, sf::Color& backgroundColor);
    void fillUpWindowWithPlayButton(sf::RenderWindow& window, sf::Color gameBackGroundColor);
    void fillUpWindowWithExitButton(sf::RenderWindow& window);
    void smoothlyReturnPreviousVariablesAndDraw(sf::RenderWindow& window);
    void menuDraw(sf::RenderWindow& window);
    void menuHandleEvents(const sf::Event& ev);
    Menu(sf::Font& font, sf::RenderWindow& window, sf::RectangleShape& mouseRect);
    ~Menu();

    void connectTGUIFont(tgui::Font& font);
private:
    MenuBackground background;

    void playButtonOnClick();
    void settingsButtonOnClick();
    void exitButtonOnClick();
};