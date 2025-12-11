#pragma once
#include <MainMenu.h>
#include<Defines.h>
#include<AskDialogue.h>


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

    //External references
    sf::RenderWindow* window_m;
    sf::RectangleShape* mouseRect_m;

    void rainbowWindowClear(sf::RenderWindow& window, sf::Color& backgroundColor);
    void fillUpWindowWithPlayButton(sf::RenderWindow& window, sf::Color gameBackGroundColor);
    void fillUpWindowWithExitButton(sf::RenderWindow& window);
    void smoothlyReturnPreviousVariablesAndDraw(sf::RenderWindow& window);
    void menuDraw(sf::RenderWindow& window);
    Menu(sf::Font& font, sf::RenderWindow& window, sf::RectangleShape& mouseRect);
    ~Menu();
};