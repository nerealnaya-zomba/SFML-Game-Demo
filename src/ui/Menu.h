#pragma once
#include <MainMenu.h>
#include<Defines.h>
#include<AskDialogue.h>
#include<TGUI/TGUI.hpp>
#include<TGUI/Backend/SFML-Graphics.hpp>
#include<TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>


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
    void fillUpWindowWithPlayButton(sf::RenderWindow& window, sf::Color gameBackGroundColor);
    void fillUpWindowWithExitButton(sf::RenderWindow& window);
    void smoothlyReturnPreviousVariablesAndDraw(sf::RenderWindow& window);
    void menuDraw(sf::RenderWindow& window);
    void menuHandleEvents(const sf::Event& ev);
    Menu(sf::Font& font, sf::RenderWindow& window, sf::RectangleShape& mouseRect);
    ~Menu();
};