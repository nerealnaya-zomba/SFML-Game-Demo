#include <sfml-headers.h>
#include <Defines.h>
int main()
{
    //Window preferences
    auto window = sf::RenderWindow(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), WINDOW_TITLE, (sf::Style::Titlebar | sf::Style::Close), sf::State::Fullscreen);
    window.setFramerateLimit(WINDOW_FPS);
    
    sf::View view({0,0},{WINDOW_WIDTH,WINDOW_HEIGHT});
    //Mouse
    sf::RectangleShape mouseRect({1.f,1.f});

    //Background
    sf::Color menuBackGroundColor({0u,0u,0u});
    sf::Color gameBackGroundColor({0,0,0,255});
    Background gameBackground;

    //Loading font
    sf::Font font;
    if(font.openFromFile("fonts/Roboto_Condensed-Black.ttf"))
        std::cout << "Font opened" << std::endl;
    else
        std::cout << "Font error" << std::endl;

    //Main menu
    Menu menu(font);
    std::vector<std::string> pathsss = find_files("images/","satiro-falling");
    for (auto &&i : pathsss)
    {
        std::cout<< "FIND FILES TEST: " << i << std::endl;
    }
    
    
    //Game
    Player player;
    Ground ground;

    Decoration decoration;
    sf::Color grassColor{0,80,0,255};
    float offset = -50.f;
    decoration.addDecoration("plant1",{100,1040+offset},{0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant2",{200,1055+offset},{0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant3",{300,1035+offset},{0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant1",{400,1040+offset},{-0.6f,0.3f}, grassColor);
    decoration.addDecoration("plant2",{500,1055+offset},{0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant3",{600,1035+offset},{-0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant1",{700,1058+offset},{0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant2",{800,1045+offset},{-0.6f,0.3f}, grassColor);
    decoration.addDecoration("plant3",{1000,1048+offset},{0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant2",{1100,1055+offset},{0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant3",{1200,1035+offset},{-0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant1",{1300,1058+offset},{0.6f,0.3f}, grassColor);
    decoration.addDecoration("plant2",{1400,1045+offset},{-0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant3",{1500,1048+offset},{0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant3",{1600,1035+offset},{-0.6f,0.3f}, grassColor);
    decoration.addDecoration("plant1",{1700,1058+offset},{0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant2",{1800,1045+offset},{-0.4f,0.3f}, grassColor);
    decoration.addDecoration("plant3",{1900,1048+offset},{0.4f,0.3f}, grassColor);
    decoration.addDecoration("cat",{WINDOW_WIDTH-900-500,WINDOW_HEIGHT-800+80},{1.f,1.f});
    
    decoration.addDecoration("plant4",{WINDOW_WIDTH-200-100,WINDOW_HEIGHT-100},{0.4f,0.4f});
    decoration.addDecoration("plant4",{WINDOW_WIDTH-200-60,WINDOW_HEIGHT-100},{-0.4f,0.4f});

    decoration.addDecoration("plant5",{WINDOW_WIDTH-750+200,WINDOW_HEIGHT-420},{0.4f,0.4f});
    decoration.addDecoration("plant6",{WINDOW_WIDTH-870+200,WINDOW_HEIGHT-420},{-0.4f,0.4f});



    Platform platforms;
    platforms.addPlatform({WINDOW_WIDTH-200-100,WINDOW_HEIGHT-100},"Single-angled");
    platforms.addPlatform({WINDOW_WIDTH-300-100,WINDOW_HEIGHT-200},"Single-square");
    platforms.addPlatform({WINDOW_WIDTH-400-100,WINDOW_HEIGHT-280},"Single-flat");
    platforms.addPlatform({WINDOW_WIDTH-500-200,WINDOW_HEIGHT-380},"Double-horizontal-1");
    platforms.addPlatform({WINDOW_WIDTH-600-300,WINDOW_HEIGHT-480},"Double-horizontal-2");
    platforms.addPlatform({WINDOW_WIDTH-700-350,WINDOW_HEIGHT-580},"Double-vertical");
    platforms.addPlatform({WINDOW_WIDTH-800-500,WINDOW_HEIGHT-680},"Triple");
    platforms.addPlatform({WINDOW_WIDTH-900-500,WINDOW_HEIGHT-800},"Quadruple");

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
                    view.setCenter({WINDOW_WIDTH/2,WINDOW_HEIGHT/2});
                    window.setView(view);
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
                if(keyPressed->scancode == sf::Keyboard::Scancode::Z)
                {
                    if(!player.isFalling)
                    {
                        player.jump();
                    }
                    
                }
                if(keyPressed->scancode == sf::Keyboard::Scancode::X)
                {
                    std::cout << "Shoot" << std::endl;
                    player.shoot(player.getSpriteScale().x>0 ? true : false);
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
        
        //Enemy logic

        
        
        //Player logic
            //Player control
        player.updateControls();
            //Physical logic
        player.updatePhysics();
        player.checkGroundCollision(ground.getRect());
        player.checkRectCollision(platforms.getRects());
        player.moveBullets();

        //Texture logic
        player.updateTextures();
        decoration.updateTextures();
        
        //Temporary control for exit
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Grave))
        {
            window.close();
        }
        
        
    
        //Drawing

        //Background drawing
        window.clear(gameBackGroundColor);
        gameBackground.drawBackground(window);

        //Decorations drawing
        decoration.draw(window);

        //Ground drawing
        ground.draw(window,WINDOW_HEIGHT-39.f);

        //Player drawing
        player.draw(window);
        player.drawBullets(window);
        //Game objects drawing
        platforms.draw(window);


        view.setCenter(player.playerRectangle_->getPosition());
        //window.setView(view);
        

        window.display();
  
        
        
        
    }



}
