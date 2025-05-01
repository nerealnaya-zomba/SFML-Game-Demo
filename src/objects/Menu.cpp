#include<Menu.h>


Menu::Menu(sf::Font& font)
{
    //Buttons init
    playButton = new sf::RectangleShape();
    playButtonText = new sf::Text(font,"Play");
    exitButton = new sf::RectangleShape();
    exitButtonText = new sf::Text(font, "Exit");

    playButton->setSize({200.f,75.f});
    playButton->setFillColor(sf::Color::Black);
    playButton->setOrigin({playButton->getSize().x/2,playButton->getSize().y/2});
    playButton->setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT-325+50});
    playButtonText->setFillColor(sf::Color::White);
    playButtonText->setOrigin({playButtonText->getGlobalBounds().size.x/2,playButtonText->getGlobalBounds().size.y/2});
    playButtonText->setPosition({playButton->getPosition().x, playButton->getPosition().y-2});
    exitButton->setSize({200.f,75.f});
    exitButton->setFillColor(sf::Color::Black);
    exitButton->setOrigin({exitButton->getSize().x/2,exitButton->getSize().y/2});
    exitButton->setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT-200+50});
    exitButtonText->setFillColor(sf::Color::White);
    exitButtonText->setOrigin({exitButtonText->getGlobalBounds().size.x/2,exitButtonText->getGlobalBounds().size.y/2});
    exitButtonText->setPosition({exitButton->getPosition().x, exitButton->getPosition().y-6});
}

Menu::~Menu()
{
    delete playButton, playButtonText, exitButton, exitButtonText;
}

void Menu::rainbowWindowClear(sf::RenderWindow& window, sf::Color& backgroundColor)
{
    srand(time(0));

    static bool rGoUp = true;
    static bool rGoDown = false;
    if(rGoUp)
    {
        backgroundColor.r+=rand()%2+1;

        if(backgroundColor.r>249)
        {
            rGoUp = false;
            rGoDown = true;
        }
    }
    else if(rGoDown)
    {
        backgroundColor.r-=rand()%2+1;

        if(backgroundColor.r<6)
        {
            rGoUp = true;
            rGoDown = false;
        }
    }
    /////////////////////////////////////////////////
    static bool gGoUp = true;
    static bool gGoDown = false;
    if(gGoUp)
    {
        backgroundColor.g+=rand()%2+1;

        if(backgroundColor.g>249)
        {
            gGoUp = false;
            gGoDown = true;
        }
    }
    else if(gGoDown)
    {
        backgroundColor.g-=rand()%2+1;

        if(backgroundColor.g<6)
        {
            gGoUp = true;
            gGoDown = false;
        }
    }

    static bool bGoUp = true;
    static bool bGoDown = false;
    if(bGoUp)
    {
        backgroundColor.b+=rand()%2+1;

        if(backgroundColor.b>249)
        {
            bGoUp = false;
            bGoDown = true;
        }
    }
    else if(bGoDown)
    {
        backgroundColor.b-=rand()%2+1;

        if(backgroundColor.b<6)
        {
            bGoUp = true;
            bGoDown = false;
        }
    }
    //std::cout << (int)backgroundColor.r << " " << (int)backgroundColor.g << " " << (int)backgroundColor.b << " " << std::endl;
    window.clear(backgroundColor);
}