#include <sfml-headers.h>
#include <Defines.h>
#include <nlohmann/json.hpp>

const std::string levelFolder = "data/levelData";

int main()
{
    srand(time(NULL));
    
    // ===== 1. ИНИЦИАЛИЗАЦИЯ ОКНА =====
    auto window = sf::RenderWindow(
        sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), 
        WINDOW_TITLE, 
        (sf::Style::Titlebar | sf::Style::Close), 
        sf::State::Fullscreen
    );
    window.setFramerateLimit(WINDOW_FPS);
    
    // ===== 2. ЗАГРУЗКА РЕСУРСОВ =====
    sf::Font font;
    if(font.openFromFile("fonts/Roboto_Condensed-Black.ttf")) 
    {
        std::cout << "Font opened" << std::endl;
    }
    else
    {
        std::cout << "Font error" << std::endl;
        return 0;
    }
    
    // ===== 3. БАЗОВЫЕ ОБЪЕКТЫ =====
    GameData gameData(&window, &font);
    sf::View view({0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT});
    sf::RectangleShape mouseRect({1.f, 1.f});  // Для определения позиции мыши
    
    // Цвета фона
    sf::Color menuBackGroundColor({0u, 0u, 0u});
    sf::Color gameBackGroundColor({0, 0, 0, 255});
    
    // ===== 4. СОЗДАНИЕ МЕНЮ =====
    Menu menu(font, window, mouseRect);
    tgui::Font tguiFont("fonts/Roboto_Condensed-Black.ttf");
    menu.connectTGUIFont(tguiFont);
    
    // ===== 5. СОЗДАНИЕ ИГРОВЫХ ОБЪЕКТОВ =====
    // 5.1. Камера и уровни
    GameCamera camera(view);
    GameLevelManager levelManager(gameData, camera, window, levelFolder);
    
    // 5.2. Игрок и интерфейс
    Player player(gameData, levelManager, camera, window);
    PlayerUI PUI(player, camera);
    
    // 5.3. Настройка кулдаунов интерфейса
    PUI.addCooldownRect(
        player.getDashClock(), 
        player.getDashCooldown(), 
        gameData.satiro_dashTextures[0]
    );
    PUI.addCooldownRect(
        player.getShootClock(), 
        player.getShootCooldown(), 
        gameData.bulletTextures[0]
    );
    PUI.addCooldownRect(
        player.getPortalClock(), 
        player.getPortalCooldown(), 
        gameData.portalBlue8Textures[0]
    );
    
    // 5.4. Связывание объектов
    levelManager.attachPlayer(player);
    levelManager.setPlayerPositionToBase();
    
    camera.attachGameLevelManager(levelManager);
    camera.attachPlayer(player);
    
    menu.connectPlayer(player);
    
    // 5.5. Торговец и частицы
    sf::Vector2f posss = {800.f, 940.f};
    Trader trader(gameData, player, posss);
    
    std::vector<Particle> particles;

    // ===== 6. ГЛАВНЫЙ ИГРОВОЙ ЦИКЛ =====
    while (window.isOpen())
    {
        // ===== 6.1. ОБРАБОТКА СОБЫТИЙ =====
        while (const std::optional event = window.pollEvent())
        {
            // Закрытие окна
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            } 
            
            // ----- РЕЖИМ МЕНЮ -----
            if(menu.isMainMenuCalled)
            {
                menu.menuHandleEvents(*event);
                continue;
            }

            // ----- РЕЖИМ ИГРЫ -----
            // Вызов меню по Escape
            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if(keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                {
                    view.setCenter({WINDOW_WIDTH/2, WINDOW_HEIGHT/2});
                    view.setSize({WINDOW_WIDTH, WINDOW_HEIGHT});
                    window.setView(view);
                    menu.isMainMenuCalled = true;
                }
            }
            
            // Обработка событий торговца и меню выбора
            if(!player.isCDMenuOpened())
            {
                trader.handleEvent(*event);
            }
            if(!trader.isShopOpened())
            {
                player.chooseDestinationMenuHandleEvents(*event);
            }

            // Отладка: вывод позиции мыши при клике
            if(const auto* keyPressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                std::cout << "MouseX: " << mousePos.x << "  MouseY: " << mousePos.y << std::endl;
            }
        }
        
        // ===== 6.2. ОТРИСОВКА МЕНЮ =====
        if(menu.isMainMenuCalled)
        {
            sf::View lView = window.getView();
            std::cout << "View - X: " << lView.getSize().x << " Y: " << lView.getSize().y << std::endl;
            menu.menuDraw(window);
            continue;
        }
        
        // ===== 6.3. ИГРОВАЯ ЛОГИКА =====
        // Управление игроком
        player.updateControls();
        
        // Физика и коллизии
        player.updatePhysics();
        player.checkGroundCollision(levelManager.getGroundRect());
        player.checkPlatformRectCollision(levelManager.getPlatformRects());
        player.moveBullets();

        // Обновление уровней и врагов
        levelManager.update();
        levelManager.updateEnemyManager();

        // Обновление текстур игрока
        player.updateTextures();
        
        // ===== 6.4. ОТЛАДОЧНЫЕ ФУНКЦИИ =====
        // Выход по клавише ~ (Grave)
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Grave))
        {
            window.close();
        }
        
        // Тестовое создание частиц по клавише 3
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3))
        {
            for (int i = 0; i < 1; i++) 
            {
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

        // ===== 6.5. ОТРИСОВКА ИГРЫ =====
        window.clear(gameBackGroundColor);

        // Задний план
        levelManager.drawBackgrounds();
        levelManager.drawDecorations();
        levelManager.drawGrounds();
        levelManager.drawEnemyManager();
        
        // Торговец
        trader.update();
        trader.draw(window);
        
        // Игрок и его пули
        player.draw(window);
        player.drawBullets(window);

        // Передний план и интерфейс
        levelManager.drawPlatforms();
        PUI.update();
        player.chooseDestinationMenuUpdate();
        PUI.draw(window);
        player.chooseDestinationMenuDraw(window);
        player.drawTransition();

        // ===== 6.6. ОБНОВЛЕНИЕ КАМЕРЫ =====
        camera.update();
        window.setView(view);
        
        // ===== 6.7. ВЫВОД НА ЭКРАН =====
        window.display();
    }
}