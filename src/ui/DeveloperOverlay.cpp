#include <DeveloperOverlay.h>

#include <GameCamera.h>
#include <GameData.h>
#include <GameLevel.h>
#include <Player.h>
#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
float cooldownRatio(const sf::Clock& clock, int cooldownMs)
{
    if (cooldownMs <= 0)
    {
        return 1.f;
    }

    return std::clamp(
        clock.getElapsedTime().asMilliseconds() / static_cast<float>(cooldownMs),
        0.f,
        1.f
    );
}

void drawCooldownLine(const char* label, const sf::Clock& clock, int cooldownMs)
{
    const float ratio = cooldownRatio(clock, cooldownMs);
    const int elapsedMs = clock.getElapsedTime().asMilliseconds();

    ImGui::Text("%s", label);
    ImGui::SameLine(170.f);
    ImGui::ProgressBar(ratio, ImVec2(-1.f, 0.f));
    ImGui::TextDisabled("%d / %d ms", std::min(elapsedMs, cooldownMs), cooldownMs);
}

const char* getAshDensityLabel(int particleCount)
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

DeveloperOverlay::DeveloperOverlay(sf::RenderWindow& window)
    : window_(&window)
{
    if (!ImGui::SFML::Init(window))
    {
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }

    ImGui::GetIO().IniFilename = "data/imgui.ini";
    applyStyle();
    tryLoadGameFont();
}

DeveloperOverlay::~DeveloperOverlay()
{
    ImGui::SFML::Shutdown();
}

void DeveloperOverlay::setActions(DeveloperOverlayActions actions)
{
    actions_ = std::move(actions);
}

bool DeveloperOverlay::handleEvent(const sf::Event& event)
{
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == sf::Keyboard::Scancode::F1)
        {
            isOpen_ = !isOpen_;
            return true;
        }
    }

    if (!isOpen_)
    {
        return false;
    }

    ImGui::SFML::ProcessEvent(*window_, event);
    return isKeyboardOrMouseEvent(event);
}

void DeveloperOverlay::beginFrame(sf::Time deltaTime)
{
    recordFrameTime(deltaTime);

    if (!isOpen_)
    {
        return;
    }

    ImGui::SFML::Update(*window_, deltaTime);
}

