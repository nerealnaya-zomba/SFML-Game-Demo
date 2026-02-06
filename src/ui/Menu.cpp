#include<Menu.h>


Menu::Menu(sf::Font& font, sf::RenderWindow& window, sf::RectangleShape& mouseRect)
{
    //External references
    mouseRect_m = &mouseRect;
    window_m = &window;
    
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

    //Exit dialogue init
    exitDialogue = new AskDialogue(sf::Vector2f(WINDOW_WIDTH/2,WINDOW_HEIGHT/2),sf::Vector2f(350,150),"Quit the game?",font,sf::Color::Black,*mouseRect_m,*window_m);
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

void Menu::fillUpWindowWithPlayButton(sf::RenderWindow &window, sf::Color gameBackGroundColor)
{
    for (int i = 0; i < WINDOW_FPS*2; i++)
    {
        playButton->setScale({playButton->getScale().x+0.05f, playButton->getScale().y+0.15f});
        if(playButtonText->getFillColor().a>0)
        {
            playButtonText->setFillColor({playButton->getFillColor().r, playButton->getFillColor().g, playButton->getFillColor().b, static_cast<uint8_t>(playButton->getFillColor().a-1)});
        }
        
        //Smoothly change button's color to game's background color
        sf::Color locButton = playButton->getFillColor();
        if(locButton.r != gameBackGroundColor.r)
        {
            if(locButton.r > gameBackGroundColor.r)
            {
                locButton.r -= 1;
            }
            else
            {
                locButton.r += 1;
            }
        }
        if(locButton.g != gameBackGroundColor.g)
        {
            if(locButton.g > gameBackGroundColor.g)
            {
                locButton.g -= 1;
            }
            else
            {
                locButton.g += 1;
            }
        }
        if(locButton.b != gameBackGroundColor.b)
        {
            if(locButton.b > gameBackGroundColor.b)
            {
                locButton.b -= 1;
            }
            else
            {
                locButton.b += 1;
            }
        }
        playButton->setFillColor(locButton);

        //std::cout << "1: " << (int)locButton.r << " 2: " << (int)locButton.g << " 3: " << (int)locButton.b << std::endl;
        //std::cout << "1: " << (int)gameBackGroundColor.r << " 2: " << (int)gameBackGroundColor.g << " 3: " << (int)gameBackGroundColor.b << std::endl;

        //draw
        window.draw(*playButton);
        window.draw(*playButtonText);
        window.display();
    }
}

void Menu::fillUpWindowWithExitButton(sf::RenderWindow &window)
{
    for (int i = 0; i < WINDOW_FPS*1.3; i++)
    {
        exitButton->setScale({exitButton->getScale().x+0.05f, exitButton->getScale().y+0.15f});
        if(exitButtonText->getFillColor().a>0)
        {
            exitButtonText->setFillColor({exitButton->getFillColor().r, exitButton->getFillColor().g, exitButton->getFillColor().b, exitButton->getFillColor().a-1});
        }

        window.draw(*exitButton);
        window.draw(*exitButtonText);
        window.display();
    }
}

void Menu::smoothlyReturnPreviousVariablesAndDraw(sf::RenderWindow& window)
{
    playButtonText->setFillColor(sf::Color::White);
    for (int i = 0; i < WINDOW_FPS*2; i++)
    {
        if(playButton->getScale().x>1 && playButton->getScale().y > 1)
        {
            playButton->setScale({playButton->getScale().x-0.05f, playButton->getScale().y-0.15f});
        }
        if(playButtonText->getFillColor().a<255)
        {
            playButtonText->setFillColor({playButton->getFillColor().r, playButton->getFillColor().g, playButton->getFillColor().b, playButton->getFillColor().a+1});
        }
        
        //Smoothly change button's color to the black
        sf::Color locButton = playButton->getFillColor();
        if(locButton.r != 0)
        {
            if(locButton.r > 0)
            {
                locButton.r -= 1;
            }
            
        }
        if(locButton.g != 0)
        {
            if(locButton.g > 0)
            {
                locButton.g -= 1;
            }
            
        }
        if(locButton.b != 0)
        {
            if(locButton.b > 0)
            {
                locButton.b -= 1;
            }
        }
        playButton->setFillColor(locButton);

        //draw
        window.clear(sf::Color::White);
        window.draw(*playButton);
        window.draw(*playButtonText);
        window.display();
    }
}

void Menu::menuDraw(sf::RenderWindow& window)
{
    window.draw(*playButton);
    window.draw(*exitButton);
    window.draw(*playButtonText);
    window.draw(*exitButtonText);
    if(exitDialogue->isCalled) exitDialogue->draw(window);
    window.display();
}
