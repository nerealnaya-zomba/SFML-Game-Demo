#include <Defines.h>
#include <DeathScreen.h>
#include <ScreenTransition.h>
#include <nlohmann/json.hpp>
#include <sfml-headers.h>

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <future>
#include <string>

namespace
{
const std::string kLevelFolder = "data/levelData";

enum class DeathFlowState
{
    Inactive,
    FadingOut,
    AwaitingChoice
};

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

    auto loadingProgress = std::make_shared<LoadingProgress>();
    LoadingScreen loadingScreen(window, font);
    auto gameDataFuture = std::async(std::launch::async, [&font, loadingProgress]() {
        try
        {
            sf::Context loadingContext;
            return std::make_unique<GameData>(&font, loadingProgress);
        }
        catch (const std::exception& exception)
        {
            loadingProgress->fail(exception.what());
            throw;
        }
        catch (...)
        {
            loadingProgress->fail("Unknown error while opening the gate");
            throw;
        }
    });

    sf::Clock loadingFrameClock;
    sf::Clock loadingSceneClock;
    const sf::Time minimumLoadingSceneTime = sf::milliseconds(750);
    std::unique_ptr<GameData> gameData{};
    bool loadingTaskCompleted = false;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        const float deltaTime = std::min(loadingFrameClock.restart().asSeconds(), 0.05f);
        loadingScreen.update(deltaTime, loadingProgress->snapshot());
        loadingScreen.draw();

        if (!loadingTaskCompleted &&
            gameDataFuture.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        {
            continue;
        }

        if (!loadingTaskCompleted)
        {
            try
            {
                gameData = gameDataFuture.get();
                loadingTaskCompleted = true;
            }
            catch (const std::exception& exception)
            {
                std::cerr << "Loading error: " << exception.what() << std::endl;
                return 1;
            }
        }

        if (
            loadingSceneClock.getElapsedTime() >= minimumLoadingSceneTime &&
            loadingScreen.isReadyToClose()
        )
        {
            break;
        }
    }

    if (!window.isOpen())
    {
        return 0;
    }

    sf::View view({0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT});
    const sf::Color gameBackGroundColor({0, 0, 0, 255});

    Menu menu(window);
    DeathScreen deathScreen(window, font);
    tgui::Font tguiFont("fonts/Roboto_Condensed-Black.ttf");
    menu.connectTGUIFont(tguiFont);

    GameCamera camera(view);
    GameLevelManager levelManager(*gameData, camera, window, kLevelFolder);
    Player player(*gameData, levelManager, camera, window);
    PlayerUI playerUI(player, camera, *gameData);

    playerUI.addCooldownRect(
        player.getDashClock(),
        player.getDashCooldown(),
        gameData->satiro_dashTextures[0]
    );
    playerUI.addCooldownRect(
        player.getShootClock(),
        player.getShootCooldown(),
        gameData->bulletTextures[0]
    );
    playerUI.addCooldownRect(
        player.getPortalClock(),
        player.getPortalCooldown(),
        gameData->portalBlue8Textures[0]
    );

    levelManager.attachPlayer(player);
    levelManager.setPlayerPositionToBase();

    camera.attachGameLevelManager(levelManager);
    camera.attachPlayer(player);

    sf::Vector2f traderPosition = {800.f, 940.f};
    Trader trader(*gameData, player, traderPosition);

    bool hasActiveRun = false;
    DeathFlowState deathFlowState = DeathFlowState::Inactive;

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
                const bool playerReady = player.isAlive || levelManager.respawnPlayerAtCurrentSpawn();
                if (!playerReady)
                {
                    return false;
                }

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

    auto handleDeathScreenAction = [&](DeathScreenAction action) {
        deathScreen.close();
        deathFlowState = DeathFlowState::Inactive;

        if (action == DeathScreenAction::RestartLevel)
        {
            if (levelManager.restartCurrentLevel())
            {
                hasActiveRun = true;
                player.playFadeInAnimation();
                syncMenuState();
            }
            return;
        }

        hasActiveRun = false;
        syncMenuState();
        resetViewForMenu(window, view);
        menu.openMainMenu();
    };

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

            if (deathScreen.isOpen())
            {
                if (const auto action = deathScreen.handleEvent(*event))
                {
                    handleDeathScreenAction(*action);
                }
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

            if (
                deathFlowState != DeathFlowState::Inactive ||
                player.isPlayingDieAnimation ||
                !player.isAlive
            )
            {
                continue;
            }

            levelManager.handleEvent(*event);

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

        const bool deathSequenceActive =
            deathFlowState != DeathFlowState::Inactive ||
            player.isPlayingDieAnimation ||
            !player.isAlive;
        const bool deathChoiceActive = deathFlowState == DeathFlowState::AwaitingChoice;

        if (!deathSequenceActive)
        {
            player.updateControls();
            player.updatePhysics();
            player.checkGroundCollision(levelManager.getGroundRect());
            player.checkPlatformRectCollision(levelManager.getPlatformRects());
            player.moveBullets();
        }

        if (!deathChoiceActive)
        {
            levelManager.update();
            levelManager.updateEnemyManager();
            trader.update();
            camera.update();
        }
        window.setView(view);

        player.updateTextures();

        if (!player.isAlive && deathFlowState == DeathFlowState::Inactive)
        {
            const int lostGold = player.takeAllGold();
            levelManager.registerDeathRecovery(player.getFeetPosition(), lostGold);
            player.getScreenTransition().fadeOut();
            deathFlowState = DeathFlowState::FadingOut;
        }

        if (
            deathFlowState == DeathFlowState::FadingOut &&
            player.getScreenTransition().isFadeOutComplete()
        )
        {
            deathScreen.open();
            deathFlowState = DeathFlowState::AwaitingChoice;
        }

        if (deathScreen.isOpen())
        {
            deathScreen.update();
        }

        if (!deathSequenceActive)
        {
            playerUI.update();
            player.chooseDestinationMenuUpdate();
        }

        window.clear(gameBackGroundColor);

        levelManager.drawBackgrounds();
        levelManager.drawDecorations();
        levelManager.drawGrounds();
        levelManager.drawEnemyManager();

        trader.draw(window);

        player.draw(window);
        player.drawBullets(window);

        levelManager.drawPlatforms();
        playerUI.draw(window);
        player.chooseDestinationMenuDraw(window);
        player.drawTransition();
        deathScreen.draw();
        window.display();
    }
}
