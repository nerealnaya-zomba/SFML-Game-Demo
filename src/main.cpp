#include <Defines.h>
#include <nlohmann/json.hpp>
#include <sfml-headers.h>

#include <cstdlib>
#include <ctime>
#include <string>

namespace
{
const std::string kLevelFolder = "data/levelData";

void resetViewForMenu(sf::RenderWindow& window, sf::View& view)
{
    view.setCenter({WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f});
    view.setSize({WINDOW_WIDTH, WINDOW_HEIGHT});
    window.setView(view);
}
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    auto window = sf::RenderWindow(
        sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}),
        WINDOW_TITLE,
        (sf::Style::Titlebar | sf::Style::Close),
        sf::State::Fullscreen
    );
    window.setFramerateLimit(WINDOW_FPS);

    sf::Font font;
    if (!font.openFromFile("fonts/Roboto_Condensed-Black.ttf"))
    {
        std::cout << "Font error" << std::endl;
        return 0;
    }

    GameData gameData(&window, &font);
    sf::View view({0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT});
    const sf::Color gameBackGroundColor({0, 0, 0, 255});

    Menu menu(window);
    tgui::Font tguiFont("fonts/Roboto_Condensed-Black.ttf");
    menu.connectTGUIFont(tguiFont);

    GameCamera camera(view);
    GameLevelManager levelManager(gameData, camera, window, kLevelFolder);
    Player player(gameData, levelManager, camera, window);
    PlayerUI playerUI(player, camera, gameData);

    playerUI.addCooldownRect(
        player.getDashClock(),
        player.getDashCooldown(),
        gameData.satiro_dashTextures[0]
    );
    playerUI.addCooldownRect(
        player.getShootClock(),
        player.getShootCooldown(),
        gameData.bulletTextures[0]
    );
    playerUI.addCooldownRect(
        player.getPortalClock(),
        player.getPortalCooldown(),
        gameData.portalBlue8Textures[0]
    );

    levelManager.attachPlayer(player);
    levelManager.setPlayerPositionToBase();

    camera.attachGameLevelManager(levelManager);
    camera.attachPlayer(player);

    sf::Vector2f traderPosition = {800.f, 940.f};
    Trader trader(gameData, player, traderPosition);

    bool hasActiveRun = false;

    auto syncMenuState = [&]() {
        MenuState state;
        state.availableLevels = levelManager.getLevelNames();
        state.currentLevelName = levelManager.getCurrentLevelName();
        state.selectedLevelName = menu.getSelectedLevelName().empty()
            ? levelManager.getCurrentLevelName()
            : menu.getSelectedLevelName();
        state.canContinue = hasActiveRun;
        state.canRestartLevel = hasActiveRun;
        menu.setState(state);
    };

    syncMenuState();
    menu.openMainMenu();

    menu.setCallbacks(MenuCallbacks{
        .onResumeGame = [&]() {
            player.playFadeInAnimation();
        },
        .onStartSelectedLevel = [&](const std::string& levelName) {
            const bool changedLevel = levelManager.goToLevel(std::make_optional(levelName));
            if (changedLevel)
            {
                hasActiveRun = true;
                player.playFadeInAnimation();
                syncMenuState();
            }
            return changedLevel;
        },
        .onRestartCurrentLevel = [&]() {
            const bool restarted = hasActiveRun && levelManager.restartCurrentLevel();
            if (restarted)
            {
                player.playFadeInAnimation();
                syncMenuState();
            }
            return restarted;
        },
        .onReturnToMainMenu = [&]() {
            syncMenuState();
            resetViewForMenu(window, view);
        },
        .onExitGame = [&]() {
            window.close();
        }
    });

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (menu.isOpen())
            {
                menu.menuHandleEvents(*event);
                continue;
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                {
                    syncMenuState();
                    resetViewForMenu(window, view);
                    menu.openPauseMenu();
                    continue;
                }
            }

            if (!player.isCDMenuOpened())
            {
                trader.handleEvent(*event);
            }
            if (!trader.isShopOpened())
            {
                player.chooseDestinationMenuHandleEvents(*event);
            }
        }

        if (menu.isOpen())
        {
            window.clear(gameBackGroundColor);
            menu.menuDraw(window);
            continue;
        }

        player.updateControls();
        player.updatePhysics();
        player.checkGroundCollision(levelManager.getGroundRect());
        player.checkPlatformRectCollision(levelManager.getPlatformRects());
        player.moveBullets();

        levelManager.update();
        levelManager.updateEnemyManager();

        player.updateTextures();

        window.clear(gameBackGroundColor);

        levelManager.drawBackgrounds();
        levelManager.drawDecorations();
        levelManager.drawGrounds();
        levelManager.drawEnemyManager();

        trader.update();
        trader.draw(window);

        player.draw(window);
        player.drawBullets(window);

        levelManager.drawPlatforms();
        playerUI.update();
        player.chooseDestinationMenuUpdate();
        playerUI.draw(window);
        player.chooseDestinationMenuDraw(window);
        player.drawTransition();

        camera.update();
        window.setView(view);
        window.display();
    }
}