void DeveloperOverlay::draw(
    Player& player,
    const GameData& gameData,
    const GameCamera& camera,
    const GameLevelManager& levelManager,
    const bool hasActiveRun,
    const bool deathSequenceActive
)
{
    if (!isOpen_)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(520.f, 620.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(24.f, 24.f), ImGuiCond_FirstUseEver);

    if (showImGuiDemo_)
    {
        ImGui::ShowDemoWindow(&showImGuiDemo_);
    }

    if (ImGui::Begin("Ritual Console", &isOpen_, ImGuiWindowFlags_NoCollapse))
    {
        const float fps = smoothedFrameMs_ > 0.001f ? 1000.f / smoothedFrameMs_ : 0.f;
        const sf::Vector2f playerCenter = player.getCenterPosition();
        const sf::Vector2f playerFeet = player.getFeetPosition();
        const sf::Vector2f cameraCenter = camera.getCameraCenterPos();
        const sf::Vector2i levelSize = levelManager.getCurrentLevelSize();
        const CampaignObjectiveSnapshot snapshot = player.getCampaignSnapshot();
        const GameData::LaunchPreferences launchPreferences = gameData.getLaunchPreferences();
        const std::vector<std::string> unlockedLevels = player.getUnlockedLevelNames(levelManager.getLevelNames());

        ImGui::TextUnformatted("F1 closes this console. World simulation is paused while it stays open.");
        if (ImGui::BeginTabBar("RitualConsoleTabs"))
        {
            if (ImGui::BeginTabItem("Performance"))
            {
                ImGui::Text("FPS: %.1f", fps);
                ImGui::Text("Frame time: %.2f ms", smoothedFrameMs_);
                ImGui::PlotLines(
                    "Frame ms",
                    frameHistory_.data(),
                    static_cast<int>(frameHistoryCount_),
                    static_cast<int>(frameHistoryIndex_ % std::max<std::size_t>(frameHistoryCount_, 1u)),
                    nullptr,
                    0.f,
                    40.f,
                    ImVec2(0.f, 72.f)
                );
                ImGui::Spacing();
                ImGui::Checkbox("Show ImGui demo", &showImGuiDemo_);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Player"))
            {
                ImGui::Text("Alive: %s", player.isAlive ? "yes" : "no");
                ImGui::Text("HP: %d / %d", player.getHP(), player.getMaxHP());
                ImGui::Text("Energy: %d / %d", player.getEnergy(), player.getMaxEnergy());
                ImGui::Text("Gold: %d", player.getGold());
                ImGui::Text("Damage: %d", player.getDamageValue());
                ImGui::Text("Player center: %.1f, %.1f", playerCenter.x, playerCenter.y);
                ImGui::Text("Feet anchor: %.1f, %.1f", playerFeet.x, playerFeet.y);
                ImGui::Text("Move speed: %.2f / %.2f", player.getAccelerationValue(), player.getMaxWalkSpeedValue());
                ImGui::Text("Jump power: %.2f", player.getJumpPowerValue());
                ImGui::Text("Dash force: %.2f", player.getDashForceValue());

                ImGui::SeparatorText("Quick Actions");
                if (ImGui::Button("Restore vitals"))
                {
                    actions_.onRestoreVitals();
                }
                ImGui::SameLine();
                if (ImGui::Button("Restart level"))
                {
                    actions_.onRestartLevel();
                }
                if (ImGui::Button("Respawn at checkpoint"))
                {
                    actions_.onRespawnAtSpawn();
                }
                ImGui::SameLine();
                if (ImGui::Button("Return to level base"))
                {
                    actions_.onReturnToBase();
                }

                ImGui::SetNextItemWidth(120.f);
                ImGui::InputInt("Grant gold", &grantGoldAmount_);
                grantGoldAmount_ = std::max(grantGoldAmount_, 1);
                ImGui::SameLine();
                if (ImGui::Button("Give"))
                {
                    actions_.onGrantGold(grantGoldAmount_);
                }

                ImGui::SeparatorText("Abilities");
                drawCooldownLine("Shot", player.getShootClock(), player.getShootCooldownValue());
                drawCooldownLine("Dash", player.getDashClock(), player.getDashCooldownValue());
                drawCooldownLine("Portal", player.getPortalClock(), player.getPortalCooldown());

                ImGui::SeparatorText("Objective");
                ImGui::TextWrapped("%s", snapshot.campaignTitle.c_str());
                ImGui::TextColored(ImVec4(0.95f, 0.78f, 0.48f, 1.f), "%s", snapshot.chapterTitle.c_str());
                ImGui::TextWrapped("%s", snapshot.narrative.c_str());
                ImGui::Spacing();
                ImGui::TextWrapped("Task: %s", snapshot.objective.c_str());
                ImGui::ProgressBar(std::clamp(snapshot.progressRatio, 0.f, 1.f), ImVec2(-1.f, 0.f));
                ImGui::Text("%s", snapshot.progressText.c_str());
                ImGui::TextWrapped("Reward: %s", snapshot.rewardText.c_str());
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("World"))
            {
                ImGui::Text("Run: %s", hasActiveRun ? "active" : "dormant");
                ImGui::Text("Death flow: %s", deathSequenceActive ? "engaged" : "stable");
                ImGui::Text("Current level: %s", levelManager.getCurrentLevelName().c_str());
                ImGui::Text("Level size: %d x %d", levelSize.x, levelSize.y);
                ImGui::Text("Camera center: %.1f, %.1f", cameraCenter.x, cameraCenter.y);
                ImGui::Text("Camera zoom: %.2f", camera.getZoom());

                ImGui::SeparatorText("Quick Travel");
                for (const auto& levelName : unlockedLevels)
                {
                    const std::string buttonLabel = levelManager.getLevelDisplayName(levelName) + "##" + levelName;
                    if (ImGui::Button(buttonLabel.c_str(), ImVec2(150.f, 0.f)))
                    {
                        actions_.onGoToLevel(levelName);
                    }

                    if (levelName != unlockedLevels.back())
                    {
                        ImGui::SameLine();
                    }
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Session"))
            {
                bool vsyncEnabled = launchPreferences.vsyncEnabled;
                if (ImGui::Checkbox("VSync", &vsyncEnabled))
                {
                    actions_.onSetVsync(vsyncEnabled);
                }

                int ashDensityIndex = launchPreferences.menuParticleCount <= 120
                    ? 0
                    : (launchPreferences.menuParticleCount >= 260 ? 2 : 1);
                const char* ashDensityLabels[] = {"Low", "Medium", "High"};
                if (ImGui::Combo("Menu ash density", &ashDensityIndex, ashDensityLabels, IM_ARRAYSIZE(ashDensityLabels)))
                {
                    const int ashDensityValues[] = {120, 200, 260};
                    actions_.onSetMenuParticleCount(ashDensityValues[ashDensityIndex]);
                }

                ImGui::SeparatorText("Stored Preferences");
                ImGui::Text("VSync: %s", launchPreferences.vsyncEnabled ? "ON" : "OFF");
                ImGui::Text(
                    "Menu ash density: %s (%d)",
                    getAshDensityLabel(launchPreferences.menuParticleCount),
                    launchPreferences.menuParticleCount
                );
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    ImGui::SFML::Render(*window_);
}

bool DeveloperOverlay::isOpen() const
{
    return isOpen_;
}

void DeveloperOverlay::applyStyle() const
{
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.f;
    style.ChildRounding = 8.f;
    style.FrameRounding = 6.f;
    style.GrabRounding = 6.f;
    style.PopupRounding = 8.f;
    style.ScrollbarRounding = 6.f;
    style.FramePadding = ImVec2(10.f, 6.f);
    style.ItemSpacing = ImVec2(10.f, 8.f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.05f, 0.06f, 0.94f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.22f, 0.06f, 0.07f, 0.96f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.34f, 0.10f, 0.09f, 0.98f);
    colors[ImGuiCol_Header] = ImVec4(0.32f, 0.14f, 0.11f, 0.78f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.48f, 0.21f, 0.15f, 0.86f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.58f, 0.28f, 0.18f, 0.94f);
    colors[ImGuiCol_Button] = ImVec4(0.38f, 0.14f, 0.10f, 0.84f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.55f, 0.22f, 0.14f, 0.90f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.66f, 0.28f, 0.17f, 0.96f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.10f, 0.12f, 0.92f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.13f, 0.15f, 0.98f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.17f, 0.19f, 0.98f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.74f, 0.40f, 1.f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.90f, 0.62f, 0.32f, 0.92f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.74f, 0.41f, 1.f);
    colors[ImGuiCol_Separator] = ImVec4(0.58f, 0.34f, 0.22f, 0.76f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.74f, 0.47f, 0.25f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.91f, 0.62f, 0.31f, 0.78f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.76f, 0.42f, 0.92f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.98f, 0.78f, 0.49f, 1.f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.86f, 0.60f, 1.f);
}

void DeveloperOverlay::tryLoadGameFont() const
{
    const std::filesystem::path fontPath = "fonts/Roboto_Condensed-Black.ttf";
    if (!std::filesystem::exists(fontPath))
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    if (io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 18.f) == nullptr)
    {
        io.Fonts->AddFontDefault();
    }

    if (!ImGui::SFML::UpdateFontTexture())
    {
        io.Fonts->Clear();
        io.Fonts->AddFontDefault();
        [[maybe_unused]] const bool fallbackLoaded = ImGui::SFML::UpdateFontTexture();
    }
}

void DeveloperOverlay::recordFrameTime(sf::Time deltaTime)
{
    const float frameMs = deltaTime.asSeconds() * 1000.f;
    smoothedFrameMs_ = smoothedFrameMs_ <= 0.001f
        ? frameMs
        : smoothedFrameMs_ * 0.88f + frameMs * 0.12f;

    frameHistory_[frameHistoryIndex_] = frameMs;
    frameHistoryIndex_ = (frameHistoryIndex_ + 1u) % frameHistory_.size();
    frameHistoryCount_ = std::min(frameHistoryCount_ + 1u, frameHistory_.size());
}

bool DeveloperOverlay::isKeyboardOrMouseEvent(const sf::Event& event) const
{
    return event.is<sf::Event::KeyPressed>()
        || event.is<sf::Event::KeyReleased>()
        || event.is<sf::Event::MouseMoved>()
        || event.is<sf::Event::MouseButtonPressed>()
        || event.is<sf::Event::MouseButtonReleased>()
        || event.is<sf::Event::MouseWheelScrolled>()
        || event.is<sf::Event::TextEntered>();
}
