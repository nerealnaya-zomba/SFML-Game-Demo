#include <sfml-headers.h>
#include <Defines.h>
#include<nlohmann/json.hpp>
int main()
{
    srand(time(NULL));
    //Window preferences
    auto window = sf::RenderWindow(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), WINDOW_TITLE, (sf::Style::Titlebar | sf::Style::Close), sf::State::Fullscreen);
    window.setFramerateLimit(WINDOW_FPS);
    
    //Loading font
    sf::Font font;
    if(font.openFromFile("fonts/Roboto_Condensed-Black.ttf")) 
        std::cout << "Font opened" << std::endl;
    else
    {
        std::cout << "Font error" << std::endl;
        return 0;
    }
    
    
    
    GameData gameData(&window,&font);

    //return 0; //REMOVELATER THIS TO STOP DEBUGGING gameTextures
    sf::View view({0,0},{WINDOW_WIDTH,WINDOW_HEIGHT});
    //Mouse
    sf::RectangleShape mouseRect({1.f,1.f});

    //Background
    sf::Color menuBackGroundColor({0u,0u,0u});
    sf::Color gameBackGroundColor({0,0,0,255});
    Background gameBackground;

    //Main menu
    Menu menu(font,window,mouseRect);
    
    
    
    
    
    //Game
    Player player(gameData);
    Ground ground(gameData);
    Decoration decoration(gameData);
    EnemyManager<Skeleton> enemyManager; //Skeleton manager
    
    
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
    platforms.addPlatform({WINDOW_WIDTH-300,WINDOW_HEIGHT-100},"Single-angled");
    platforms.addPlatform({WINDOW_WIDTH-400,WINDOW_HEIGHT-200},"Single-square");
    platforms.addPlatform({WINDOW_WIDTH-500,WINDOW_HEIGHT-280},"Single-flat");
    platforms.addPlatform({WINDOW_WIDTH-700,WINDOW_HEIGHT-380},"Double-horizontal-1");
    platforms.addPlatform({WINDOW_WIDTH-900,WINDOW_HEIGHT-480},"Double-horizontal-2");
    platforms.addPlatform({WINDOW_WIDTH-1050,WINDOW_HEIGHT-580},"Double-vertical");
    platforms.addPlatform({WINDOW_WIDTH-1300,WINDOW_HEIGHT-680},"Triple");
    platforms.addPlatform({WINDOW_WIDTH-1400,WINDOW_HEIGHT-800},"Quadruple");

    std::vector<Particle> particles;

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
                        if(mouseRect.getGlobalBounds().findIntersection(menu.playButton->getGlobalBounds()) && !menu.exitDialogue->isCalled)
                        {
                            menu.isMainMenuCalled = false;
                        }
                        //On exitButton fill-up all window with button's area, then close window
                        else if(mouseRect.getGlobalBounds().findIntersection(menu.exitButton->getGlobalBounds()) && !menu.exitDialogue->isCalled)
                        {
                            menu.exitDialogue->isCalled = true;
                        }
                        else if(menu.exitDialogue->isCalled)
                        {
                            menu.exitDialogue->checkAnswer();
                            if(menu.exitDialogue->answer_m == AskDialogue::Answer::Yes)
                            {
                                window.close();
                            }
                            else if(menu.exitDialogue->answer_m == AskDialogue::Answer::No)
                            {
                                menu.exitDialogue->isCalled = false;
                                menu.exitDialogue->answer_m = AskDialogue::Answer::NoAnswer;
                            }
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
                    //menu.smoothlyReturnPreviousVariablesAndDraw(window);

                    menu.isMainMenuCalled = true;
                }
            }
            
           //↑↑-----MAIN MENU-----↑↑ 

           //↓↓-----GAME-----↓↓
            // //    Jump on up arrow
            // bool isEnabledStoppingAutoRepeat_z = false;
            // bool isEnabledStoppingAutoRepeat_x = false;
            // bool isEnabledStoppingAutoRepeat_c = false;

            // static bool isPressed_x = false;
            // static bool isPressed_z = false;
            // static bool isPressed_c = false;

            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                // if(keyPressed->scancode == sf::Keyboard::Scancode::Z)
                // {
                //     if(!isPressed_z || !isEnabledStoppingAutoRepeat_z)
                //     {
                //         if(!player.isFalling)
                //         {
                //             player.jump();
                //         }
                //         isPressed_z = true;
                //     }
                // }
                
                // if(keyPressed->scancode == sf::Keyboard::Scancode::X)
                // {
                //     if(!isPressed_x || !isEnabledStoppingAutoRepeat_x)
                //     {
                //         std::cout << "Shoot" << std::endl;
                //         player.shoot(player.getSpriteScale().x > 0);
                //         isPressed_x = true;
                //     }
                // }
                
                // if(keyPressed->scancode == sf::Keyboard::Scancode::C)
                // {
                //     if(!isPressed_c || !isEnabledStoppingAutoRepeat_c)
                //     {
                //         std::cout << "Dash" << std::endl;
                //         player.dash();
                //         isPressed_c = true;
                //     }
                // }
                
                if(keyPressed->scancode == sf::Keyboard::Scancode::Num1)
                {
                    enemyManager.add(new Skeleton(gameData, window, ground, platforms, player, "white", 
                        sf::Vector2f(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y)));
                }
                
                if(keyPressed->scancode == sf::Keyboard::Scancode::Num2)
                {
                    enemyManager.add(new Skeleton(gameData, window, ground, platforms, player, "yellow", 
                        sf::Vector2f(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y)));
                }
            }

            // if(const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
            // {
            //     if(keyReleased->scancode == sf::Keyboard::Scancode::X)
            //     {
            //         isPressed_x = false;
            //     }
                
            //     if(keyReleased->scancode == sf::Keyboard::Scancode::Z)
            //     {
            //         isPressed_z = false;
            //     }
                
            //     if(keyReleased->scancode == sf::Keyboard::Scancode::C)
            //     {
            //         isPressed_c = false;
            //     }
            // }
            //↑↑-----GAME-----↑↑
        }
        //Main menu drawing
        if(menu.isMainMenuCalled)
        {
            //Dont size up play and exit buttons if exit dialogue window called
            if(menu.exitDialogue->isCalled == false)
            {
                sizeUpRectangleOnHover(*menu.playButton,*menu.playButtonText,mouseRect,window, 0.03f , 0.02f);
                sizeUpRectangleOnHover(*menu.exitButton,*menu.exitButtonText,mouseRect,window, 0.03f , 0.02f);
            }
            
            
            

            menu.rainbowWindowClear(window, menuBackGroundColor);

            
            menu.menuDraw(window);
            
            
            continue;
        }
        
        //Enemy logic
            //Skeleton AI
        enemyManager.updateAI_all();
            //Skeleton physics
        enemyManager.updatePhysics_all();
        
        //Contol logic
            //Player contol
        player.updateControls();
            //Enemy contol
        enemyManager.updateControls_all();

        //Logic
            //Player logic
                //Physical logic
        player.updatePhysics();
        player.checkGroundCollision(ground.getRect());
        player.checkRectCollision(platforms.getRects());
        player.moveBullets();

        //Texture update
        enemyManager.updateTextures_all();
        player.updateTextures();
        decoration.updateTextures();
        
        //Temporary control for exit
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Grave))
        {
            window.close();
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3))
        {
                for (int i = 0; i < 1; i++) {
                    particles.emplace_back(
                        sf::Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2),
                        sf::Vector2f(rand() % 300 - 150, rand() % -300 - 150),
                        sf::Vector2f(rand() % 120 - 60, rand() % 120 - 60),
                        sf::Color(255, 0, 0),
                        2.0f,
                        150.0f,
                        0.8f,
                        2.0f
                    );
                }
        }
        
        for (auto& particle : particles) { //REMOVELATER particles
            particle.update();
        }
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return !p.getIsAlive(); }),
            particles.end()
        );
    
        //Drawing

            //Background drawing
        window.clear(gameBackGroundColor);
        gameBackground.drawBackground(window);

            //Decorations drawing
        decoration.draw(window);
        
            //Ground drawing
        ground.draw(window,WINDOW_HEIGHT-39.f);

            //Enemy drawing
        enemyManager.draw_all();
        
            //Player drawing
        player.draw(window);
        player.drawBullets(window);
            //Game objects drawing
        platforms.draw(window);
        for (auto& particle : particles) { //REMOVELATER particle
            particle.draw(window);
        }
        

        window.display();
        
        
    }



}
