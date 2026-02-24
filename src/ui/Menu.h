#pragma once
#include <MainMenu.h>
#include<Defines.h>
#include<AskDialogue.h>
#include<TGUI/TGUI.hpp>
#include<TGUI/Backend/SFML-Graphics.hpp>
#include<TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>

const tgui::String BASE_EXIT_BUTTON_TEXT = "Exit";
const tgui::String BASE_PLAY_BUTTON_TEXT = "Play";
const tgui::String BASE_SETTINGS_BUTTON_TEXT = "Settings";
const unsigned int BASE_MENU_BUTTONS_CHARACTER_SIZE = 25;

class Menu{
    public:
    //Bools
    bool isMainMenuCalled = true;

    //Butons
    sf::RectangleShape* playButton;
    sf::Text* playButtonText;
    sf::RectangleShape* exitButton;
    sf::Text* exitButtonText;
    
    AskDialogue* exitDialogue;
    //TGUI
        //Group
        tgui::Gui gui;
        //Buttons
        tgui::Button::Ptr settingsButton;
        tgui::Button::Ptr exitButton1;
        tgui::Button::Ptr playButton1;

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
    void playButtonOnClick();
    void settingsButtonOnClick();
    void exitButtonOnClick();
};