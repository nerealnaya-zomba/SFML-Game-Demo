#include <sfml-headers.h>

int main()
{
    //Window preferences
    auto window = sf::RenderWindow(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), WINDOW_TITLE, sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(WINDOW_FPS);

    //Bools
    bool isMainMenuCalled = true;



    //Mouse
    sf::RectangleShape mouseRect({1.f,1.f});

    //Background
    sf::Color menuBackGroundColor({0u,0u,0u});
    sf::Color gameBackGroundColor({255,255,255,255});

    //Loading font
    sf::Font font;
    if(font.openFromFile("fonts/Roboto_Condensed-Black.ttf"))
        std::cout << "Font opened" << std::endl;
    else
        std::cout << "Font error" << std::endl;

    //Main menu
    sf::RectangleShape playButton(sf::Vector2f{200.f,75.f});
    playButton.setFillColor(sf::Color::Black);
    playButton.setOrigin({playButton.getSize().x/2,playButton.getSize().y/2});
    playButton.setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT-325+50});
    sf::Text playButtonText(font,"Play");
    playButtonText.setFillColor(sf::Color::White);
    playButtonText.setOrigin({playButtonText.getGlobalBounds().size.x/2,playButtonText.getGlobalBounds().size.y/2});
    playButtonText.setPosition({playButton.getPosition().x, playButton.getPosition().y-2});
    sf::RectangleShape exitButton(sf::Vector2f{200.f,75.f});
    exitButton.setFillColor(sf::Color::Black);
    exitButton.setOrigin({exitButton.getSize().x/2,exitButton.getSize().y/2});
    exitButton.setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT-200+50});
    sf::Text exitButtonText(font,"Exit");
    exitButtonText.setFillColor(sf::Color::White);
    exitButtonText.setOrigin({exitButtonText.getGlobalBounds().size.x/2,exitButtonText.getGlobalBounds().size.y/2});
    exitButtonText.setPosition({exitButton.getPosition().x, exitButton.getPosition().y-6});

    



    //Main loop
    while (window.isOpen())
    {
        


        while (const std::optional event = window.pollEvent())
        {
            if(isMainMenuCalled)
            {
                if(const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>())
                {
                    if(mouseButtonPressed->button == sf::Mouse::Button::Left)
                    {
                        moveRectToMouse(mouseRect,window);
                        if(mouseRect.getGlobalBounds().findIntersection(playButton.getGlobalBounds()))
                        {

                            //Fill-up all window with button's area, then start game
                            for (int i = 0; i < WINDOW_FPS*2; i++)
                            {
                                playButton.setScale({playButton.getScale().x+0.05f, playButton.getScale().y+0.15f});
                                if(playButtonText.getFillColor().a>0)
                                {
                                    playButtonText.setFillColor({playButton.getFillColor().r, playButton.getFillColor().g, playButton.getFillColor().b, playButton.getFillColor().a-1});
                                }
                                
                                //Smoothly change button's color to game's background color
                                sf::Color locButton = playButton.getFillColor();
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
                                playButton.setFillColor(locButton);

                                //std::cout << "1: " << (int)locButton.r << " 2: " << (int)locButton.g << " 3: " << (int)locButton.b << std::endl;
                                //std::cout << "1: " << (int)gameBackGroundColor.r << " 2: " << (int)gameBackGroundColor.g << " 3: " << (int)gameBackGroundColor.b << std::endl;

                                //draw
                                window.draw(playButton);
                                window.draw(playButtonText);
                                window.display();
                            }
                            /////////////////////////////////////////////////////////

                            isMainMenuCalled = false;
                        }
                        else if(mouseRect.getGlobalBounds().findIntersection(exitButton.getGlobalBounds()))
                        {
                            //Fill-up all window with button's area, then close window
                            for (int i = 0; i < WINDOW_FPS*1.3; i++)
                            {
                                exitButton.setScale({exitButton.getScale().x+0.05f, exitButton.getScale().y+0.15f});
                                if(exitButtonText.getFillColor().a>0)
                                {
                                    exitButtonText.setFillColor({exitButton.getFillColor().r, exitButton.getFillColor().g, exitButton.getFillColor().b, exitButton.getFillColor().a-1});
                                }

                                window.draw(exitButton);
                                window.draw(exitButtonText);
                                window.display();
                            }
                            window.close();
                            /////////////////////////////////////////////////////////
                        }
                    }
                }
            }
            //Close window on "Close" button
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }
        
        //Main menu
        if(isMainMenuCalled)
        {
            sizeUpRectangleOnHover(playButton,mouseRect,window, 0.03f , 0.02f);
            sizeUpRectangleOnHover(exitButton,mouseRect,window, 0.03f , 0.02f);

            rainbowWindowClear(window, menuBackGroundColor);

            window.draw(playButton);
            window.draw(exitButton);
            window.draw(playButtonText);
            window.draw(exitButtonText);
            window.display();
            continue;
        }

        try
        {
            window.clear(gameBackGroundColor);
            
            window.display();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }



}
