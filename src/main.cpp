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
    tgui::Font tguiFont("fonts/Roboto_Condensed-Black.ttf");
    menu.connectTGUIFont(tguiFont);
    
    //Game
    GameCamera camera(view);                                                            // Camera
    GameLevelManager levelManager(gameData,camera,window,levelFolder);                  // Level manager
    Player player(gameData,levelManager,camera);                                        // Player
    PlayerUI PUI(player,camera);
    PUI.addCooldownRect(player.getDashClock(),player.getDashCooldown(),gameData.satiro_dashTextures[0]);
    PUI.addCooldownRect(player.getShootClock(),player.getShootCooldown(),gameData.bulletTextures[0]);
    PUI.addCooldownRect(player.getPortalClock(),player.getPortalCooldown(),gameData.portalBlue8Textures[0]);
    levelManager.attachPlayer(player);
    camera.attachGameLevelManager(levelManager);
    camera.attachPlayer(player);
    
    sf::Vector2f posss = {800.f,940.f};
    Trader trader(gameData, player, posss);

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
                menu.menuHandleEvents(*event);
                continue;
            }

            //Call main menu on 'Escape' button
            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if(keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                {
                    view.setCenter({WINDOW_WIDTH/2,WINDOW_HEIGHT/2});
                    view.setSize({WINDOW_WIDTH, WINDOW_HEIGHT});
                    window.setView(view);

                    menu.isMainMenuCalled = true;
                }
            }
            
           //↑↑-----MAIN MENU-----↑↑ 

           //↓↓-----GAME-----↓↓
            trader.handleEvent(*event);
            player.chooseDestinationMenuHandleEvents(*event);

            if(const auto* keyPressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                std::cout << "MouseX: " << mousePos.x << "  MouseY: " << mousePos.y << std::endl;
            }

            //↑↑-----GAME-----↑↑
        }
        //Main menu drawing
        if(menu.isMainMenuCalled)
        {
            // view.zoom(1.f);
            // view.setCenter({WINDOW_WIDTH/2,WINDOW_HEIGHT/2});
            // window.setView(view);
            sf::View lView = window.getView();
            std::cout << "X: " << lView.getSize().x << "Y: " << lView.getSize().y << std::endl;
            menu.menuDraw(window);
            
            continue;
        }
        
        //Contol logic
            //Player contol
        player.updateControls();
        
        //Logic 
            //Player logic
                //Physical logic
        player.updatePhysics();
        player.checkGroundCollision(levelManager.getGroundRect());
        player.checkPlatformRectCollision(levelManager.getPlatformRects());
        player.moveBullets();

        // Level objects updating
        levelManager.update();
        levelManager.updateEnemyManager();

        //Texture update
        player.updateTextures();
        
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
        
        trader.update();
        trader.draw(window);
            //Player drawing
        player.draw(window);
        player.drawBullets(window);

            //Front level drawing
        levelManager.drawPlatforms();
        PUI.update();
        player.chooseDestinationMenuUpdate();
        PUI.draw(window);
        player.chooseDestinationMenuDraw(window);
        

        //////////////////
        // Camera updating
        //////////////////
        camera.update();
        window.setView(view);
        //////////////////
        window.display();
        
        
    }
}
