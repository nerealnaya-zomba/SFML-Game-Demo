#include <CampaignProgress.h>
#include <Defines.h>
#include <DeveloperOverlay.h>
#include <DeathScreen.h>
#include <NotificationFeed.h>
#include <PlayerUI.h>
#include <ScreenTransition.h>
#include <nlohmann/json.hpp>
#include <sfml-headers.h>

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <cmath>
#include <fstream>
#include <future>
#include <optional>
#include <string>
#include <vector>

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
    const sf::View& defaultView = window.getDefaultView();
    view.setCenter(defaultView.getCenter());
    view.setSize(defaultView.getSize());
    window.setView(view);
}

sf::Vector2u chooseInitialWindowSize()
{
    const sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    const auto clampDimension = [](unsigned int requested, unsigned int desktop, unsigned int minimum) {
        if (desktop <= minimum)
        {
            return desktop;
        }

        return std::clamp(requested, minimum, desktop);
    };

    const unsigned int requestedWidth = static_cast<unsigned int>(std::round(static_cast<float>(desktopMode.size.x) * 0.88f));
    const unsigned int requestedHeight = static_cast<unsigned int>(std::round(static_cast<float>(desktopMode.size.y) * 0.86f));

    return {
        clampDimension(std::min(requestedWidth, WINDOW_WIDTH), desktopMode.size.x, 1280u),
        clampDimension(std::min(requestedHeight, WINDOW_HEIGHT), desktopMode.size.y, 720u)
    };
}

bool loadInitialFullscreenPreference()
{
    std::ifstream input("data/launchSettings.json");
    if (!input.is_open())
    {
        return false;
    }

    try
    {
        const nlohmann::json document = nlohmann::json::parse(input);
        const nlohmann::json videoSettings = document.value("video", nlohmann::json::object());
        return videoSettings.value(
            "fullscreenEnabled",
            document.value("fullscreenEnabled", false)
        );
    }
    catch (const nlohmann::json::exception&)
    {
        return false;
    }
}

std::optional<std::string> parseRequestedLevelIdentifier(int argc, char** argv)
{
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if ((argument == "--level" || argument == "--force-level") && index + 1 < argc)
        {
            return std::string(argv[index + 1]);
        }
    }

    return std::nullopt;
}

const char* getAshDensityLabel(const int particleCount)
{
    if (particleCount <= 120)
    {
        return "Low";
    }

    if (particleCount >= 260)
    {
        return "High";
    }

    return "Medium";
}
}

