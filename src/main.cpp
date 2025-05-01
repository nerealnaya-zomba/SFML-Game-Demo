#include <sfml-headers.h>
#include <Defines.h>

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
    Menu menu(font);

    //Game
    Player player;
    


    //Main loop
    while (window.isOpen())
    {
        


        while (const std::optional event = window.pollEvent())
        {
            //Close window on "Close" button
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            //↓↓-----MAIN MENU SECTION-----↓↓
            if(isMainMenuCalled)
            {
                if(const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>())
                {
                    if(mouseButtonPressed->button == sf::Mouse::Button::Left)
                    {
                        moveRectToMouse(mouseRect,window);
                        if(mouseRect.getGlobalBounds().findIntersection(menu.playButton->getGlobalBounds()))
                        {

                            //Fill-up all window with button's area, then start game
                            for (int i = 0; i < WINDOW_FPS*2; i++)
                            {
                                menu.playButton->setScale({menu.playButton->getScale().x+0.05f, menu.playButton->getScale().y+0.15f});
                                if(menu.playButtonText->getFillColor().a>0)
                                {
                                    menu.playButtonText->setFillColor({menu.playButton->getFillColor().r, menu.playButton->getFillColor().g, menu.playButton->getFillColor().b, menu.playButton->getFillColor().a-1});
                                }
                                
                                //Smoothly change button's color to game's background color
                                sf::Color locButton = menu.playButton->getFillColor();
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
                                menu.playButton->setFillColor(locButton);

                                //std::cout << "1: " << (int)locButton.r << " 2: " << (int)locButton.g << " 3: " << (int)locButton.b << std::endl;
                                //std::cout << "1: " << (int)gameBackGroundColor.r << " 2: " << (int)gameBackGroundColor.g << " 3: " << (int)gameBackGroundColor.b << std::endl;

                                //draw
                                window.draw(*menu.playButton);
                                window.draw(*menu.playButtonText);
                                window.display();
                            }
                            /////////////////////////////////////////////////////////

                            isMainMenuCalled = false;
                        }
                        else if(mouseRect.getGlobalBounds().findIntersection(menu.exitButton->getGlobalBounds()))
                        {
                            //Fill-up all window with button's area, then close window
                            for (int i = 0; i < WINDOW_FPS*1.3; i++)
                            {
                                menu.exitButton->setScale({menu.exitButton->getScale().x+0.05f, menu.exitButton->getScale().y+0.15f});
                                if(menu.exitButtonText->getFillColor().a>0)
                                {
                                    menu.exitButtonText->setFillColor({menu.exitButton->getFillColor().r, menu.exitButton->getFillColor().g, menu.exitButton->getFillColor().b, menu.exitButton->getFillColor().a-1});
                                }

                                window.draw(*menu.exitButton);
                                window.draw(*menu.exitButtonText);
                                window.display();
                            }
                            window.close();
                            /////////////////////////////////////////////////////////
                        }
                    }
                }
                continue;
            }
            //↑↑-----MAIN MENU SECTION-----↑↑

            //Call main menu on 'Escape' button
            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if(keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                {
                    //Smoothly return previous variables on playButton and PlayButtonText
                    menu.playButtonText->setFillColor(sf::Color::White);
                    for (int i = 0; i < WINDOW_FPS*2; i++)
                    {
                        if(menu.playButton->getScale().x>1 && menu.playButton->getScale().y > 1)
                        {
                            menu.playButton->setScale({menu.playButton->getScale().x-0.05f, menu.playButton->getScale().y-0.15f});
                        }
                        if(menu.playButtonText->getFillColor().a<255)
                        {
                            menu.playButtonText->setFillColor({menu.playButton->getFillColor().r, menu.playButton->getFillColor().g, menu.playButton->getFillColor().b, menu.playButton->getFillColor().a+1});
                        }
                        
                        //Smoothly change button's color to the black
                        sf::Color locButton = menu.playButton->getFillColor();
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
                        menu.playButton->setFillColor(locButton);

                        //draw
                        window.clear(sf::Color::White);
                        window.draw(*menu.playButton);
                        window.draw(*menu.playButtonText);
                        window.display();
                    }
                    
                    ////////////////////////////////////////////////////////
                    isMainMenuCalled = true;
                }
            }
            
            
        }
        
        //Main menu
        if(isMainMenuCalled)
        {
            sizeUpRectangleOnHover(*menu.playButton,mouseRect,window, 0.03f , 0.02f);
            sizeUpRectangleOnHover(*menu.exitButton,mouseRect,window, 0.03f , 0.02f);

            menu.rainbowWindowClear(window, menuBackGroundColor);

            window.draw(*menu.playButton);
            window.draw(*menu.exitButton);
            window.draw(*menu.playButtonText);
            window.draw(*menu.exitButtonText);
            window.display();
            continue;
        }

        //Player control
        player.isIdle = true;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            player.isIdle = false;
            if(!player.isFalling)
            {
                player.switchToNextRunningSprite();
            }
            player.playerRectangle->setScale({-1.f,1.f});
            player.playerRectangle->move({-1.f,0.f});
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            player.isIdle = false;
            if(!player.isFalling)
            {
                player.switchToNextRunningSprite();
            }
            player.playerRectangle->setScale({1.f,1.f});
            player.playerRectangle->move({1.f,0.f});
        }

        //Player logic
            //Physical logic
                //If player is on the edge of down side of window
        if((player.playerRectangle->getPosition().y+player.playerRectangle->getSize().y/2-15)>=WINDOW_HEIGHT)
        {
            player.isFalling = false;
        }
        if(player.isFalling)
        {
            player.playerRectangle->move({0.f,1.f});
        }
        
            //Texture logic
        if(player.isIdle)
        {
            player.switchToNextIdleSprite();
        }
        if(player.isFalling)
        {  
            player.switchToNextFallingTexture();
        }
        
        
        try
        {
            window.clear(gameBackGroundColor);
            

            //Player drawing
            player.drawPlayer(window);


            window.display();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }



}
