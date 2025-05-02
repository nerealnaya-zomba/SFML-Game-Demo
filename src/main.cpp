#include <sfml-headers.h>
#include <Defines.h>

int main()
{
    //Window preferences
    auto window = sf::RenderWindow(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), WINDOW_TITLE, sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(WINDOW_FPS);


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
            //↓↓-----MAIN MENU-----↓↓
            if(menu.isMainMenuCalled)
            {
                if(const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>())
                {
                    
                    if(mouseButtonPressed->button == sf::Mouse::Button::Left)
                    {
                        moveRectToMouse(mouseRect,window);
                        //On playButton click fill-up all window with button's area, then start game
                        if(mouseRect.getGlobalBounds().findIntersection(menu.playButton->getGlobalBounds()))
                        {
                            menu.fillUpWindowWithPlayButton(window, gameBackGroundColor);

                            menu.isMainMenuCalled = false;
                        }
                        //On exitButton fill-up all window with button's area, then close window
                        else if(mouseRect.getGlobalBounds().findIntersection(menu.exitButton->getGlobalBounds()))
                        {
                            
                            menu.fillUpWindowWithExitButton(window);
                            window.close();
                        }
                    }
                }
                continue;
            }

            //Call main menu on 'Escape' button
            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if(keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                {
                    //Smoothly return previous variables on playButton and PlayButtonText
                    menu.smoothlyReturnPreviousVariablesAndDraw(window);

                    menu.isMainMenuCalled = true;
                }
            }
            
            
        }
        
        //Main menu drawing
        if(menu.isMainMenuCalled)
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
        //↑↑-----MAIN MENU-----↑↑

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