int main(int argc, char** argv)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    const std::optional<std::string> requestedLevelIdentifier = parseRequestedLevelIdentifier(argc, argv);
    const bool initialFullscreenEnabled = loadInitialFullscreenPreference();

    sf::RenderWindow window;
    if (initialFullscreenEnabled)
    {
        window.create(
            sf::VideoMode::getDesktopMode(),
            WINDOW_TITLE,
            sf::State::Fullscreen
        );
    }
    else
    {
        window.create(
            sf::VideoMode(chooseInitialWindowSize()),
            WINDOW_TITLE,
            (sf::Style::Titlebar | sf::Style::Close),
            sf::State::Windowed
        );
    }
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
            const sf::Event& currentEvent = *event;
            if (currentEvent.is<sf::Event::Closed>())
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

    window.setVerticalSyncEnabled(gameData->isVsyncEnabled());
    window.setFramerateLimit(gameData->isVsyncEnabled() ? 0u : WINDOW_FPS);

    Menu menu(window, *gameData);
    NotificationFeed notificationFeed(font);
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
    DeveloperOverlay developerOverlay(window);
    auto isTraderActive = [&]() {
        return levelManager.getCurrentLevelName() == "level1.json";
    };

    bool hasActiveRun = false;
    DeathFlowState deathFlowState = DeathFlowState::Inactive;

    auto pushNotification = [&](std::string title, std::string body, const NotificationTone tone = NotificationTone::Info) {
        notificationFeed.push(std::move(title), std::move(body), tone);
    };
    levelManager.setNotificationSink(pushNotification);

    auto syncMenuState = [&]() {
        MenuState state;
        state.availableLevels = player.getUnlockedLevelNames(levelManager.getLevelNames());
        state.levelDisplayNames = levelManager.getLevelDisplayNames();
        state.currentLevelName = levelManager.getCurrentLevelName();
        state.selectedLevelName = menu.getSelectedLevelName().empty()
            ? levelManager.getCurrentLevelName()
            : menu.getSelectedLevelName();
        state.canContinue = hasActiveRun;
        state.canRestartLevel = hasActiveRun;
        menu.setState(state);
    };

    auto startLevelByNameInternal = [&](const std::string& levelName, const bool ignoreUnlocks) {
        const bool changedLevel = levelManager.goToLevel(std::make_optional(levelName), ignoreUnlocks);
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
            pushNotification(
                "Gate opened",
                levelManager.getLevelDisplayName(levelName) + " awaits.",
                NotificationTone::Success
            );
        }
        else
        {
            pushNotification(
                "Gate sealed",
                "That realm cannot be opened right now.",
                NotificationTone::Warning
            );
        }
        return changedLevel;
    };

    auto startLevelByName = [&](const std::string& levelName) {
        return startLevelByNameInternal(levelName, false);
    };

    syncMenuState();
    menu.openMainMenu();
    if (requestedLevelIdentifier.has_value() && startLevelByNameInternal(*requestedLevelIdentifier, true))
    {
        menu.close();
    }

    developerOverlay.setActions(DeveloperOverlayActions{
        .onRestoreVitals = [&]() {
            player.restoreVitalResources();
            pushNotification("Ritual Console", "Vital resources restored.", NotificationTone::Success);
        },
        .onGrantGold = [&](int amount) {
            player.addGold(amount);
            pushNotification(
                "Ritual Console",
                "Granted " + std::to_string(amount) + " grave-gold.",
                NotificationTone::Success
            );
        },
        .onRestartLevel = [&]() {
            if (levelManager.restartCurrentLevel())
            {
                hasActiveRun = true;
                player.playFadeInAnimation();
                syncMenuState();
                pushNotification("Ritual Console", "Current realm reloaded.", NotificationTone::Info);
            }
        },
        .onRespawnAtSpawn = [&]() {
            if (levelManager.respawnPlayerAtCurrentSpawn())
            {
                hasActiveRun = true;
                player.playFadeInAnimation();
                syncMenuState();
                pushNotification("Ritual Console", "Respawned at the current checkpoint.", NotificationTone::Info);
            }
        },
        .onReturnToBase = [&]() {
            levelManager.setPlayerPositionToBase();
            player.restoreVitalResources();
            hasActiveRun = true;
            player.playFadeInAnimation();
            syncMenuState();
            pushNotification("Ritual Console", "Returned to the level base.", NotificationTone::Info);
        },
        .onGoToLevel = [&](const std::string& levelName) {
            return startLevelByName(levelName);
        },
        .onSetVsync = [&](bool enabled) {
            gameData->setVsyncEnabled(enabled);
            window.setVerticalSyncEnabled(enabled);
            window.setFramerateLimit(enabled ? 0u : WINDOW_FPS);
            pushNotification(
                "Ritual Console",
                enabled ? "VSync enabled." : "VSync disabled.",
                NotificationTone::Info
            );
        },
        .onSetMenuParticleCount = [&](int count) {
            gameData->setMenuParticleCount(count);
            pushNotification(
                "Ritual Console",
                std::string("Menu ash density: ") + getAshDensityLabel(count) + ".",
                NotificationTone::Info
            );
        }
    });

    menu.setCallbacks(MenuCallbacks{
        .onResumeGame = [&]() {
            player.playFadeInAnimation();
        },
        .onStartSelectedLevel = startLevelByName,
        .onRestartCurrentLevel = [&]() {
            const bool restarted = hasActiveRun && levelManager.restartCurrentLevel();
            if (restarted)
            {
                player.playFadeInAnimation();
                syncMenuState();
                pushNotification("Run reset", "The current realm has been restarted.", NotificationTone::Info);
            }
            return restarted;
        },
        .onResetProgress = [&]() {
            player.resetProgress();
            levelManager.clearDeathRecoveries();
            deathScreen.close();
            deathFlowState = DeathFlowState::Inactive;
            hasActiveRun = false;

            const std::vector<std::string> unlockedLevels = player.getUnlockedLevelNames(levelManager.getLevelNames());
            if (!unlockedLevels.empty() && levelManager.goToLevel(std::make_optional(unlockedLevels.front())))
            {
                levelManager.respawnPlayerAtCurrentSpawn();
            }

            resetViewForMenu(window, view);
            syncMenuState();
            pushNotification(
                "Progress reset",
                "Gold, relics and unlocked gates were returned to the first rite.",
                NotificationTone::Warning
            );
            return true;
        },
        .onSetFullscreen = [&](bool enabled) {
            try
            {
                const sf::Vector2u previousWindowSize = window.getSize();
                const sf::Vector2u targetWindowSize = enabled ? sf::Vector2u(sf::VideoMode::getDesktopMode().size)
                                                              : chooseInitialWindowSize();
                if (enabled)
                {
                    window.create(
                        sf::VideoMode::getDesktopMode(),
                        WINDOW_TITLE,
                        sf::State::Fullscreen
                    );
                }
                else
                {
                    window.create(
                        sf::VideoMode(targetWindowSize),
                        WINDOW_TITLE,
                        (sf::Style::Titlebar | sf::Style::Close),
                        sf::State::Windowed
                    );
                }
                window.setVerticalSyncEnabled(gameData->isVsyncEnabled());
                window.setFramerateLimit(gameData->isVsyncEnabled() ? 0u : WINDOW_FPS);
                menu.attachWindow(window);
                resetViewForMenu(window, view);

                if (!enabled && previousWindowSize != targetWindowSize)
                {
                    window.setPosition({60, 40});
                }

                return true;
            }
            catch (...)
            {
                return false;
            }
        },
        .onNotify = [&](std::string title, std::string body, const NotificationTone tone) {
            pushNotification(std::move(title), std::move(body), tone);
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

    sf::Clock frameClock;

    while (window.isOpen())
    {
        const sf::Time frameDelta = sf::seconds(std::min(frameClock.restart().asSeconds(), 0.05f));
        notificationFeed.update();

        while (const std::optional event = window.pollEvent())
        {
            const sf::Event& currentEvent = *event;
            if (currentEvent.is<sf::Event::Closed>())
            {
                window.close();
            }

            if (menu.isOpen())
            {
                menu.menuHandleEvents(currentEvent);
                continue;
            }

            if (developerOverlay.handleEvent(currentEvent))
            {
                continue;
            }

            if (deathScreen.isOpen())
            {
                if (const auto action = deathScreen.handleEvent(currentEvent))
                {
                    handleDeathScreenAction(*action);
                }
                continue;
            }

            if (const auto* keyPressed = currentEvent.getIf<sf::Event::KeyPressed>())
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

            if (playerUI.handleEvent(currentEvent))
            {
                continue;
            }

            player.chooseDestinationMenuHandleEvents(currentEvent);
            if (player.isCDMenuOpened())
            {
                continue;
            }

            const bool levelEventConsumed = levelManager.handleEvent(currentEvent);
            const bool levelModalOpen = levelManager.hasBlockingInteractiveModal();

            if (!levelEventConsumed && !levelModalOpen && isTraderActive())
            {
                trader.handleEvent(currentEvent);
            }
        }

        if (menu.isOpen())
        {
            window.clear(gameBackGroundColor);
            menu.menuDraw(window);
            notificationFeed.draw(window);
            window.display();
            continue;
        }

        developerOverlay.beginFrame(frameDelta);

        const bool deathSequenceActive =
            deathFlowState != DeathFlowState::Inactive ||
            player.isPlayingDieAnimation ||
            !player.isAlive;
        const bool deathChoiceActive = deathFlowState == DeathFlowState::AwaitingChoice;
        const bool overlayActive = developerOverlay.isOpen();

        if (!deathSequenceActive && !overlayActive)
        {
            levelManager.updatePlatforms();
            player.updateControls();
            player.updatePhysics();
            player.moveBullets();
        }

        if (!deathChoiceActive && !overlayActive)
        {
            levelManager.update();
            levelManager.updateEnemyManager();
            if (isTraderActive())
            {
                trader.update();
            }
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

        if (!deathSequenceActive && !overlayActive)
        {
            playerUI.update();
            player.chooseDestinationMenuUpdate();
        }

        window.clear(gameBackGroundColor);

        levelManager.draw();

        if (isTraderActive())
        {
            trader.draw(window);
        }

        levelManager.drawInteractiveOverlays();
        if (isTraderActive())
        {
            trader.drawOverlay(window);
        }
        playerUI.draw(window);
        player.chooseDestinationMenuDraw(window);
        player.drawTransition();
        deathScreen.draw();
        developerOverlay.draw(
            player,
            *gameData,
            camera,
            levelManager,
            hasActiveRun,
            deathSequenceActive
        );
        notificationFeed.draw(window);
        window.display();
    }
}
