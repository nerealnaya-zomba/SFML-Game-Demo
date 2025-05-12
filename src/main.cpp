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
            
           //↑↑-----MAIN MENU-----↑↑ 

           //↓↓-----GAME-----↓↓
           //Jump on up arrow
           if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if(keyPressed->scancode == sf::Keyboard::Scancode::Up)
                {
                    if(!player.isFalling)
                    {
                        std::cout << "Jump" << std::endl;
                        player.fallingSpeed = -5.f;
                    }
                    
                }
            }
            //↑↑-----GAME-----↑↑
        }
        
        //Main menu drawing
        if(menu.isMainMenuCalled)
        {
            sizeUpRectangleOnHover(*menu.playButton,mouseRect,window, 0.03f , 0.02f);
            sizeUpRectangleOnHover(*menu.exitButton,mouseRect,window, 0.03f , 0.02f);

            menu.rainbowWindowClear(window, menuBackGroundColor);

            menu.menuDraw(window);
            continue;
        }
        



        //Player logic
            //Player control
        player.updateControls();
            //Physical logic
        player.updatePhysics();
            //Texture logic
        player.updateTextures();
        
        

        
        
    
        //Drawing

        window.clear(gameBackGroundColor);
        

        //Player drawing
        player.drawPlayer(window);


        window.display();
  
        
        
        
    }



}
