#include <sfml-headers.h>
#include <Defines.h>
#include<nlohmann/json.hpp>

const std::string levelFolder = "data/levelData";

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

    sf::View view({0,0},{WINDOW_WIDTH,WINDOW_HEIGHT});
    //Mouse
    sf::RectangleShape mouseRect({1.f,1.f});

    //Background vars
    sf::Color menuBackGroundColor({0u,0u,0u});
    sf::Color gameBackGroundColor({0,0,0,255});
    
    
    //Main menu
    Menu menu(font,window,mouseRect);                                                   // Menu
    
    //Game
    GameCamera camera(view);                                                            // Camera
    std::shared_ptr<Player> player;                                                     // Player   
    GameLevelManager levelManager(gameData,camera,player,window,levelFolder);                  // Level manager
    player = std::make_shared<Player>(gameData,levelManager);   
    camera.attachGameLevelManager(levelManager);
    camera.attachPlayer(*player);
    

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

            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                

            }

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
        
        //Contol logic
            //Player contol
        player->updateControls();
        
        //Logic 
            //Player logic
                //Physical logic
        player->updatePhysics();
        player->checkGroundCollision(levelManager.getGroundRect());
        player->checkPlatformRectCollision(levelManager.getPlatformRects());
        player->moveBullets();

        // Level objects updating
        levelManager.update();
        levelManager.updateEnemyManager();

        //Texture update
        player->updateTextures();
        
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

        //Drawing
        window.clear(gameBackGroundColor);

            //Back level drawing
        levelManager.drawBackgrounds();
        levelManager.drawDecorations();
        levelManager.drawGrounds();
        levelManager.drawEnemyManager();
        
            //Player drawing
        player->draw(window);
        player->drawBullets(window);

            //Front level drawing
        levelManager.drawPlatforms();

        //////////////////
        // Camera updating
        //////////////////
        camera.update();
        window.setView(view);
        //////////////////
        window.display();
        
        
    }
}
