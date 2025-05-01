#include <MainMenu.h>
#include<Defines.h>


class Menu{
    public:
    //Bools
    bool isMainMenuCalled = true;

    //Butons
    sf::RectangleShape* playButton;
    sf::Text* playButtonText;
    sf::RectangleShape* exitButton;
    sf::Text* exitButtonText;

    void rainbowWindowClear(sf::RenderWindow& window, sf::Color& backgroundColor);

    Menu(sf::Font& font);
    ~Menu();
};