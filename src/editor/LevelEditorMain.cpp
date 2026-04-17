#include <Defines.h>
#include <GameData.h>
#include <Ground.h>
#include <LevelRegistry.h>
#include <Platform.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <sfml-headers.h>

#include <SFML/Graphics/VertexArray.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifndef SFML_GAME_DEMO_SOURCE_DIR
#define SFML_GAME_DEMO_SOURCE_DIR ""
#endif

namespace
{
constexpr char kRuntimeLevelsFolder[] = "data/levelData";
constexpr float kGridSpacing = 64.f;
constexpr float kSelectionOutlineThickness = 2.f;
constexpr float kPlatformHandleSize = 18.f;
constexpr float kPlatformMinHitboxSize = 8.f;
constexpr float kPlatformMinScale = 0.02f;
constexpr float kDecorationMinScale = 0.05f;
constexpr float kCameraArrowSpeed = 1280.f;
constexpr sf::Vector2f kWorldNameplateScale{0.30f, 0.30f};
constexpr sf::Color kGridColor(255, 255, 255, 18);
constexpr sf::Color kPlatformOutlineColor(248, 206, 134, 255);
constexpr sf::Color kPlatformFillColor(248, 206, 134, 30);
constexpr sf::Color kPlatformSpriteOutlineColor(132, 214, 255, 255);
constexpr sf::Color kPlatformSpriteFillColor(132, 214, 255, 18);
constexpr sf::Color kDecorationOutlineColor(124, 224, 196, 255);
constexpr sf::Color kDecorationFillColor(124, 224, 196, 24);
constexpr sf::Color kBackgroundOutlineColor(140, 176, 255, 255);
constexpr sf::Color kSpawnerOutlineColor(244, 128, 104, 255);
constexpr sf::Color kInteractiveOutlineColor(236, 216, 144, 255);
constexpr sf::Color kMiniLocationOutlineColor(176, 232, 220, 255);
constexpr sf::Color kSpawnOutlineColor(255, 255, 255, 255);

const std::array<const char*, 5> kEnemyTypes{
    "SkeletonWhite",
    "SkeletonYellow",
    "WraithBat",
    "VoidSlime",
    "DreadScorpion"
};

const std::array<const char*, 4> kInteractiveTypes{
    "EchoTablet",
    "CustomSign",
    "RestShrine",
    "GoldCache"
};

const std::array<const char*, 6> kWeatherThemes{
    "VerdantDawn",
    "StormFront",
    "BloodMoon",
    "BoneCrypt",
    "TwilightRift",
    "MidnightRain"
};

const std::array<const char*, 5> kPlatformAtmosphereModeOptions{
    "Default",
    "None",
    "Dust",
    "Ember",
    "Drip"
};

const std::array<const char*, 22> kAnimatedDecorationNames{
    "plant1",
    "plant2",
    "plant3",
    "plant4",
    "plant5",
    "plant6",
    "plant7",
    "jumpPlant",
    "jumpPlant2",
    "jumpBloom2",
    "windPlant1",
    "windPlant",
    "plantWind1",
    "blueFlower1",
    "blueFlower2",
    "blueFlower",
    "blueFlowerClosed",
    "poisonPlant",
    "cat",
    "portalGreen",
    "portalBlue1",
    "portalBlue8"
};

enum class EditorTab
{
    Level,
    Platforms,
    Decorations,
    Backgrounds,
    Ground,
    Spawners,
    Interactives,
    MiniLocations
};

enum class SelectionKind
{
    None,
    Spawn,
    Platform,
    Decoration,
    Background,
    Ground,
    Spawner,
    Interactive,
    MiniLocation
};

struct EditorSelection
{
    SelectionKind kind = SelectionKind::None;
    std::size_t index = 0u;

    bool isValid() const
    {
        return kind != SelectionKind::None;
    }

    void clear()
    {
        kind = SelectionKind::None;
        index = 0u;
    }
};

bool operator==(const EditorSelection& lhs, const EditorSelection& rhs)
{
    return lhs.kind == rhs.kind && lhs.index == rhs.index;
}

bool operator!=(const EditorSelection& lhs, const EditorSelection& rhs)
{
    return !(lhs == rhs);
}

enum class PlatformInteractionMode
{
    None,
    Move,
    MoveHitbox,
    ResizeHitbox,
    ResizeScale
};

struct PlatformInteractionState
{
    PlatformInteractionMode mode = PlatformInteractionMode::None;
    std::size_t index = 0u;
    sf::Vector2f startWorld{0.f, 0.f};
    sf::Vector2f startPosition{0.f, 0.f};
    sf::Vector2f startHitboxOffset{0.f, 0.f};
    sf::Vector2f startHitboxSize{0.f, 0.f};
    sf::Vector2f startScale{1.f, 1.f};
    sf::Vector2f startSpriteHalfSize{0.f, 0.f};

    bool active() const
    {
        return mode != PlatformInteractionMode::None;
    }

    void clear()
    {
        mode = PlatformInteractionMode::None;
        index = 0u;
        startWorld = {0.f, 0.f};
        startPosition = {0.f, 0.f};
        startHitboxOffset = {0.f, 0.f};
        startHitboxSize = {0.f, 0.f};
        startScale = {1.f, 1.f};
        startSpriteHalfSize = {0.f, 0.f};
    }
};

enum class DecorationInteractionMode
{
    None,
    Move,
    ResizeScale
};

struct DecorationInteractionState
{
    DecorationInteractionMode mode = DecorationInteractionMode::None;
    std::size_t index = 0u;
    sf::Vector2f startWorld{0.f, 0.f};
    sf::Vector2f startPosition{0.f, 0.f};
    sf::Vector2f startScale{1.f, 1.f};
    sf::Vector2f startSpriteHalfSize{0.f, 0.f};

    bool active() const
    {
        return mode != DecorationInteractionMode::None;
    }

    void clear()
    {
        mode = DecorationInteractionMode::None;
        index = 0u;
        startWorld = {0.f, 0.f};
        startPosition = {0.f, 0.f};
        startScale = {1.f, 1.f};
        startSpriteHalfSize = {0.f, 0.f};
    }
};

struct SpawnInteractionState
{
    bool active = false;

    void clear()
    {
        active = false;
    }
};

struct PendingSelectionCycleState
{
    bool active = false;
    sf::Vector2i startPixel{};
    sf::Vector2f startWorld{0.f, 0.f};
    EditorSelection initialSelection{};
    std::vector<EditorSelection> candidates{};

    void clear()
    {
        active = false;
        startPixel = {};
        startWorld = {0.f, 0.f};
        initialSelection.clear();
        candidates.clear();
    }
};

enum class InteractiveInteractionMode
{
    None,
    Move,
    ResizeScale
};

struct InteractiveInteractionState
{
    InteractiveInteractionMode mode = InteractiveInteractionMode::None;
    std::size_t index = 0u;
    sf::Vector2f startWorld{0.f, 0.f};
    sf::Vector2f startPosition{0.f, 0.f};
    sf::Vector2f startScale{1.f, 1.f};
    sf::Vector2f startSpriteHalfSize{0.f, 0.f};

    bool active() const
    {
        return mode != InteractiveInteractionMode::None;
    }

    void clear()
    {
        mode = InteractiveInteractionMode::None;
        index = 0u;
        startWorld = {0.f, 0.f};
        startPosition = {0.f, 0.f};
        startScale = {1.f, 1.f};
        startSpriteHalfSize = {0.f, 0.f};
    }
};

enum class SpawnerInteractionMode
{
    None,
    Move,
    Resize
};

struct SpawnerInteractionState
{
    SpawnerInteractionMode mode = SpawnerInteractionMode::None;
    std::size_t index = 0u;
    sf::Vector2f startWorld{0.f, 0.f};
    sf::FloatRect startBounds{};

    bool active() const
    {
        return mode != SpawnerInteractionMode::None;
    }

    void clear()
    {
        mode = SpawnerInteractionMode::None;
        index = 0u;
        startWorld = {0.f, 0.f};
        startBounds = {};
    }
};

enum class MiniLocationInteractionMode
{
    None,
    Move,
    Resize
};

struct MiniLocationInteractionState
{
    MiniLocationInteractionMode mode = MiniLocationInteractionMode::None;
    std::size_t index = 0u;
    sf::Vector2f startWorld{0.f, 0.f};
    sf::FloatRect startBounds{};

    bool active() const
    {
        return mode != MiniLocationInteractionMode::None;
    }

    void clear()
    {
        mode = MiniLocationInteractionMode::None;
        index = 0u;
        startWorld = {0.f, 0.f};
        startBounds = {};
    }
};

enum class GroundInteractionMode
{
    None,
    Move,
    Resize
};

struct GroundInteractionState
{
    GroundInteractionMode mode = GroundInteractionMode::None;
    std::size_t index = 0u;
    sf::Vector2f startWorld{0.f, 0.f};
    float startStartX = 0.f;
    float startEndX = 0.f;
    float startYPos = 0.f;
    float startOffset = 0.f;

    bool active() const
    {
        return mode != GroundInteractionMode::None;
    }

    void clear()
    {
        mode = GroundInteractionMode::None;
        index = 0u;
        startWorld = {0.f, 0.f};
        startStartX = 0.f;
        startEndX = 0.f;
        startYPos = 0.f;
        startOffset = 0.f;
    }
};

sf::Vector2f readVector2f(const nlohmann::json& value, const sf::Vector2f fallback = {0.f, 0.f})
{
    if (!value.is_array() || value.size() < 2)
    {
        return fallback;
    }

    return {
        value[0].get<float>(),
        value[1].get<float>()
    };
}

sf::Color readColor(const nlohmann::json& value, const sf::Color fallback = sf::Color::White)
{
    if (!value.is_array() || value.size() < 4)
    {
        return fallback;
    }

    return sf::Color{
        value[0].get<std::uint8_t>(),
        value[1].get<std::uint8_t>(),
        value[2].get<std::uint8_t>(),
        value[3].get<std::uint8_t>()
    };
}

sf::FloatRect readRect(const nlohmann::json& value, const sf::FloatRect fallback = sf::FloatRect({0.f, 0.f}, {0.f, 0.f}))
{
    if (!value.is_array() || value.size() < 4)
    {
        return fallback;
    }

    return sf::FloatRect(
        {value[0].get<float>(), value[1].get<float>()},
        {value[2].get<float>(), value[3].get<float>()}
    );
}

nlohmann::json toJson(const sf::Vector2f value)
{
    return nlohmann::json::array({value.x, value.y});
}

nlohmann::json toJson(const sf::Color color)
{
    return nlohmann::json::array({color.r, color.g, color.b, color.a});
}

nlohmann::json toJson(const sf::FloatRect rect)
{
    return nlohmann::json::array({rect.position.x, rect.position.y, rect.size.x, rect.size.y});
}

void copyStringToBuffer(const std::string& value, char* buffer, const std::size_t size)
{
    if (size == 0u)
    {
        return;
    }

    std::snprintf(buffer, size, "%s", value.c_str());
}

std::string makeSafeFileName(std::string value)
{
    std::replace_if(value.begin(), value.end(), [](const unsigned char character) {
        return !(std::isalnum(character) || character == '_' || character == '-' || character == '.');
    }, '_');

    if (value.empty())
    {
        value = "new_level.json";
    }

    if (value.find('.') == std::string::npos)
    {
        value += ".json";
    }

    return value;
}

std::vector<std::string> sortedTextureKeys(const std::map<std::string, sf::Texture>& textures)
{
    std::vector<std::string> keys;
    keys.reserve(textures.size());
    for (const auto& [name, _] : textures)
    {
        keys.push_back(name);
    }

    std::sort(keys.begin(), keys.end());
    return keys;
}

std::optional<std::filesystem::path> getSourceLevelsFolder()
{
    const std::string sourceDir = SFML_GAME_DEMO_SOURCE_DIR;
    if (sourceDir.empty())
    {
        return std::nullopt;
    }

    return std::filesystem::path(sourceDir) / "data" / "levelData";
}

bool comboFromStrings(const char* label, const std::vector<std::string>& options, std::string& currentValue)
{
    if (options.empty())
    {
        ImGui::TextDisabled("%s: no options", label);
        return false;
    }

    const char* previewValue = currentValue.empty() ? options.front().c_str() : currentValue.c_str();
    bool changed = false;

    if (ImGui::BeginCombo(label, previewValue))
    {
        for (const std::string& option : options)
        {
            const bool selected = (currentValue == option);
            if (ImGui::Selectable(option.c_str(), selected))
            {
                currentValue = option;
                changed = true;
            }

            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    return changed;
}

bool comboFromLiteralArray(const char* label, const std::vector<std::string>& options, std::string& currentValue)
{
    return comboFromStrings(label, options, currentValue);
}

bool editStringField(const char* label, nlohmann::json& object, const char* key, const std::size_t capacity = 256u)
{
    std::vector<char> buffer(capacity, '\0');
    copyStringToBuffer(object.value(key, std::string{}), buffer.data(), buffer.size());
    if (ImGui::InputText(label, buffer.data(), buffer.size()))
    {
        object[key] = std::string(buffer.data());
        return true;
    }

    return false;
}

bool editMultilineStringField(const char* label, nlohmann::json& object, const char* key, const ImVec2 size, const std::size_t capacity = 1024u)
{
    std::vector<char> buffer(capacity, '\0');
    copyStringToBuffer(object.value(key, std::string{}), buffer.data(), buffer.size());
    if (ImGui::InputTextMultiline(label, buffer.data(), buffer.size(), size))
    {
        object[key] = std::string(buffer.data());
        return true;
    }

    return false;
}

bool editVector2Field(const char* label, nlohmann::json& object, const char* key, const sf::Vector2f fallback = {0.f, 0.f})
{
    if (!object.contains(key))
    {
        object[key] = toJson(fallback);
    }

    sf::Vector2f value = readVector2f(object[key], fallback);
    float raw[2]{value.x, value.y};
    if (ImGui::InputFloat2(label, raw))
    {
        object[key] = nlohmann::json::array({raw[0], raw[1]});
        return true;
    }

    return false;
}

bool editColorField(const char* label, nlohmann::json& object, const char* key, const sf::Color fallback = sf::Color::White)
{
    if (!object.contains(key))
    {
        object[key] = toJson(fallback);
    }

    const sf::Color color = readColor(object[key], fallback);
    float raw[4]{
        color.r / 255.f,
        color.g / 255.f,
        color.b / 255.f,
        color.a / 255.f
    };
    if (ImGui::ColorEdit4(label, raw))
    {
        object[key] = nlohmann::json::array({
            static_cast<int>(std::round(raw[0] * 255.f)),
            static_cast<int>(std::round(raw[1] * 255.f)),
            static_cast<int>(std::round(raw[2] * 255.f)),
            static_cast<int>(std::round(raw[3] * 255.f))
        });
        return true;
    }

    return false;
}

bool editParallaxFactorField(const char* label, nlohmann::json& object, const sf::Vector2f fallback = {1.f, 1.f})
{
    if (!object.contains("ParallaxFactor"))
    {
        object["ParallaxFactor"] = toJson(fallback);
    }

    const sf::Vector2f value = readVector2f(object["ParallaxFactor"], fallback);
    float raw[2]{value.x, value.y};
    if (ImGui::SliderFloat2(label, raw, -1.0f, 1.5f, "%.2f"))
    {
        object["ParallaxFactor"] = nlohmann::json::array({raw[0], raw[1]});
        return true;
    }

    return false;
}

std::string formatPositionLabel(const sf::Vector2f position)
{
    return std::to_string(static_cast<int>(std::round(position.x)))
        + ", "
        + std::to_string(static_cast<int>(std::round(position.y)));
}

std::uint32_t hashNameplateSeed(const std::string& value)
{
    std::uint32_t hash = 2166136261u;
    for (const unsigned char symbol : value)
    {
        hash ^= symbol;
        hash *= 16777619u;
    }
    return hash;
}

class LevelEditorApp
{
public:
    LevelEditorApp()
        : window_(
            sf::VideoMode({1600u, 900u}),
            "SFML Game Demo Level Editor",
            (sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize)
        )
        , worldView_({0.f, 0.f}, {1600.f, 900.f})
    {
        window_.setFramerateLimit(60u);

        if (!font_.openFromFile("fonts/Roboto_Condensed-Black.ttf"))
        {
            throw std::runtime_error("Editor font was not loaded");
        }

        gameData_ = std::make_unique<GameData>(&font_);
        if (!ImGui::SFML::Init(window_))
        {
            throw std::runtime_error("Failed to initialize ImGui-SFML for level editor");
        }

        ImGui::GetIO().IniFilename = "data/level_editor_imgui.ini";
        applyEditorStyle();
        buildCatalogs();
        refreshRegistry();

        if (!registry_.getLevels().empty())
        {
            loadLevel(registry_.getLevels().front().filePath);
        }
        else
        {
            createDefaultLevel();
        }
    }

    ~LevelEditorApp()
    {
        ImGui::SFML::Shutdown();
    }

    int run()
    {
        while (window_.isOpen())
        {
            while (const std::optional event = window_.pollEvent())
            {
                handleEvent(*event);
            }

            const sf::Time deltaTime = sf::seconds(std::min(frameClock_.restart().asSeconds(), 0.05f));
            ImGui::SFML::Update(window_, deltaTime);
            updateCameraKeyboardMovement(deltaTime.asSeconds());

            window_.clear(sf::Color(13, 12, 16, 255));
            drawWorld();
            drawUi();
            window_.setView(window_.getDefaultView());
            ImGui::SFML::Render(window_);
            window_.display();
        }

        return 0;
    }

private:
    sf::RenderWindow window_;
    sf::View worldView_;
    sf::Clock frameClock_;
    sf::Font font_;
    std::unique_ptr<GameData> gameData_{};
    LevelRegistry registry_{};

    nlohmann::json document_{};
    std::filesystem::path currentFilePath_{};
    bool dirty_ = false;

    EditorTab activeTab_ = EditorTab::Level;
    EditorSelection selection_{};
    SelectionKind placementMode_ = SelectionKind::None;

    bool draggingView_ = false;
    sf::Vector2i lastDragPixel_{};
    PendingSelectionCycleState pendingSelectionCycle_{};
    SpawnInteractionState spawnInteraction_{};
    PlatformInteractionState platformInteraction_{};
    DecorationInteractionState decorationInteraction_{};
    InteractiveInteractionState interactiveInteraction_{};
    SpawnerInteractionState spawnerInteraction_{};
    MiniLocationInteractionState miniLocationInteraction_{};
    GroundInteractionState groundInteraction_{};
    bool openWorldContextMenu_ = false;

    std::map<std::string, sf::Texture> platformPreviewTextures_{};
    std::map<std::string, const sf::Texture*> previewTextures_{};
    std::vector<std::string> platformTypes_{};
    std::vector<std::string> decorationOptions_{};
    std::vector<std::string> backgroundOptions_{};
    std::vector<std::string> groundTileOptions_{};
    std::vector<std::string> groundStyleOptions_{};
    std::vector<std::string> weatherThemeOptions_{};
    std::vector<std::string> nameplateTextureOptions_{};
    std::vector<std::string> previewTextureOptions_{};
    std::vector<std::string> enemyTypeOptions_{};
    std::vector<std::string> interactiveTypeOptions_{};

    int selectedPlatformTypeIndex_ = 0;
    int selectedDecorationIndex_ = 0;
    int selectedBackgroundIndex_ = 0;
    int selectedGroundTileIndex_ = 0;
    int selectedGroundStyleIndex_ = 0;
    int selectedNameplateTextureIndex_ = 0;
    int selectedPreviewTextureIndex_ = 0;
    int selectedEnemyTypeIndex_ = 0;
    int selectedInteractiveTypeIndex_ = 0;

    char fileNameBuffer_[256]{};
    char levelIdBuffer_[256]{};
    char levelTitleBuffer_[256]{};

    void applyEditorStyle()
    {
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 9.f;
        style.FrameRounding = 6.f;
        style.GrabRounding = 5.f;
        style.PopupRounding = 8.f;
        style.FramePadding = ImVec2(9.f, 6.f);
        style.ItemSpacing = ImVec2(10.f, 8.f);

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.97f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.13f, 0.18f, 0.96f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.20f, 0.27f, 0.98f);
        colors[ImGuiCol_Header] = ImVec4(0.22f, 0.30f, 0.40f, 0.78f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.42f, 0.55f, 0.86f);
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.29f, 0.38f, 0.84f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.42f, 0.56f, 0.92f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.38f, 0.50f, 0.66f, 0.96f);
        colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.18f, 0.22f, 0.98f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.30f, 0.38f, 0.98f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.24f, 0.34f, 0.46f, 0.98f);
    }

    void buildCatalogs()
    {
        platformTypes_ = Platform::getAvailableTypes();
        for (const std::string& platformType : platformTypes_)
        {
            const auto definition = Platform::getTypeDefinition(platformType);
            if (!definition.has_value() || definition->texturePath.empty())
            {
                continue;
            }

            sf::Texture texture;
            if (!texture.loadFromFile(definition->texturePath))
            {
                continue;
            }

            platformPreviewTextures_.emplace(platformType, std::move(texture));
        }

        decorationOptions_ = sortedTextureKeys(gameData_->allStaticTextures);
        for (const char* animatedName : kAnimatedDecorationNames)
        {
            decorationOptions_.push_back(animatedName);
        }
        std::sort(decorationOptions_.begin(), decorationOptions_.end());
        decorationOptions_.erase(std::unique(decorationOptions_.begin(), decorationOptions_.end()), decorationOptions_.end());

        backgroundOptions_ = sortedTextureKeys(gameData_->backgroundTextures);
        groundTileOptions_ = sortedTextureKeys(gameData_->TileSetGreenTextures);
        groundStyleOptions_ = Ground::getAvailableStyles();
        weatherThemeOptions_.assign(kWeatherThemes.begin(), kWeatherThemes.end());

        enemyTypeOptions_.assign(kEnemyTypes.begin(), kEnemyTypes.end());
        interactiveTypeOptions_.assign(kInteractiveTypes.begin(), kInteractiveTypes.end());

        for (const auto& [name, texture] : gameData_->allStaticTextures)
        {
            previewTextures_[name] = &texture;
        }
        for (const auto& [name, texture] : gameData_->backgroundTextures)
        {
            previewTextures_[name] = &texture;
        }
        for (const auto& [name, texture] : gameData_->itemsTextures)
        {
            previewTextures_[name] = &texture;
        }
        for (const auto& [name, texture] : gameData_->guiTextures)
        {
            previewTextures_[name] = &texture;
        }
        for (const auto& [name, texture] : gameData_->TileSetGreenTextures)
        {
            previewTextures_[name] = &texture;
        }
        for (auto& [name, texture] : platformPreviewTextures_)
        {
            previewTextures_[name] = &texture;
        }

        const auto registerAnimatedPreview = [&](const std::string& name, const std::vector<sf::Texture>& textures) {
            if (!textures.empty())
            {
                previewTextures_[name] = &textures.front();
            }
        };

        registerAnimatedPreview("plant1", gameData_->plant1Textures);
        registerAnimatedPreview("plant2", gameData_->plant2Textures);
        registerAnimatedPreview("plant3", gameData_->plant3Textures);
        registerAnimatedPreview("plant4", gameData_->plant4Textures);
        registerAnimatedPreview("plant5", gameData_->plant5Textures);
        registerAnimatedPreview("plant6", gameData_->plant6Textures);
        registerAnimatedPreview("plant7", gameData_->plant7Textures);
        registerAnimatedPreview("jumpPlant", gameData_->jumpPlantTextures);
        registerAnimatedPreview("jumpPlant2", gameData_->jumpPlant2Textures);
        registerAnimatedPreview("jumpBloom2", gameData_->jumpPlant2Textures);
        registerAnimatedPreview("windPlant1", gameData_->plantWind1Textures);
        registerAnimatedPreview("windPlant", gameData_->plantWind1Textures);
        registerAnimatedPreview("plantWind1", gameData_->plantWind1Textures);
        registerAnimatedPreview("blueFlower1", gameData_->blueFlower1Textures);
        registerAnimatedPreview("blueFlower2", gameData_->blueFlower2Textures);
        registerAnimatedPreview("blueFlower", gameData_->blueFlower1Textures);
        registerAnimatedPreview("blueFlowerClosed", gameData_->blueFlower2Textures);
        registerAnimatedPreview("poisonPlant", gameData_->plant8PoisonTextures);
        registerAnimatedPreview("cat", gameData_->cat1Textures);
        registerAnimatedPreview("portalGreen", gameData_->portalGreenTextures);
        registerAnimatedPreview("portalBlue1", gameData_->portalBlue1Textures);
        registerAnimatedPreview("portalBlue8", gameData_->portalBlue8Textures);

        previewTextureOptions_.reserve(previewTextures_.size());
        for (const auto& [name, _] : previewTextures_)
        {
            previewTextureOptions_.push_back(name);
            if (name.rfind("nameplate_", 0) == 0)
            {
                nameplateTextureOptions_.push_back(name);
            }
        }
        std::sort(previewTextureOptions_.begin(), previewTextureOptions_.end());
        std::sort(nameplateTextureOptions_.begin(), nameplateTextureOptions_.end());
    }

    std::filesystem::path runtimeLevelsFolder() const
    {
        return std::filesystem::path(kRuntimeLevelsFolder);
    }

    void refreshRegistry()
    {
        registry_.scan(runtimeLevelsFolder());
    }

    void markDirty()
    {
        dirty_ = true;
    }

    void syncMetadataBuffers()
    {
        copyStringToBuffer(currentFilePath_.filename().string(), fileNameBuffer_, sizeof(fileNameBuffer_));
        copyStringToBuffer(document_["Presets"].value("LevelId", std::string{}), levelIdBuffer_, sizeof(levelIdBuffer_));
        copyStringToBuffer(document_["Presets"].value("Title", std::string{}), levelTitleBuffer_, sizeof(levelTitleBuffer_));
    }

    void ensureDocumentShape()
    {
        if (!document_.is_object())
        {
            document_ = nlohmann::json::object();
        }

        if (!document_.contains("Presets") || !document_["Presets"].is_object())
        {
            document_["Presets"] = nlohmann::json::object();
        }

        auto& presets = document_["Presets"];
        if (!presets.contains("isConstant"))
        {
            presets["isConstant"] = true;
        }
        if (!presets.contains("isAvaiable"))
        {
            presets["isAvaiable"] = true;
        }
        if (!presets.contains("GenerateMiniLocations"))
        {
            presets["GenerateMiniLocations"] = false;
        }
        if (!presets.contains("Size"))
        {
            presets["Size"] = nlohmann::json::array({3840, 1080});
        }
        if (!presets.contains("MainWorldWidth"))
        {
            presets["MainWorldWidth"] = presets["Size"][0];
        }
        if (!presets.contains("PlayerSpawn"))
        {
            presets["PlayerSpawn"] = nlohmann::json::array({200.f, 900.f});
        }
        if (!presets.contains("BackgroundTheme"))
        {
            presets["BackgroundTheme"] = "VerdantDawn";
        }
        if (!presets.contains("Title"))
        {
            presets["Title"] = currentFilePath_.stem().string();
        }
        if (!presets.contains("LevelId"))
        {
            presets["LevelId"] = currentFilePath_.filename().string();
        }
        if (!presets.contains("MenuOrder"))
        {
            presets["MenuOrder"] = static_cast<int>(registry_.getLevels().size()) + 1;
        }

        for (const char* arrayName : {"Platforms", "Decorations", "Background", "Ground", "Spawners", "Interactives", "MiniLocations"})
        {
            if (!document_.contains(arrayName) || !document_[arrayName].is_array())
            {
                document_[arrayName] = nlohmann::json::array();
            }
        }

        syncMetadataBuffers();
        selection_.clear();
    }

    void createDefaultLevel()
    {
        currentFilePath_.clear();
        document_ = {
            {"Presets", {
                {"LevelId", "new_level.json"},
                {"Title", "New Level"},
                {"MenuOrder", static_cast<int>(registry_.getLevels().size()) + 1},
                {"isConstant", true},
                {"isAvaiable", true},
                {"GenerateMiniLocations", false},
                {"Size", {3840, 1080}},
                {"MainWorldWidth", 3840},
                {"PlayerSpawn", {200.f, 900.f}},
                {"BackgroundTheme", "VerdantDawn"}
            }},
            {"Platforms", nlohmann::json::array({
                {
                    {"Type", platformTypes_.empty() ? "Runed-ledge" : platformTypes_.front()},
                    {"Position", {360.f, 900.f}}
                }
            })},
            {"Decorations", nlohmann::json::array()},
            {"Background", nlohmann::json::array({
                {
                    {"BgName", backgroundOptions_.empty() ? "" : backgroundOptions_.front()},
                    {"Position", {960.f, 540.f}},
                    {"ParallaxFactor", {0.08f, 0.06f}},
                    {"Type", 0}
                }
            })},
            {"Ground", nlohmann::json::array({
                {
                    {"GroundStyle", groundStyleOptions_.empty() ? "VerdantKeep" : groundStyleOptions_.front()},
                    {"GroundName", groundTileOptions_.empty() ? "TileSetGreen_02.png" : groundTileOptions_.front()},
                    {"Points", {0, 3840}},
                    {"YPos", 980},
                    {"DepthRows", 2},
                    {"Offset", 8.f}
                }
            })},
            {"Spawners", nlohmann::json::array()},
            {"Interactives", nlohmann::json::array()},
            {"MiniLocations", nlohmann::json::array()}
        };

        ensureDocumentShape();
        worldView_.setCenter({960.f, 540.f});
        dirty_ = true;
    }

    void loadLevel(const std::filesystem::path& filePath)
    {
        std::ifstream input(filePath);
        if (!input.is_open())
        {
            return;
        }

        document_ = nlohmann::json::parse(input, nullptr, false);
        if (document_.is_discarded())
        {
            createDefaultLevel();
            return;
        }

        currentFilePath_ = filePath;
        ensureDocumentShape();
        const auto levelSize = readVector2f(document_["Presets"]["Size"], {3840.f, 1080.f});
        worldView_.setCenter({levelSize.x * 0.5f, levelSize.y * 0.5f});
        dirty_ = false;
    }

    bool saveDocumentToPath(const std::filesystem::path& targetPath)
    {
        std::filesystem::create_directories(targetPath.parent_path());

        std::ofstream output(targetPath);
        if (!output.is_open())
        {
            return false;
        }

        output << document_.dump(4);
        output.close();

        if (const auto sourceLevelsFolder = getSourceLevelsFolder(); sourceLevelsFolder.has_value())
        {
            std::filesystem::create_directories(*sourceLevelsFolder);
            const std::filesystem::path sourcePath = *sourceLevelsFolder / targetPath.filename();
            if (sourcePath != targetPath)
            {
                std::ofstream sourceOutput(sourcePath);
                if (sourceOutput.is_open())
                {
                    sourceOutput << document_.dump(4);
                }
            }
        }

        return true;
    }

    void syncMetadataFromBuffers()
    {
        document_["Presets"]["LevelId"] = std::string(levelIdBuffer_);
        document_["Presets"]["Title"] = std::string(levelTitleBuffer_);
        if (document_["Presets"].value("MainWorldWidth", 0) <= 0)
        {
            document_["Presets"]["MainWorldWidth"] = document_["Presets"]["Size"][0];
        }

        float farthestRight = document_["Presets"]["Size"][0].get<float>();
        for (const auto& room : document_["MiniLocations"])
        {
            const sf::FloatRect bounds = readRect(room.value("Bounds", nlohmann::json::array()));
            farthestRight = std::max(farthestRight, bounds.position.x + bounds.size.x + 180.f);
        }
        document_["Presets"]["Size"][0] = std::max(
            document_["Presets"]["Size"][0].get<int>(),
            static_cast<int>(std::ceil(farthestRight))
        );
    }

    bool saveLevel()
    {
        syncMetadataFromBuffers();

        std::string fileName = makeSafeFileName(fileNameBuffer_);
        if (fileName.empty())
        {
            fileName = makeSafeFileName(std::string(levelIdBuffer_));
        }

        const std::filesystem::path targetPath = runtimeLevelsFolder() / fileName;
        if (!saveDocumentToPath(targetPath))
        {
            return false;
        }

        currentFilePath_ = targetPath;
        dirty_ = false;
        refreshRegistry();
        syncMetadataBuffers();
        return true;
    }

    void handleEvent(const sf::Event& event)
    {
        ImGui::SFML::ProcessEvent(window_, event);

        if (event.is<sf::Event::Closed>())
        {
            window_.close();
            return;
        }

        if (const auto* resized = event.getIf<sf::Event::Resized>())
        {
            const sf::Vector2f newSize = {
                static_cast<float>(resized->size.x),
                static_cast<float>(resized->size.y)
            };
            worldView_.setSize(newSize);
            return;
        }

        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->scancode == sf::Keyboard::Scancode::Delete)
            {
                deleteSelection();
                return;
            }

            if (keyPressed->scancode == sf::Keyboard::Scancode::S && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
            {
                saveLevel();
                return;
            }
        }

        if (const auto* wheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            if (ImGui::GetIO().WantCaptureMouse)
            {
                return;
            }

            const float zoomFactor = wheelScrolled->delta > 0.f ? 0.88f : 1.14f;
            worldView_.zoom(zoomFactor);
            return;
        }

        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Middle)
            {
                draggingView_ = true;
                lastDragPixel_ = mousePressed->position;
                return;
            }

            if (ImGui::GetIO().WantCaptureMouse)
            {
                return;
            }

            const sf::Vector2f worldPosition = window_.mapPixelToCoords(mousePressed->position, worldView_);

            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                if (startPendingSelectionCycle(worldPosition, mousePressed->position))
                {
                    return;
                }
                if (beginSpawnInteraction(worldPosition))
                {
                    return;
                }
                if (beginInteractiveInteraction(worldPosition))
                {
                    return;
                }
                if (beginSpawnerInteraction(worldPosition))
                {
                    return;
                }
                if (beginPlatformInteraction(worldPosition))
                {
                    return;
                }
                if (beginDecorationInteraction(worldPosition))
                {
                    return;
                }
                if (beginMiniLocationInteraction(worldPosition))
                {
                    return;
                }
                if (beginGroundInteraction(worldPosition))
                {
                    return;
                }

                handleWorldLeftClick(worldPosition);
                return;
            }

            if (mousePressed->button == sf::Mouse::Button::Right)
            {
                if (!selectionContainsPoint(selection_, worldPosition))
                {
                    selectObjectAt(worldPosition, true);
                }
                if (selection_.isValid())
                {
                    openWorldContextMenu_ = true;
                }
                return;
            }
        }

        if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>())
        {
            if (mouseReleased->button == sf::Mouse::Button::Left)
            {
                if (pendingSelectionCycle_.active)
                {
                    cyclePendingSelection();
                    return;
                }
                finishSpawnInteraction();
                finishInteractiveInteraction();
                finishSpawnerInteraction();
                finishPlatformInteraction();
                finishDecorationInteraction();
                finishMiniLocationInteraction();
                finishGroundInteraction();
                return;
            }

            if (mouseReleased->button == sf::Mouse::Button::Middle)
            {
                draggingView_ = false;
                return;
            }
        }

        if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>())
        {
            if (pendingSelectionCycle_.active)
            {
                const sf::Vector2i pixelDelta = mouseMoved->position - pendingSelectionCycle_.startPixel;
                if ((pixelDelta.x * pixelDelta.x + pixelDelta.y * pixelDelta.y) >= 9)
                {
                    pendingSelectionCycle_.clear();

                    const sf::Vector2f movedWorldPosition = window_.mapPixelToCoords(mouseMoved->position, worldView_);
                    if (beginSpawnInteraction(movedWorldPosition) ||
                        beginInteractiveInteraction(movedWorldPosition) ||
                        beginSpawnerInteraction(movedWorldPosition) ||
                        beginPlatformInteraction(movedWorldPosition) ||
                        beginDecorationInteraction(movedWorldPosition) ||
                        beginMiniLocationInteraction(movedWorldPosition) ||
                        beginGroundInteraction(movedWorldPosition))
                    {
                        return;
                    }
                }
            }

            if (spawnInteraction_.active)
            {
                updateSpawnInteraction(window_.mapPixelToCoords(mouseMoved->position, worldView_));
                return;
            }
            if (interactiveInteraction_.active())
            {
                updateInteractiveInteraction(window_.mapPixelToCoords(mouseMoved->position, worldView_));
                return;
            }
            if (spawnerInteraction_.active())
            {
                updateSpawnerInteraction(window_.mapPixelToCoords(mouseMoved->position, worldView_));
                return;
            }
            if (platformInteraction_.active())
            {
                updatePlatformInteraction(window_.mapPixelToCoords(mouseMoved->position, worldView_));
                return;
            }
            if (decorationInteraction_.active())
            {
                updateDecorationInteraction(window_.mapPixelToCoords(mouseMoved->position, worldView_));
                return;
            }
            if (miniLocationInteraction_.active())
            {
                updateMiniLocationInteraction(window_.mapPixelToCoords(mouseMoved->position, worldView_));
                return;
            }
            if (groundInteraction_.active())
            {
                updateGroundInteraction(window_.mapPixelToCoords(mouseMoved->position, worldView_));
                return;
            }

            if (!draggingView_)
            {
                return;
            }

            const sf::Vector2f previousWorld = window_.mapPixelToCoords(lastDragPixel_, worldView_);
            const sf::Vector2f currentWorld = window_.mapPixelToCoords(mouseMoved->position, worldView_);
            worldView_.move(previousWorld - currentWorld);
            lastDragPixel_ = mouseMoved->position;
        }
    }

    void updateCameraKeyboardMovement(const float deltaSeconds)
    {
        if (ImGui::GetIO().WantCaptureKeyboard)
        {
            return;
        }

        sf::Vector2f cameraOffset{0.f, 0.f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            cameraOffset.x -= 1.f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            cameraOffset.x += 1.f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        {
            cameraOffset.y -= 1.f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        {
            cameraOffset.y += 1.f;
        }

        if (cameraOffset == sf::Vector2f{0.f, 0.f})
        {
            return;
        }

        const float length = std::sqrt(cameraOffset.x * cameraOffset.x + cameraOffset.y * cameraOffset.y);
        if (length > 0.f)
        {
            cameraOffset /= length;
        }

        const float viewScale = std::max(worldView_.getSize().x / 1600.f, 0.55f);
        worldView_.move(cameraOffset * (kCameraArrowSpeed * viewScale * deltaSeconds));
    }

    void handleWorldLeftClick(const sf::Vector2f worldPosition)
    {
        switch (placementMode_)
        {
        case SelectionKind::Spawn:
            document_["Presets"]["PlayerSpawn"] = toJson(worldPosition);
            selection_.kind = SelectionKind::Spawn;
            markDirty();
            break;
        case SelectionKind::Platform:
            placePlatform(worldPosition);
            break;
        case SelectionKind::Decoration:
            placeDecoration(worldPosition);
            break;
        case SelectionKind::Background:
            placeBackground(worldPosition);
            break;
        case SelectionKind::Spawner:
            placeSpawner(worldPosition);
            break;
        case SelectionKind::Interactive:
            placeInteractive(worldPosition);
            break;
        case SelectionKind::MiniLocation:
            placeMiniLocation(worldPosition);
            break;
        default:
            selectObjectAt(worldPosition);
            break;
        }
    }

    const sf::Texture* findPreviewTexture(const std::string& textureName) const
    {
        const auto textureIt = previewTextures_.find(textureName);
        return textureIt != previewTextures_.end() ? textureIt->second : nullptr;
    }

    sf::FloatRect textureBoundsAt(const std::string& textureName, const sf::Vector2f position, const sf::Vector2f scale, const sf::Vector2f fallbackSize = {64.f, 64.f}) const
    {
        const sf::Texture* texture = findPreviewTexture(textureName);
        const sf::Vector2f size = texture != nullptr
            ? sf::Vector2f{
                static_cast<float>(texture->getSize().x) * std::abs(scale.x),
                static_cast<float>(texture->getSize().y) * std::abs(scale.y)
            }
            : sf::Vector2f{fallbackSize.x * std::abs(scale.x), fallbackSize.y * std::abs(scale.y)};

        return sf::FloatRect(
            {position.x - size.x * 0.5f, position.y - size.y * 0.5f},
            size
        );
    }

    sf::Vector2f runtimePreviewViewSize() const
    {
        return {WINDOW_WIDTH * ZOOM_SCALE, WINDOW_HEIGHT * ZOOM_SCALE};
    }

    float editorGroundCameraClampRight() const
    {
        if (!document_.contains("Ground") || !document_["Ground"].is_array() || document_["Ground"].empty())
        {
            const int levelWidth = document_["Presets"]["Size"][0].get<int>();
            const int mainWorldWidth = document_["Presets"].value("MainWorldWidth", levelWidth);
            return static_cast<float>(std::clamp(mainWorldWidth, 0, levelWidth));
        }

        const auto& ground = document_["Ground"].back();
        const float pointBegin = ground["Points"][0].get<float>();
        const float pointEnd = ground["Points"][1].get<float>();
        const sf::Texture* tileTexture = findPreviewTexture(ground.value("GroundName", std::string{}));
        if (tileTexture == nullptr || tileTexture->getSize().x == 0u)
        {
            return pointEnd;
        }

        const float tileWidth = static_cast<float>(tileTexture->getSize().x);
        const float spanWidth = pointEnd - pointBegin;
        const float remainder = std::fmod(spanWidth, tileWidth);
        if (remainder <= 0.01f || std::abs(remainder - tileWidth) <= 0.01f)
        {
            return pointEnd;
        }

        return std::max(pointBegin, pointEnd - (tileWidth - remainder));
    }

    sf::FloatRect editorCameraBoundsForPosition(const sf::Vector2f position) const
    {
        const float levelHeight = document_["Presets"]["Size"][1].get<float>();

        if (document_.contains("MiniLocations") && document_["MiniLocations"].is_array())
        {
            for (const auto& location : document_["MiniLocations"])
            {
                const sf::FloatRect bounds = readRect(location.value("Bounds", nlohmann::json::array()));
                if (position.x >= bounds.position.x && position.x <= bounds.position.x + bounds.size.x)
                {
                    return sf::FloatRect(
                        {bounds.position.x, 0.f},
                        {bounds.size.x, levelHeight}
                    );
                }
            }
        }

        const int levelWidth = document_["Presets"]["Size"][0].get<int>();
        const int mainWorldWidth = std::clamp(
            document_["Presets"].value("MainWorldWidth", levelWidth),
            0,
            levelWidth
        );
        const float mainWorldRight = std::max(
            editorGroundCameraClampRight() > 0.f
                ? std::min(editorGroundCameraClampRight(), static_cast<float>(mainWorldWidth > 0 ? mainWorldWidth : levelWidth))
                : static_cast<float>(mainWorldWidth),
            0.f
        );

        return sf::FloatRect({0.f, 0.f}, {mainWorldRight, levelHeight});
    }

    sf::Vector2f clampPreviewCameraCenterToBounds(sf::Vector2f position, const sf::FloatRect& cameraBounds) const
    {
        const sf::Vector2f viewSize = runtimePreviewViewSize();
        const float halfWidth = viewSize.x * 0.5f;
        const float halfHeight = viewSize.y * 0.5f;

        const float left = cameraBounds.position.x;
        const float top = cameraBounds.position.y;
        const float right = cameraBounds.position.x + cameraBounds.size.x;
        const float bottom = cameraBounds.position.y + cameraBounds.size.y;

        const float minX = left + halfWidth;
        const float maxX = right - halfWidth;
        if (maxX < minX)
        {
            position.x = left + cameraBounds.size.x * 0.5f;
        }
        else
        {
            position.x = std::clamp(position.x, minX, maxX);
        }

        const float minY = top + halfHeight;
        const float maxY = bottom - halfHeight;
        if (maxY < minY)
        {
            position.y = top + cameraBounds.size.y * 0.5f;
        }
        else
        {
            position.y = std::clamp(position.y, minY, maxY);
        }

        return position;
    }

    sf::Vector2f runtimePreviewCameraCenter() const
    {
        const sf::Vector2f spawn = readVector2f(
            document_["Presets"].value("PlayerSpawn", nlohmann::json::array()),
            {200.f, 900.f}
        );
        return clampPreviewCameraCenterToBounds(spawn, editorCameraBoundsForPosition(spawn));
    }

    sf::Vector2f previewCameraOffsetFromRuntimeBase() const
    {
        return runtimePreviewCameraCenter() - BASE_CAMERAPOS;
    }

    sf::Vector2f applyParallaxPreview(const sf::Vector2f storedPosition, const sf::Vector2f parallaxFactor) const
    {
        const sf::Vector2f cameraOffset = previewCameraOffsetFromRuntimeBase();
        return {
            storedPosition.x + cameraOffset.x * parallaxFactor.x,
            storedPosition.y + cameraOffset.y * parallaxFactor.y
        };
    }

    sf::Vector2f removeParallaxPreview(const sf::Vector2f renderedPosition, const sf::Vector2f parallaxFactor) const
    {
        const sf::Vector2f cameraOffset = previewCameraOffsetFromRuntimeBase();
        return {
            renderedPosition.x - cameraOffset.x * parallaxFactor.x,
            renderedPosition.y - cameraOffset.y * parallaxFactor.y
        };
    }

    bool editDisplayedParallaxPositionField(
        const char* label,
        nlohmann::json& object,
        const sf::Vector2f displayedPosition,
        const sf::Vector2f parallaxFactor)
    {
        float raw[2]{displayedPosition.x, displayedPosition.y};
        if (!ImGui::InputFloat2(label, raw))
        {
            return false;
        }

        object["Position"] = toJson(removeParallaxPreview({raw[0], raw[1]}, parallaxFactor));
        return true;
    }

    bool drawParallaxControls(
        const char* sliderLabel,
        const char* idPrefix,
        nlohmann::json& object,
        const sf::Vector2f renderedPosition,
        const sf::Vector2f fallbackParallax)
    {
        bool changed = false;
        if (editParallaxFactorField(sliderLabel, object, fallbackParallax))
        {
            object["Position"] = toJson(removeParallaxPreview(
                renderedPosition,
                readVector2f(object["ParallaxFactor"], fallbackParallax)
            ));
            changed = true;
        }

        struct Preset
        {
            const char* label;
            sf::Vector2f value;
        };

        const std::array<Preset, 4> presets{{
            {"Static", {0.f, 0.f}},
            {"Back", {0.20f, 0.20f}},
            {"World", {1.f, 1.f}},
            {"Front", {1.20f, 1.20f}}
        }};

        for (std::size_t index = 0; index < presets.size(); ++index)
        {
            if (index > 0u)
            {
                ImGui::SameLine();
            }

            const std::string buttonLabel = std::string(presets[index].label) + "##" + idPrefix;
            if (ImGui::Button(buttonLabel.c_str()))
            {
                object["ParallaxFactor"] = toJson(presets[index].value);
                object["Position"] = toJson(removeParallaxPreview(renderedPosition, presets[index].value));
                changed = true;
            }
        }

        ImGui::TextDisabled("Parallax uses one shared rule for decorations and backgrounds.");
        ImGui::TextDisabled("0.0 = static screen layer, 1.0 = normal world object, >1.0 = foreground.");
        ImGui::TextDisabled("Editor preview is anchored to the runtime camera at player spawn.");
        return changed;
    }

    nlohmann::json* selectedObject()
    {
        if (!selection_.isValid())
        {
            return nullptr;
        }

        nlohmann::json* array = nullptr;
        switch (selection_.kind)
        {
        case SelectionKind::Platform:
            array = &document_["Platforms"];
            break;
        case SelectionKind::Decoration:
            array = &document_["Decorations"];
            break;
        case SelectionKind::Background:
            array = &document_["Background"];
            break;
        case SelectionKind::Ground:
            array = &document_["Ground"];
            break;
        case SelectionKind::Spawner:
            array = &document_["Spawners"];
            break;
        case SelectionKind::Interactive:
            array = &document_["Interactives"];
            break;
        case SelectionKind::MiniLocation:
            array = &document_["MiniLocations"];
            break;
        default:
            return nullptr;
        }

        if (selection_.index >= array->size())
        {
            return nullptr;
        }

        return &(*array)[selection_.index];
    }

    nlohmann::json* arrayForSelectionKind(const SelectionKind kind)
    {
        switch (kind)
        {
        case SelectionKind::Platform:
            return &document_["Platforms"];
        case SelectionKind::Decoration:
            return &document_["Decorations"];
        case SelectionKind::Background:
            return &document_["Background"];
        case SelectionKind::Ground:
            return &document_["Ground"];
        case SelectionKind::Spawner:
            return &document_["Spawners"];
        case SelectionKind::Interactive:
            return &document_["Interactives"];
        case SelectionKind::MiniLocation:
            return &document_["MiniLocations"];
        default:
            return nullptr;
        }
    }

    const nlohmann::json* arrayForSelectionKind(const SelectionKind kind) const
    {
        switch (kind)
        {
        case SelectionKind::Platform:
            return &document_["Platforms"];
        case SelectionKind::Decoration:
            return &document_["Decorations"];
        case SelectionKind::Background:
            return &document_["Background"];
        case SelectionKind::Ground:
            return &document_["Ground"];
        case SelectionKind::Spawner:
            return &document_["Spawners"];
        case SelectionKind::Interactive:
            return &document_["Interactives"];
        case SelectionKind::MiniLocation:
            return &document_["MiniLocations"];
        default:
            return nullptr;
        }
    }

    nlohmann::json* selectedPlatform()
    {
        if (selection_.kind != SelectionKind::Platform || selection_.index >= document_["Platforms"].size())
        {
            return nullptr;
        }

        return &document_["Platforms"][selection_.index];
    }

    nlohmann::json* selectedDecoration()
    {
        if (selection_.kind != SelectionKind::Decoration || selection_.index >= document_["Decorations"].size())
        {
            return nullptr;
        }

        return &document_["Decorations"][selection_.index];
    }

    nlohmann::json* selectedGround()
    {
        if (selection_.kind != SelectionKind::Ground || selection_.index >= document_["Ground"].size())
        {
            return nullptr;
        }

        return &document_["Ground"][selection_.index];
    }

    nlohmann::json* selectedInteractive()
    {
        if (selection_.kind != SelectionKind::Interactive || selection_.index >= document_["Interactives"].size())
        {
            return nullptr;
        }

        return &document_["Interactives"][selection_.index];
    }

    nlohmann::json* selectedSpawner()
    {
        if (selection_.kind != SelectionKind::Spawner || selection_.index >= document_["Spawners"].size())
        {
            return nullptr;
        }

        return &document_["Spawners"][selection_.index];
    }

    const nlohmann::json* selectedPlatform() const
    {
        if (selection_.kind != SelectionKind::Platform || selection_.index >= document_["Platforms"].size())
        {
            return nullptr;
        }

        return &document_["Platforms"][selection_.index];
    }

    const nlohmann::json* selectedDecoration() const
    {
        if (selection_.kind != SelectionKind::Decoration || selection_.index >= document_["Decorations"].size())
        {
            return nullptr;
        }

        return &document_["Decorations"][selection_.index];
    }

    const nlohmann::json* selectedInteractive() const
    {
        if (selection_.kind != SelectionKind::Interactive || selection_.index >= document_["Interactives"].size())
        {
            return nullptr;
        }

        return &document_["Interactives"][selection_.index];
    }

    const nlohmann::json* selectedSpawner() const
    {
        if (selection_.kind != SelectionKind::Spawner || selection_.index >= document_["Spawners"].size())
        {
            return nullptr;
        }

        return &document_["Spawners"][selection_.index];
    }

    bool interactiveUsesAutoNameplate(const nlohmann::json& interactive) const
    {
        if (interactive.contains("AutoNameplate"))
        {
            return interactive["AutoNameplate"].get<bool>();
        }

        return interactive.value("Type", std::string{"EchoTablet"}) != "CustomSign";
    }

    int interactiveTypeSeedValue(const std::string& typeName) const
    {
        if (typeName == "RestShrine")
        {
            return 0;
        }
        if (typeName == "GoldCache")
        {
            return 2;
        }

        return 1;
    }

    std::string pickAutomaticNameplateTexture(const nlohmann::json& interactive) const
    {
        if (nameplateTextureOptions_.empty())
        {
            return interactive.value("Texture", std::string{});
        }

        const std::string seed =
            std::to_string(interactiveTypeSeedValue(interactive.value("Type", std::string{"EchoTablet"})))
            + "|"
            + interactive.value("Title", std::string{})
            + "|"
            + std::to_string(static_cast<int>(std::round(readVector2f(interactive.value("Position", nlohmann::json::array())).x)))
            + "|"
            + std::to_string(static_cast<int>(std::round(readVector2f(interactive.value("Position", nlohmann::json::array())).y)));

        const std::size_t textureIndex = hashNameplateSeed(seed) % nameplateTextureOptions_.size();
        return nameplateTextureOptions_[textureIndex];
    }

    std::string interactivePreviewTextureName(const nlohmann::json& interactive) const
    {
        if (interactiveUsesAutoNameplate(interactive))
        {
            return pickAutomaticNameplateTexture(interactive);
        }

        return interactive.value("Texture", std::string{});
    }

    sf::Vector2f interactivePreviewScale(const nlohmann::json& interactive) const
    {
        if (interactiveUsesAutoNameplate(interactive))
        {
            return kWorldNameplateScale;
        }

        return readVector2f(interactive.value("Scale", nlohmann::json::array()), {1.f, 1.f});
    }

    sf::Vector2f interactiveBaseSize(const nlohmann::json& interactive) const
    {
        const sf::Texture* texture = findPreviewTexture(interactivePreviewTextureName(interactive));
        if (texture == nullptr)
        {
            return {84.f, 84.f};
        }

        return {
            static_cast<float>(texture->getSize().x),
            static_cast<float>(texture->getSize().y)
        };
    }

    std::optional<Platform::TypeDefinition> platformDefinition(const nlohmann::json& platform) const
    {
        return Platform::getTypeDefinition(platform.value("Type", std::string{}));
    }

    bool platformEditHitboxEnabled(const nlohmann::json& platform) const
    {
        return platform.value("EditHitbox", false);
    }

    sf::Vector2f platformHitboxOffset(const nlohmann::json& platform) const
    {
        return readVector2f(platform.value("HitboxOffset", nlohmann::json::array()), {0.f, 0.f});
    }

    sf::Vector2f platformBasePosition(const nlohmann::json& platform) const
    {
        return readVector2f(platform.value("Position", nlohmann::json::array()));
    }

    sf::Vector2f platformHitboxSize(const nlohmann::json& platform) const
    {
        const auto definition = platformDefinition(platform);
        const sf::Vector2f fallback = definition.has_value() ? definition->hitboxSize : sf::Vector2f{96.f, 32.f};
        const sf::Vector2f value = readVector2f(platform.value("HitboxSize", nlohmann::json::array()), fallback);
        return {
            std::max(value.x, kPlatformMinHitboxSize),
            std::max(value.y, kPlatformMinHitboxSize)
        };
    }

    sf::Vector2f platformScale(const nlohmann::json& platform) const
    {
        const auto definition = platformDefinition(platform);
        const sf::Vector2f fallback = definition.has_value() ? definition->spriteScale : sf::Vector2f{1.f, 1.f};
        const sf::Vector2f value = readVector2f(platform.value("Scale", nlohmann::json::array()), fallback);
        return {
            std::max(std::abs(value.x), kPlatformMinScale),
            std::max(std::abs(value.y), kPlatformMinScale)
        };
    }

    std::string platformAtmosphereMode(const nlohmann::json& platform) const
    {
        return platform.value("AtmosphereStyle", std::string{"Default"});
    }

    sf::Color platformAtmosphereColor(const nlohmann::json& platform) const
    {
        const auto definition = platformDefinition(platform);
        const sf::Color fallback = definition.has_value() ? definition->atmosphereColor : sf::Color::Transparent;
        return readColor(platform.value("AtmosphereColor", nlohmann::json::array()), fallback);
    }

    float platformAtmosphereDensity(const nlohmann::json& platform) const
    {
        const auto definition = platformDefinition(platform);
        return std::max(platform.value("AtmosphereDensity", definition.has_value() ? definition->atmosphereDensity : 1.f), 0.f);
    }

    sf::FloatRect platformBounds(const nlohmann::json& platform) const
    {
        return sf::FloatRect(platformBasePosition(platform) + platformHitboxOffset(platform), platformHitboxSize(platform));
    }

    sf::Vector2f platformSpriteCenter(const nlohmann::json& platform) const
    {
        const auto definition = platformDefinition(platform);
        const sf::Vector2f anchorSize = definition.has_value() ? definition->hitboxSize : platformHitboxSize(platform);
        return platformBasePosition(platform) + anchorSize * 0.5f;
    }

    sf::FloatRect platformSpriteBounds(const nlohmann::json& platform) const
    {
        const auto definition = platformDefinition(platform);
        if (!definition.has_value() || definition->texturePath.empty())
        {
            return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
        }

        return textureBoundsAt(
            platform.value("Type", std::string{}),
            platformSpriteCenter(platform) + definition->spriteOffset,
            platformScale(platform),
            platformHitboxSize(platform)
        );
    }

    sf::FloatRect platformHandleBounds(const sf::Vector2f corner) const
    {
        return sf::FloatRect(
            {corner.x - kPlatformHandleSize * 0.5f, corner.y - kPlatformHandleSize * 0.5f},
            {kPlatformHandleSize, kPlatformHandleSize}
        );
    }

    sf::FloatRect platformHitboxHandleBounds(const nlohmann::json& platform) const
    {
        const sf::FloatRect bounds = platformBounds(platform);
        return platformHandleBounds(bounds.position + bounds.size);
    }

    sf::FloatRect platformScaleHandleBounds(const nlohmann::json& platform) const
    {
        const sf::FloatRect bounds = platformSpriteBounds(platform);
        if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
        {
            return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
        }

        return platformHandleBounds(bounds.position + bounds.size);
    }

    bool resizeZoneContains(const sf::FloatRect bounds, const sf::Vector2f worldPosition) const
    {
        if (!bounds.contains(worldPosition))
        {
            return false;
        }

        const float resizeThreshold = kPlatformHandleSize * 1.15f;
        const float distanceToRight = bounds.position.x + bounds.size.x - worldPosition.x;
        const float distanceToBottom = bounds.position.y + bounds.size.y - worldPosition.y;
        return distanceToRight <= resizeThreshold || distanceToBottom <= resizeThreshold;
    }

    bool platformContainsPoint(const nlohmann::json& platform, const sf::Vector2f worldPosition) const
    {
        const sf::FloatRect hitboxBounds = platformBounds(platform);
        if (hitboxBounds.contains(worldPosition))
        {
            return true;
        }

        const sf::FloatRect spriteBounds = platformSpriteBounds(platform);
        return spriteBounds.size.x > 0.f && spriteBounds.size.y > 0.f && spriteBounds.contains(worldPosition);
    }

    std::optional<std::size_t> findPlatformAt(const sf::Vector2f worldPosition) const
    {
        for (std::size_t index = document_["Platforms"].size(); index > 0u; --index)
        {
            if (platformContainsPoint(document_["Platforms"][index - 1u], worldPosition))
            {
                return index - 1u;
            }
        }

        return std::nullopt;
    }

    bool beginSpawnInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None)
        {
            return false;
        }

        const bool canEditSpawn = activeTab_ == EditorTab::Level || selection_.kind == SelectionKind::Spawn;
        if (!canEditSpawn)
        {
            return false;
        }

        const sf::Vector2f spawn = readVector2f(document_["Presets"].value("PlayerSpawn", nlohmann::json::array()), {200.f, 900.f});
        const sf::FloatRect spawnBounds({spawn.x - 24.f, spawn.y - 48.f}, {48.f, 48.f});
        if (!spawnBounds.contains(worldPosition))
        {
            return false;
        }

        selection_ = {SelectionKind::Spawn, 0u};
        spawnInteraction_.active = true;
        return true;
    }

    void updateSpawnInteraction(const sf::Vector2f worldPosition)
    {
        if (!spawnInteraction_.active)
        {
            return;
        }

        document_["Presets"]["PlayerSpawn"] = toJson(worldPosition);
        markDirty();
    }

    void finishSpawnInteraction()
    {
        spawnInteraction_.clear();
    }

    bool beginPlatformInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None)
        {
            return false;
        }

        if (const nlohmann::json* currentPlatform = selectedPlatform(); currentPlatform != nullptr)
        {
            const bool editHitbox = platformEditHitboxEnabled(*currentPlatform);
            const sf::FloatRect hitboxBounds = platformBounds(*currentPlatform);
            const sf::FloatRect spriteBounds = platformSpriteBounds(*currentPlatform);

            if (editHitbox &&
                (platformHitboxHandleBounds(*currentPlatform).contains(worldPosition) ||
                 resizeZoneContains(hitboxBounds, worldPosition)))
            {
                platformInteraction_.mode = PlatformInteractionMode::ResizeHitbox;
                platformInteraction_.index = selection_.index;
            }
            else if (platformScaleHandleBounds(*currentPlatform).contains(worldPosition) ||
                     resizeZoneContains(spriteBounds, worldPosition))
            {
                platformInteraction_.mode = PlatformInteractionMode::ResizeScale;
                platformInteraction_.index = selection_.index;
            }
            else if (editHitbox && hitboxBounds.contains(worldPosition))
            {
                platformInteraction_.mode = PlatformInteractionMode::MoveHitbox;
                platformInteraction_.index = selection_.index;
            }
            else if (platformContainsPoint(*currentPlatform, worldPosition))
            {
                platformInteraction_.mode = PlatformInteractionMode::Move;
                platformInteraction_.index = selection_.index;
            }
        }

        if (!platformInteraction_.active())
        {
            const std::optional<std::size_t> platformIndex = findPlatformAt(worldPosition);
            if (!platformIndex.has_value())
            {
                return false;
            }

            selection_ = {SelectionKind::Platform, *platformIndex};
            platformInteraction_.mode = PlatformInteractionMode::Move;
            platformInteraction_.index = *platformIndex;
        }

        const nlohmann::json& platform = document_["Platforms"][platformInteraction_.index];
        platformInteraction_.startWorld = worldPosition;
        platformInteraction_.startPosition = platformBasePosition(platform);
        platformInteraction_.startHitboxOffset = platformHitboxOffset(platform);
        platformInteraction_.startHitboxSize = platformHitboxSize(platform);
        platformInteraction_.startScale = platformScale(platform);
        platformInteraction_.startSpriteHalfSize = platformSpriteBounds(platform).size * 0.5f;
        return true;
    }

    void updatePlatformInteraction(const sf::Vector2f worldPosition)
    {
        if (!platformInteraction_.active() || platformInteraction_.index >= document_["Platforms"].size())
        {
            platformInteraction_.clear();
            return;
        }

        auto& platform = document_["Platforms"][platformInteraction_.index];
        const sf::Vector2f delta = worldPosition - platformInteraction_.startWorld;

        switch (platformInteraction_.mode)
        {
        case PlatformInteractionMode::Move:
            platform["Position"] = toJson(platformInteraction_.startPosition + delta);
            break;
        case PlatformInteractionMode::MoveHitbox:
            platform["HitboxOffset"] = toJson(platformInteraction_.startHitboxOffset + delta);
            break;
        case PlatformInteractionMode::ResizeHitbox:
            platform["HitboxSize"] = toJson(sf::Vector2f{
                std::max(platformInteraction_.startHitboxSize.x + delta.x, kPlatformMinHitboxSize),
                std::max(platformInteraction_.startHitboxSize.y + delta.y, kPlatformMinHitboxSize)
            });
            break;
        case PlatformInteractionMode::ResizeScale:
        {
            const std::string typeName = platform.value("Type", std::string{});
            const sf::Texture* texture = findPreviewTexture(typeName);
            if (texture == nullptr)
            {
                break;
            }

            const sf::Vector2f textureSize = {
                static_cast<float>(texture->getSize().x),
                static_cast<float>(texture->getSize().y)
            };
            const sf::Vector2f newHalfSize = {
                std::max(platformInteraction_.startSpriteHalfSize.x + delta.x, textureSize.x * kPlatformMinScale * 0.5f),
                std::max(platformInteraction_.startSpriteHalfSize.y + delta.y, textureSize.y * kPlatformMinScale * 0.5f)
            };

            platform["Scale"] = toJson(sf::Vector2f{
                std::max((newHalfSize.x * 2.f) / std::max(textureSize.x, 1.f), kPlatformMinScale),
                std::max((newHalfSize.y * 2.f) / std::max(textureSize.y, 1.f), kPlatformMinScale)
            });
            break;
        }
        default:
            break;
        }

        markDirty();
    }

    void finishPlatformInteraction()
    {
        platformInteraction_.clear();
    }

    sf::Vector2f decorationParallax(const nlohmann::json& decoration) const
    {
        return readVector2f(decoration.value("ParallaxFactor", nlohmann::json::array()), {1.f, 1.f});
    }

    sf::Vector2f decorationDisplayPosition(const nlohmann::json& decoration) const
    {
        return applyParallaxPreview(
            readVector2f(decoration.value("Position", nlohmann::json::array())),
            decorationParallax(decoration)
        );
    }

    sf::FloatRect decorationBounds(const nlohmann::json& decoration) const
    {
        return textureBoundsAt(
            decoration.value("Name", std::string{}),
            decorationDisplayPosition(decoration),
            decorationScale(decoration),
            {72.f, 72.f}
        );
    }

    sf::Vector2f decorationBaseSize(const nlohmann::json& decoration) const
    {
        const sf::Texture* texture = findPreviewTexture(decoration.value("Name", std::string{}));
        if (texture == nullptr)
        {
            return {72.f, 72.f};
        }

        return {
            static_cast<float>(texture->getSize().x),
            static_cast<float>(texture->getSize().y)
        };
    }

    sf::Vector2f decorationScale(const nlohmann::json& decoration) const
    {
        sf::Vector2f value = readVector2f(decoration.value("Scale", nlohmann::json::array()), {1.f, 1.f});
        if (std::abs(value.x) < kDecorationMinScale)
        {
            value.x = (value.x < 0.f ? -1.f : 1.f) * kDecorationMinScale;
        }
        if (std::abs(value.y) < kDecorationMinScale)
        {
            value.y = (value.y < 0.f ? -1.f : 1.f) * kDecorationMinScale;
        }

        return value;
    }

    sf::FloatRect decorationScaleHandleBounds(const nlohmann::json& decoration) const
    {
        const sf::FloatRect bounds = decorationBounds(decoration);
        return platformHandleBounds(bounds.position + bounds.size);
    }

    bool decorationContainsPoint(const nlohmann::json& decoration, const sf::Vector2f worldPosition) const
    {
        return decorationBounds(decoration).contains(worldPosition);
    }

    std::optional<std::size_t> findDecorationAt(const sf::Vector2f worldPosition) const
    {
        for (std::size_t index = document_["Decorations"].size(); index > 0u; --index)
        {
            if (decorationContainsPoint(document_["Decorations"][index - 1u], worldPosition))
            {
                return index - 1u;
            }
        }

        return std::nullopt;
    }

    bool beginDecorationInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None)
        {
            return false;
        }

        if (const nlohmann::json* currentDecoration = selectedDecoration(); currentDecoration != nullptr)
        {
            if (decorationScaleHandleBounds(*currentDecoration).contains(worldPosition) ||
                resizeZoneContains(decorationBounds(*currentDecoration), worldPosition))
            {
                decorationInteraction_.mode = DecorationInteractionMode::ResizeScale;
                decorationInteraction_.index = selection_.index;
            }
        }

        if (!decorationInteraction_.active())
        {
            const std::optional<std::size_t> decorationIndex = findDecorationAt(worldPosition);
            if (!decorationIndex.has_value())
            {
                return false;
            }

            selection_ = {SelectionKind::Decoration, *decorationIndex};
            decorationInteraction_.mode = DecorationInteractionMode::Move;
            decorationInteraction_.index = *decorationIndex;
        }

        const nlohmann::json& decoration = document_["Decorations"][decorationInteraction_.index];
        decorationInteraction_.startWorld = worldPosition;
        decorationInteraction_.startPosition = decorationDisplayPosition(decoration);
        decorationInteraction_.startScale = decorationScale(decoration);
        decorationInteraction_.startSpriteHalfSize = decorationBounds(decoration).size * 0.5f;
        return true;
    }

    void updateDecorationInteraction(const sf::Vector2f worldPosition)
    {
        if (!decorationInteraction_.active() || decorationInteraction_.index >= document_["Decorations"].size())
        {
            decorationInteraction_.clear();
            return;
        }

        auto& decoration = document_["Decorations"][decorationInteraction_.index];
        const sf::Vector2f delta = worldPosition - decorationInteraction_.startWorld;

        switch (decorationInteraction_.mode)
        {
        case DecorationInteractionMode::Move:
            decoration["Position"] = toJson(removeParallaxPreview(
                decorationInteraction_.startPosition + delta,
                decorationParallax(decoration)
            ));
            break;
        case DecorationInteractionMode::ResizeScale:
        {
            const sf::Vector2f baseSize = decorationBaseSize(decoration);
            const sf::Vector2f newHalfSize = {
                std::max(decorationInteraction_.startSpriteHalfSize.x + delta.x, baseSize.x * kDecorationMinScale * 0.5f),
                std::max(decorationInteraction_.startSpriteHalfSize.y + delta.y, baseSize.y * kDecorationMinScale * 0.5f)
            };
            const float signX = decorationInteraction_.startScale.x < 0.f ? -1.f : 1.f;
            const float signY = decorationInteraction_.startScale.y < 0.f ? -1.f : 1.f;
            decoration["Scale"] = toJson(sf::Vector2f{
                signX * std::max((newHalfSize.x * 2.f) / std::max(baseSize.x, 1.f), kDecorationMinScale),
                signY * std::max((newHalfSize.y * 2.f) / std::max(baseSize.y, 1.f), kDecorationMinScale)
            });
            break;
        }
        default:
            break;
        }

        markDirty();
    }

    void finishDecorationInteraction()
    {
        decorationInteraction_.clear();
    }

    sf::Vector2f backgroundParallax(const nlohmann::json& background) const
    {
        return readVector2f(background.value("ParallaxFactor", nlohmann::json::array()), {0.08f, 0.06f});
    }

    sf::Vector2f backgroundDisplayPosition(const nlohmann::json& background) const
    {
        return applyParallaxPreview(
            readVector2f(background.value("Position", nlohmann::json::array()), {960.f, 540.f}),
            backgroundParallax(background)
        );
    }

    sf::FloatRect backgroundBounds(const nlohmann::json& background) const
    {
        const sf::Vector2f center = backgroundDisplayPosition(background);
        return sf::FloatRect(
            {center.x - WINDOW_WIDTH * 0.5f, center.y - WINDOW_HEIGHT * 0.5f},
            {static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)}
        );
    }

    sf::FloatRect spawnerBounds(const nlohmann::json& spawner) const
    {
        if (!spawner.contains("SpawnArea") || !spawner["SpawnArea"].is_array() || spawner["SpawnArea"].size() < 2)
        {
            return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
        }

        const auto& spawnArea = spawner["SpawnArea"];
        const float minX = spawnArea[0][0].get<float>();
        const float maxX = spawnArea[0][1].get<float>();
        const float minY = spawnArea[1][0].get<float>();
        const float maxY = spawnArea[1][1].get<float>();
        return sf::FloatRect({minX, minY}, {maxX - minX, maxY - minY});
    }

    sf::FloatRect interactiveBounds(const nlohmann::json& interactive) const
    {
        return textureBoundsAt(
            interactivePreviewTextureName(interactive),
            readVector2f(interactive.value("Position", nlohmann::json::array())),
            interactivePreviewScale(interactive),
            {84.f, 84.f}
        );
    }

    sf::FloatRect interactiveScaleHandleBounds(const nlohmann::json& interactive) const
    {
        return platformHandleBounds(interactiveBounds(interactive).position + interactiveBounds(interactive).size);
    }

    std::optional<std::size_t> findInteractiveAt(const sf::Vector2f worldPosition) const
    {
        for (std::size_t index = document_["Interactives"].size(); index > 0u; --index)
        {
            if (interactiveBounds(document_["Interactives"][index - 1u]).contains(worldPosition))
            {
                return index - 1u;
            }
        }

        return std::nullopt;
    }

    bool beginInteractiveInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None)
        {
            return false;
        }

        if (const nlohmann::json* currentInteractive = selectedInteractive(); currentInteractive != nullptr)
        {
            if (interactiveScaleHandleBounds(*currentInteractive).contains(worldPosition) ||
                resizeZoneContains(interactiveBounds(*currentInteractive), worldPosition))
            {
                interactiveInteraction_.mode = InteractiveInteractionMode::ResizeScale;
                interactiveInteraction_.index = selection_.index;
            }
            else if (interactiveBounds(*currentInteractive).contains(worldPosition))
            {
                interactiveInteraction_.mode = InteractiveInteractionMode::Move;
                interactiveInteraction_.index = selection_.index;
            }
        }

        if (!interactiveInteraction_.active())
        {
            const std::optional<std::size_t> interactiveIndex = findInteractiveAt(worldPosition);
            if (!interactiveIndex.has_value())
            {
                return false;
            }

            selection_ = {SelectionKind::Interactive, *interactiveIndex};
            interactiveInteraction_.mode = InteractiveInteractionMode::Move;
            interactiveInteraction_.index = *interactiveIndex;
        }

        const nlohmann::json& interactive = document_["Interactives"][interactiveInteraction_.index];
        interactiveInteraction_.startWorld = worldPosition;
        interactiveInteraction_.startPosition = readVector2f(interactive.value("Position", nlohmann::json::array()));
        interactiveInteraction_.startScale = interactivePreviewScale(interactive);
        interactiveInteraction_.startSpriteHalfSize = interactiveBounds(interactive).size * 0.5f;
        return true;
    }

    void updateInteractiveInteraction(const sf::Vector2f worldPosition)
    {
        if (!interactiveInteraction_.active() || interactiveInteraction_.index >= document_["Interactives"].size())
        {
            interactiveInteraction_.clear();
            return;
        }

        auto& interactive = document_["Interactives"][interactiveInteraction_.index];
        const sf::Vector2f delta = worldPosition - interactiveInteraction_.startWorld;

        switch (interactiveInteraction_.mode)
        {
        case InteractiveInteractionMode::Move:
            interactive["Position"] = toJson(interactiveInteraction_.startPosition + delta);
            break;
        case InteractiveInteractionMode::ResizeScale:
        {
            if (interactiveUsesAutoNameplate(interactive))
            {
                interactive["AutoNameplate"] = false;
                interactive["Texture"] = pickAutomaticNameplateTexture(interactive);
                interactive["Scale"] = toJson(interactiveInteraction_.startScale);
            }

            const sf::Vector2f baseSize = interactiveBaseSize(interactive);
            const sf::Vector2f newHalfSize = {
                std::max(interactiveInteraction_.startSpriteHalfSize.x + delta.x, baseSize.x * kDecorationMinScale * 0.5f),
                std::max(interactiveInteraction_.startSpriteHalfSize.y + delta.y, baseSize.y * kDecorationMinScale * 0.5f)
            };
            const float signX = interactiveInteraction_.startScale.x < 0.f ? -1.f : 1.f;
            const float signY = interactiveInteraction_.startScale.y < 0.f ? -1.f : 1.f;
            interactive["Scale"] = toJson(sf::Vector2f{
                signX * std::max((newHalfSize.x * 2.f) / std::max(baseSize.x, 1.f), kDecorationMinScale),
                signY * std::max((newHalfSize.y * 2.f) / std::max(baseSize.y, 1.f), kDecorationMinScale)
            });
            break;
        }
        case InteractiveInteractionMode::None:
        default:
            break;
        }

        markDirty();
    }

    void finishInteractiveInteraction()
    {
        interactiveInteraction_.clear();
    }

    sf::FloatRect spawnerResizeHandleBounds(const nlohmann::json& spawner) const
    {
        return platformHandleBounds(spawnerBounds(spawner).position + spawnerBounds(spawner).size);
    }

    std::optional<std::size_t> findSpawnerAt(const sf::Vector2f worldPosition) const
    {
        for (std::size_t index = document_["Spawners"].size(); index > 0u; --index)
        {
            if (spawnerBounds(document_["Spawners"][index - 1u]).contains(worldPosition))
            {
                return index - 1u;
            }
        }

        return std::nullopt;
    }

    bool beginSpawnerInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None)
        {
            return false;
        }

        if (const nlohmann::json* currentSpawner = selectedSpawner(); currentSpawner != nullptr)
        {
            if (spawnerResizeHandleBounds(*currentSpawner).contains(worldPosition) ||
                resizeZoneContains(spawnerBounds(*currentSpawner), worldPosition))
            {
                spawnerInteraction_.mode = SpawnerInteractionMode::Resize;
                spawnerInteraction_.index = selection_.index;
            }
            else if (spawnerBounds(*currentSpawner).contains(worldPosition))
            {
                spawnerInteraction_.mode = SpawnerInteractionMode::Move;
                spawnerInteraction_.index = selection_.index;
            }
        }

        if (!spawnerInteraction_.active())
        {
            const std::optional<std::size_t> spawnerIndex = findSpawnerAt(worldPosition);
            if (!spawnerIndex.has_value())
            {
                return false;
            }

            selection_ = {SelectionKind::Spawner, *spawnerIndex};
            spawnerInteraction_.mode = SpawnerInteractionMode::Move;
            spawnerInteraction_.index = *spawnerIndex;
        }

        spawnerInteraction_.startWorld = worldPosition;
        spawnerInteraction_.startBounds = spawnerBounds(document_["Spawners"][spawnerInteraction_.index]);
        return true;
    }

    void updateSpawnerInteraction(const sf::Vector2f worldPosition)
    {
        if (!spawnerInteraction_.active() || spawnerInteraction_.index >= document_["Spawners"].size())
        {
            spawnerInteraction_.clear();
            return;
        }

        auto& spawner = document_["Spawners"][spawnerInteraction_.index];
        const sf::Vector2f delta = worldPosition - spawnerInteraction_.startWorld;
        sf::FloatRect bounds = spawnerInteraction_.startBounds;

        if (spawnerInteraction_.mode == SpawnerInteractionMode::Move)
        {
            bounds.position += delta;
        }
        else if (spawnerInteraction_.mode == SpawnerInteractionMode::Resize)
        {
            bounds.size = {
                std::max(bounds.size.x + delta.x, 24.f),
                std::max(bounds.size.y + delta.y, 24.f)
            };
        }

        spawner["SpawnArea"] = nlohmann::json::array({
            nlohmann::json::array({bounds.position.x, bounds.position.x + bounds.size.x}),
            nlohmann::json::array({bounds.position.y, bounds.position.y + bounds.size.y})
        });
        markDirty();
    }

    void finishSpawnerInteraction()
    {
        spawnerInteraction_.clear();
    }

    sf::FloatRect miniLocationBounds(const nlohmann::json& location) const
    {
        return readRect(location.value("Bounds", nlohmann::json::array()));
    }

    sf::FloatRect miniLocationResizeHandleBounds(const nlohmann::json& location) const
    {
        const sf::FloatRect bounds = miniLocationBounds(location);
        return platformHandleBounds(bounds.position + bounds.size);
    }

    std::optional<std::size_t> findMiniLocationAt(const sf::Vector2f worldPosition) const
    {
        for (std::size_t index = document_["MiniLocations"].size(); index > 0u; --index)
        {
            if (miniLocationBounds(document_["MiniLocations"][index - 1u]).contains(worldPosition))
            {
                return index - 1u;
            }
        }

        return std::nullopt;
    }

    bool beginMiniLocationInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None)
        {
            return false;
        }

        if (selection_.kind == SelectionKind::MiniLocation && selection_.index < document_["MiniLocations"].size())
        {
            const nlohmann::json& currentLocation = document_["MiniLocations"][selection_.index];
            if (miniLocationResizeHandleBounds(currentLocation).contains(worldPosition) ||
                resizeZoneContains(miniLocationBounds(currentLocation), worldPosition))
            {
                miniLocationInteraction_.mode = MiniLocationInteractionMode::Resize;
                miniLocationInteraction_.index = selection_.index;
            }
            else if (miniLocationBounds(currentLocation).contains(worldPosition))
            {
                miniLocationInteraction_.mode = MiniLocationInteractionMode::Move;
                miniLocationInteraction_.index = selection_.index;
            }
        }

        if (!miniLocationInteraction_.active())
        {
            const std::optional<std::size_t> miniLocationIndex = findMiniLocationAt(worldPosition);
            if (!miniLocationIndex.has_value())
            {
                return false;
            }

            selection_ = {SelectionKind::MiniLocation, *miniLocationIndex};
            miniLocationInteraction_.mode = MiniLocationInteractionMode::Move;
            miniLocationInteraction_.index = *miniLocationIndex;
        }

        miniLocationInteraction_.startWorld = worldPosition;
        miniLocationInteraction_.startBounds = miniLocationBounds(document_["MiniLocations"][miniLocationInteraction_.index]);
        return true;
    }

    void updateMiniLocationInteraction(const sf::Vector2f worldPosition)
    {
        if (!miniLocationInteraction_.active() || miniLocationInteraction_.index >= document_["MiniLocations"].size())
        {
            miniLocationInteraction_.clear();
            return;
        }

        auto& location = document_["MiniLocations"][miniLocationInteraction_.index];
        const sf::Vector2f delta = worldPosition - miniLocationInteraction_.startWorld;

        if (miniLocationInteraction_.mode == MiniLocationInteractionMode::Move)
        {
            location["Bounds"] = toJson(sf::FloatRect{
                miniLocationInteraction_.startBounds.position + delta,
                miniLocationInteraction_.startBounds.size
            });

            auto offsetVectorField = [&](nlohmann::json& object, const char* key) {
                object[key] = toJson(readVector2f(object.value(key, nlohmann::json::array())) + delta);
            };

            if (location.contains("Entry") && location["Entry"].is_object())
            {
                offsetVectorField(location["Entry"], "Position");
                offsetVectorField(location["Entry"], "DestinationSupport");
            }

            if (location.contains("Exit") && location["Exit"].is_object())
            {
                offsetVectorField(location["Exit"], "Position");
                offsetVectorField(location["Exit"], "DestinationSupport");
            }

            if (location.contains("Hazard") && location["Hazard"].is_object())
            {
                sf::FloatRect hazardRect = readRect(location["Hazard"].value("Rect", nlohmann::json::array()));
                hazardRect.position += delta;
                location["Hazard"]["Rect"] = toJson(hazardRect);
            }
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::Resize)
        {
            sf::FloatRect bounds = miniLocationInteraction_.startBounds;
            bounds.size = {
                std::max(bounds.size.x + delta.x, 64.f),
                std::max(bounds.size.y + delta.y, 64.f)
            };
            location["Bounds"] = toJson(bounds);
        }

        markDirty();
    }

    void finishMiniLocationInteraction()
    {
        miniLocationInteraction_.clear();
    }

    sf::FloatRect groundBounds(const nlohmann::json& ground) const
    {
        const float levelHeight = document_["Presets"]["Size"][1].get<float>();
        const float startX = ground["Points"][0].get<float>();
        const float endX = ground["Points"][1].get<float>();
        const float yPos = ground.value("YPos", 980.f);
        const float offset = ground.value("Offset", 8.f);
        return sf::FloatRect({startX, yPos + offset}, {endX - startX, levelHeight - (yPos + offset)});
    }

    sf::FloatRect groundResizeHandleBounds(const nlohmann::json& ground) const
    {
        const sf::FloatRect bounds = groundBounds(ground);
        return platformHandleBounds({bounds.position.x + bounds.size.x, bounds.position.y});
    }

    bool groundResizeZoneContains(const sf::FloatRect bounds, const sf::Vector2f worldPosition) const
    {
        if (!bounds.contains(worldPosition))
        {
            return false;
        }

        const float resizeThreshold = kPlatformHandleSize * 1.15f;
        const float distanceToRight = bounds.position.x + bounds.size.x - worldPosition.x;
        const float distanceToTop = worldPosition.y - bounds.position.y;
        return distanceToRight <= resizeThreshold || distanceToTop <= resizeThreshold;
    }

    std::optional<std::size_t> findGroundAt(const sf::Vector2f worldPosition) const
    {
        for (std::size_t index = document_["Ground"].size(); index > 0u; --index)
        {
            if (groundBounds(document_["Ground"][index - 1u]).contains(worldPosition))
            {
                return index - 1u;
            }
        }

        return std::nullopt;
    }

    bool beginGroundInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None)
        {
            return false;
        }

        if (selection_.kind == SelectionKind::Ground && selection_.index < document_["Ground"].size())
        {
            const nlohmann::json& currentGround = document_["Ground"][selection_.index];
            const sf::FloatRect bounds = groundBounds(currentGround);
            if (groundResizeHandleBounds(currentGround).contains(worldPosition) ||
                groundResizeZoneContains(bounds, worldPosition))
            {
                groundInteraction_.mode = GroundInteractionMode::Resize;
                groundInteraction_.index = selection_.index;
            }
            else if (bounds.contains(worldPosition))
            {
                groundInteraction_.mode = GroundInteractionMode::Move;
                groundInteraction_.index = selection_.index;
            }
        }

        if (!groundInteraction_.active())
        {
            const std::optional<std::size_t> groundIndex = findGroundAt(worldPosition);
            if (!groundIndex.has_value())
            {
                return false;
            }

            selection_ = {SelectionKind::Ground, *groundIndex};
            groundInteraction_.mode = GroundInteractionMode::Move;
            groundInteraction_.index = *groundIndex;
        }

        const auto& ground = document_["Ground"][groundInteraction_.index];
        groundInteraction_.startWorld = worldPosition;
        groundInteraction_.startStartX = ground["Points"][0].get<float>();
        groundInteraction_.startEndX = ground["Points"][1].get<float>();
        groundInteraction_.startYPos = ground.value("YPos", 980.f);
        groundInteraction_.startOffset = ground.value("Offset", 8.f);
        return true;
    }

    void updateGroundInteraction(const sf::Vector2f worldPosition)
    {
        if (!groundInteraction_.active() || groundInteraction_.index >= document_["Ground"].size())
        {
            groundInteraction_.clear();
            return;
        }

        auto& ground = document_["Ground"][groundInteraction_.index];
        const sf::Vector2f delta = worldPosition - groundInteraction_.startWorld;
        const float levelHeight = document_["Presets"]["Size"][1].get<float>();
        float startX = groundInteraction_.startStartX;
        float endX = groundInteraction_.startEndX;
        float yPos = groundInteraction_.startYPos;

        if (groundInteraction_.mode == GroundInteractionMode::Move)
        {
            startX += delta.x;
            endX += delta.x;
            yPos += delta.y;
        }
        else if (groundInteraction_.mode == GroundInteractionMode::Resize)
        {
            endX = std::max(groundInteraction_.startEndX + delta.x, groundInteraction_.startStartX + 64.f);
            const float minTop = 0.f;
            const float maxTop = levelHeight - 16.f;
            const float newTop = std::clamp(
                groundInteraction_.startYPos + groundInteraction_.startOffset + delta.y,
                minTop,
                maxTop
            );
            yPos = newTop - groundInteraction_.startOffset;
        }

        ground["Points"] = nlohmann::json::array({
            static_cast<int>(std::round(std::min(startX, endX))),
            static_cast<int>(std::round(std::max(startX, endX)))
        });
        ground["YPos"] = static_cast<int>(std::round(yPos));
        markDirty();
    }

    void finishGroundInteraction()
    {
        groundInteraction_.clear();
    }

    bool selectionContainsPoint(const EditorSelection& selection, const sf::Vector2f worldPosition) const
    {
        switch (selection.kind)
        {
        case SelectionKind::Spawn:
        {
            const sf::Vector2f spawn = readVector2f(document_["Presets"].value("PlayerSpawn", nlohmann::json::array()), {0.f, 0.f});
            return sf::FloatRect({spawn.x - 24.f, spawn.y - 48.f}, {48.f, 48.f}).contains(worldPosition);
        }
        case SelectionKind::Platform:
            return selection.index < document_["Platforms"].size() &&
                platformContainsPoint(document_["Platforms"][selection.index], worldPosition);
        case SelectionKind::Decoration:
            return selection.index < document_["Decorations"].size() &&
                decorationContainsPoint(document_["Decorations"][selection.index], worldPosition);
        case SelectionKind::Background:
            return selection.index < document_["Background"].size() &&
                backgroundBounds(document_["Background"][selection.index]).contains(worldPosition);
        case SelectionKind::Ground:
            return selection.index < document_["Ground"].size() &&
                groundBounds(document_["Ground"][selection.index]).contains(worldPosition);
        case SelectionKind::Spawner:
            return selection.index < document_["Spawners"].size() &&
                spawnerBounds(document_["Spawners"][selection.index]).contains(worldPosition);
        case SelectionKind::Interactive:
            return selection.index < document_["Interactives"].size() &&
                interactiveBounds(document_["Interactives"][selection.index]).contains(worldPosition);
        case SelectionKind::MiniLocation:
            return selection.index < document_["MiniLocations"].size() &&
                miniLocationBounds(document_["MiniLocations"][selection.index]).contains(worldPosition);
        case SelectionKind::None:
        default:
            return false;
        }
    }

    bool selectionUsesDirectManipulationHandle(const EditorSelection& selection, const sf::Vector2f worldPosition) const
    {
        switch (selection.kind)
        {
        case SelectionKind::Platform:
            if (selection.index < document_["Platforms"].size())
            {
                const nlohmann::json& platform = document_["Platforms"][selection.index];
                if (platformScaleHandleBounds(platform).contains(worldPosition) ||
                    resizeZoneContains(platformSpriteBounds(platform), worldPosition))
                {
                    return true;
                }

                if (platformEditHitboxEnabled(platform))
                {
                    const sf::FloatRect hitboxBounds = platformBounds(platform);
                    if (platformHitboxHandleBounds(platform).contains(worldPosition) ||
                        resizeZoneContains(hitboxBounds, worldPosition))
                    {
                        return true;
                    }
                }
            }
            return false;
        case SelectionKind::Decoration:
            return selection.index < document_["Decorations"].size() &&
                (decorationScaleHandleBounds(document_["Decorations"][selection.index]).contains(worldPosition) ||
                 resizeZoneContains(decorationBounds(document_["Decorations"][selection.index]), worldPosition));
        case SelectionKind::Interactive:
            return selection.index < document_["Interactives"].size() &&
                (interactiveScaleHandleBounds(document_["Interactives"][selection.index]).contains(worldPosition) ||
                 resizeZoneContains(interactiveBounds(document_["Interactives"][selection.index]), worldPosition));
        case SelectionKind::Spawner:
            return selection.index < document_["Spawners"].size() &&
                (spawnerResizeHandleBounds(document_["Spawners"][selection.index]).contains(worldPosition) ||
                 resizeZoneContains(spawnerBounds(document_["Spawners"][selection.index]), worldPosition));
        case SelectionKind::Ground:
            return selection.index < document_["Ground"].size() &&
                (groundResizeHandleBounds(document_["Ground"][selection.index]).contains(worldPosition) ||
                 groundResizeZoneContains(groundBounds(document_["Ground"][selection.index]), worldPosition));
        case SelectionKind::MiniLocation:
            return selection.index < document_["MiniLocations"].size() &&
                (miniLocationResizeHandleBounds(document_["MiniLocations"][selection.index]).contains(worldPosition) ||
                 resizeZoneContains(miniLocationBounds(document_["MiniLocations"][selection.index]), worldPosition));
        default:
            return false;
        }
    }

    std::vector<EditorSelection> collectSelectionsAt(const sf::Vector2f worldPosition) const
    {
        std::vector<EditorSelection> candidates;
        candidates.reserve(
            document_["Background"].size() +
            document_["Ground"].size() +
            document_["MiniLocations"].size() +
            document_["Decorations"].size() +
            document_["Platforms"].size() +
            document_["Spawners"].size() +
            document_["Interactives"].size() +
            1u
        );

        const sf::Vector2f spawn = readVector2f(document_["Presets"].value("PlayerSpawn", nlohmann::json::array()), {0.f, 0.f});
        if (sf::FloatRect({spawn.x - 24.f, spawn.y - 48.f}, {48.f, 48.f}).contains(worldPosition))
        {
            candidates.push_back({SelectionKind::Spawn, 0u});
        }

        for (std::size_t index = document_["Interactives"].size(); index > 0u; --index)
        {
            if (interactiveBounds(document_["Interactives"][index - 1u]).contains(worldPosition))
            {
                candidates.push_back({SelectionKind::Interactive, index - 1u});
            }
        }

        for (std::size_t index = document_["Spawners"].size(); index > 0u; --index)
        {
            if (spawnerBounds(document_["Spawners"][index - 1u]).contains(worldPosition))
            {
                candidates.push_back({SelectionKind::Spawner, index - 1u});
            }
        }

        for (std::size_t index = document_["Platforms"].size(); index > 0u; --index)
        {
            if (platformContainsPoint(document_["Platforms"][index - 1u], worldPosition))
            {
                candidates.push_back({SelectionKind::Platform, index - 1u});
            }
        }

        for (std::size_t index = document_["Decorations"].size(); index > 0u; --index)
        {
            if (decorationBounds(document_["Decorations"][index - 1u]).contains(worldPosition))
            {
                candidates.push_back({SelectionKind::Decoration, index - 1u});
            }
        }

        for (std::size_t index = document_["MiniLocations"].size(); index > 0u; --index)
        {
            if (miniLocationBounds(document_["MiniLocations"][index - 1u]).contains(worldPosition))
            {
                candidates.push_back({SelectionKind::MiniLocation, index - 1u});
            }
        }

        for (std::size_t index = document_["Ground"].size(); index > 0u; --index)
        {
            if (groundBounds(document_["Ground"][index - 1u]).contains(worldPosition))
            {
                candidates.push_back({SelectionKind::Ground, index - 1u});
            }
        }

        for (std::size_t index = document_["Background"].size(); index > 0u; --index)
        {
            if (backgroundBounds(document_["Background"][index - 1u]).contains(worldPosition))
            {
                candidates.push_back({SelectionKind::Background, index - 1u});
            }
        }

        return candidates;
    }

    bool startPendingSelectionCycle(const sf::Vector2f worldPosition, const sf::Vector2i pixelPosition)
    {
        if (placementMode_ != SelectionKind::None || !selection_.isValid())
        {
            return false;
        }

        if (!selectionContainsPoint(selection_, worldPosition) ||
            selectionUsesDirectManipulationHandle(selection_, worldPosition))
        {
            return false;
        }

        std::vector<EditorSelection> candidates = collectSelectionsAt(worldPosition);
        if (candidates.size() <= 1u)
        {
            return false;
        }

        const auto currentIt = std::find(candidates.begin(), candidates.end(), selection_);
        if (currentIt == candidates.end())
        {
            return false;
        }

        pendingSelectionCycle_.active = true;
        pendingSelectionCycle_.startPixel = pixelPosition;
        pendingSelectionCycle_.startWorld = worldPosition;
        pendingSelectionCycle_.initialSelection = selection_;
        pendingSelectionCycle_.candidates = std::move(candidates);
        return true;
    }

    void cyclePendingSelection()
    {
        if (!pendingSelectionCycle_.active || pendingSelectionCycle_.candidates.empty())
        {
            pendingSelectionCycle_.clear();
            return;
        }

        const auto currentIt = std::find(
            pendingSelectionCycle_.candidates.begin(),
            pendingSelectionCycle_.candidates.end(),
            pendingSelectionCycle_.initialSelection
        );
        if (currentIt == pendingSelectionCycle_.candidates.end())
        {
            selection_ = pendingSelectionCycle_.candidates.front();
            pendingSelectionCycle_.clear();
            return;
        }

        std::size_t nextIndex = static_cast<std::size_t>(std::distance(pendingSelectionCycle_.candidates.begin(), currentIt));
        nextIndex = (nextIndex + 1u) % pendingSelectionCycle_.candidates.size();
        selection_ = pendingSelectionCycle_.candidates[nextIndex];
        pendingSelectionCycle_.clear();
    }

    bool moveSelectedDrawOrder(const int delta)
    {
        nlohmann::json* array = arrayForSelectionKind(selection_.kind);
        if (array == nullptr || selection_.index >= array->size())
        {
            return false;
        }

        const long long currentIndex = static_cast<long long>(selection_.index);
        const long long newIndex = std::clamp(
            currentIndex + static_cast<long long>(delta),
            0ll,
            static_cast<long long>(array->size()) - 1ll
        );
        if (newIndex == currentIndex)
        {
            return false;
        }

        nlohmann::json object = (*array)[selection_.index];
        array->erase(array->begin() + static_cast<nlohmann::json::difference_type>(selection_.index));
        array->insert(array->begin() + static_cast<nlohmann::json::difference_type>(newIndex), object);
        selection_.index = static_cast<std::size_t>(newIndex);
        markDirty();
        return true;
    }

    bool moveSelectedToDrawOrderEdge(const bool toFront)
    {
        nlohmann::json* array = arrayForSelectionKind(selection_.kind);
        if (array == nullptr || selection_.index >= array->size() || array->empty())
        {
            return false;
        }

        const std::size_t targetIndex = toFront ? array->size() - 1u : 0u;
        if (selection_.index == targetIndex)
        {
            return false;
        }

        nlohmann::json object = (*array)[selection_.index];
        array->erase(array->begin() + static_cast<nlohmann::json::difference_type>(selection_.index));
        array->insert(array->begin() + static_cast<nlohmann::json::difference_type>(targetIndex), object);
        selection_.index = targetIndex;
        markDirty();
        return true;
    }

    void drawSelectedDrawOrderControls(const char* idSuffix)
    {
        const nlohmann::json* array = arrayForSelectionKind(selection_.kind);
        if (array == nullptr || selection_.kind == SelectionKind::Spawn || selection_.index >= array->size())
        {
            return;
        }

        ImGui::SeparatorText("Draw Order");
        ImGui::TextDisabled(
            "Index %zu of %zu. Higher index is drawn on top.",
            selection_.index + 1u,
            array->size()
        );

        const bool canMoveBackward = selection_.index > 0u;
        const bool canMoveForward = selection_.index + 1u < array->size();

        ImGui::BeginDisabled(!canMoveBackward);
        if (ImGui::Button((std::string("To Back##") + idSuffix).c_str()))
        {
            moveSelectedToDrawOrderEdge(false);
        }
        ImGui::SameLine();
        if (ImGui::Button((std::string("Backward##") + idSuffix).c_str()))
        {
            moveSelectedDrawOrder(-1);
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!canMoveForward);
        if (ImGui::Button((std::string("Forward##") + idSuffix).c_str()))
        {
            moveSelectedDrawOrder(1);
        }
        ImGui::SameLine();
        if (ImGui::Button((std::string("To Front##") + idSuffix).c_str()))
        {
            moveSelectedToDrawOrderEdge(true);
        }
        ImGui::EndDisabled();
    }

    bool trySelectSpawnAt(const sf::Vector2f worldPosition)
    {
        const sf::Vector2f spawn = readVector2f(document_["Presets"].value("PlayerSpawn", nlohmann::json::array()), {0.f, 0.f});
        const sf::FloatRect spawnBounds({spawn.x - 24.f, spawn.y - 48.f}, {48.f, 48.f});
        if (!spawnBounds.contains(worldPosition))
        {
            return false;
        }

        selection_.kind = SelectionKind::Spawn;
        selection_.index = 0u;
        return true;
    }

    bool trySelectPlatformAt(const sf::Vector2f worldPosition)
    {
        for (std::size_t index = document_["Platforms"].size(); index > 0u; --index)
        {
            if (platformContainsPoint(document_["Platforms"][index - 1u], worldPosition))
            {
                selection_.kind = SelectionKind::Platform;
                selection_.index = index - 1u;
                return true;
            }
        }

        return false;
    }

    template <typename BoundsBuilder>
    bool trySelectFromArray(nlohmann::json& array, const SelectionKind kind, const sf::Vector2f worldPosition, const BoundsBuilder& boundsBuilder)
    {
        for (std::size_t index = array.size(); index > 0u; --index)
        {
            if (boundsBuilder(array[index - 1u]).contains(worldPosition))
            {
                selection_.kind = kind;
                selection_.index = index - 1u;
                return true;
            }
        }

        return false;
    }

    void selectObjectAt(const sf::Vector2f worldPosition, const bool searchAllKinds = false)
    {
        selection_.clear();

        if (searchAllKinds)
        {
            if (trySelectSpawnAt(worldPosition))
            {
                return;
            }
            if (trySelectFromArray(document_["Interactives"], SelectionKind::Interactive, worldPosition, [&](const auto& value) { return interactiveBounds(value); }))
            {
                return;
            }
            if (trySelectFromArray(document_["Spawners"], SelectionKind::Spawner, worldPosition, [&](const auto& value) { return spawnerBounds(value); }))
            {
                return;
            }
            if (trySelectPlatformAt(worldPosition))
            {
                return;
            }
            if (trySelectFromArray(document_["Decorations"], SelectionKind::Decoration, worldPosition, [&](const auto& value) { return decorationBounds(value); }))
            {
                return;
            }
            if (trySelectFromArray(document_["MiniLocations"], SelectionKind::MiniLocation, worldPosition, [&](const auto& value) {
                    return readRect(value.value("Bounds", nlohmann::json::array()));
                }))
            {
                return;
            }
            if (trySelectFromArray(document_["Ground"], SelectionKind::Ground, worldPosition, [&](const auto& value) {
                    const float levelHeight = document_["Presets"]["Size"][1].get<float>();
                    const float offset = value.value("Offset", 8.f);
                    const float yPos = value.value("YPos", 980.f);
                    const float startX = value["Points"][0].template get<float>();
                    const float endX = value["Points"][1].template get<float>();
                    return sf::FloatRect({startX, yPos + offset}, {endX - startX, levelHeight - (yPos + offset)});
                }))
            {
                return;
            }
            trySelectFromArray(document_["Background"], SelectionKind::Background, worldPosition, [&](const auto& value) { return backgroundBounds(value); });
            return;
        }

        if (activeTab_ == EditorTab::Level)
        {
            trySelectSpawnAt(worldPosition);
            return;
        }

        switch (activeTab_)
        {
        case EditorTab::Platforms:
            trySelectPlatformAt(worldPosition);
            break;
        case EditorTab::Decorations:
            trySelectFromArray(document_["Decorations"], SelectionKind::Decoration, worldPosition, [&](const auto& value) { return decorationBounds(value); });
            break;
        case EditorTab::Backgrounds:
            trySelectFromArray(document_["Background"], SelectionKind::Background, worldPosition, [&](const auto& value) { return backgroundBounds(value); });
            break;
        case EditorTab::Ground:
            trySelectFromArray(document_["Ground"], SelectionKind::Ground, worldPosition, [&](const auto& value) {
                const float levelHeight = document_["Presets"]["Size"][1].get<float>();
                const float offset = value.value("Offset", 8.f);
                const float yPos = value.value("YPos", 980.f);
                const float startX = value["Points"][0].template get<float>();
                const float endX = value["Points"][1].template get<float>();
                return sf::FloatRect({startX, yPos + offset}, {endX - startX, levelHeight - (yPos + offset)});
            });
            break;
        case EditorTab::Spawners:
            trySelectFromArray(document_["Spawners"], SelectionKind::Spawner, worldPosition, [&](const auto& value) { return spawnerBounds(value); });
            break;
        case EditorTab::Interactives:
            trySelectFromArray(document_["Interactives"], SelectionKind::Interactive, worldPosition, [&](const auto& value) { return interactiveBounds(value); });
            break;
        case EditorTab::MiniLocations:
            trySelectFromArray(document_["MiniLocations"], SelectionKind::MiniLocation, worldPosition, [&](const auto& value) {
                return readRect(value.value("Bounds", nlohmann::json::array()));
            });
            break;
        default:
            break;
        }
    }

    void placePlatform(const sf::Vector2f worldPosition)
    {
        if (platformTypes_.empty())
        {
            return;
        }

        const std::string typeName = platformTypes_.at(std::clamp(selectedPlatformTypeIndex_, 0, static_cast<int>(platformTypes_.size()) - 1));
        document_["Platforms"].push_back({
            {"Type", typeName},
            {"Position", toJson(worldPosition)}
        });
        selection_ = {SelectionKind::Platform, document_["Platforms"].size() - 1u};
        markDirty();
    }

    void placeDecoration(const sf::Vector2f worldPosition)
    {
        if (decorationOptions_.empty())
        {
            return;
        }

        const std::string decorationName = decorationOptions_.at(std::clamp(selectedDecorationIndex_, 0, static_cast<int>(decorationOptions_.size()) - 1));
        const sf::Vector2f parallax{1.f, 1.f};
        document_["Decorations"].push_back({
            {"Name", decorationName},
            {"Position", toJson(removeParallaxPreview(worldPosition, parallax))},
            {"Scale", {1.f, 1.f}},
            {"Color", {255, 255, 255, 255}},
            {"ParallaxFactor", toJson(parallax)},
            {"Z", 0}
        });
        selection_ = {SelectionKind::Decoration, document_["Decorations"].size() - 1u};
        markDirty();
    }

    void placeBackground(const sf::Vector2f worldPosition)
    {
        if (backgroundOptions_.empty())
        {
            return;
        }

        const std::string backgroundName = backgroundOptions_.at(std::clamp(selectedBackgroundIndex_, 0, static_cast<int>(backgroundOptions_.size()) - 1));
        const sf::Vector2f parallax{0.08f, 0.06f};
        document_["Background"].push_back({
            {"BgName", backgroundName},
            {"Position", toJson(removeParallaxPreview(worldPosition, parallax))},
            {"ParallaxFactor", toJson(parallax)},
            {"Type", 0}
        });
        selection_ = {SelectionKind::Background, document_["Background"].size() - 1u};
        markDirty();
    }

    void placeSpawner(const sf::Vector2f worldPosition)
    {
        if (enemyTypeOptions_.empty())
        {
            return;
        }

        const std::string enemyName = enemyTypeOptions_.at(std::clamp(selectedEnemyTypeIndex_, 0, static_cast<int>(enemyTypeOptions_.size()) - 1));
        document_["Spawners"].push_back({
            {"EnemyName", enemyName},
            {"EnemyAmount", 3},
            {"SpawnCooldown", 5000},
            {"EnemyPerSpawn", 1},
            {"SpawnArea", {
                {worldPosition.x - 90.f, worldPosition.x + 90.f},
                {worldPosition.y - 80.f, worldPosition.y + 20.f}
            }}
        });
        selection_ = {SelectionKind::Spawner, document_["Spawners"].size() - 1u};
        markDirty();
    }

    void placeInteractive(const sf::Vector2f worldPosition)
    {
        const std::string interactiveType = interactiveTypeOptions_.empty()
            ? "EchoTablet"
            : interactiveTypeOptions_.at(std::clamp(selectedInteractiveTypeIndex_, 0, static_cast<int>(interactiveTypeOptions_.size()) - 1));
        const bool isCustomSign = interactiveType == "CustomSign";
        const std::string textureName = isCustomSign
            ? (nameplateTextureOptions_.empty()
                ? std::string{}
                : nameplateTextureOptions_.at(std::clamp(selectedNameplateTextureIndex_, 0, static_cast<int>(nameplateTextureOptions_.size()) - 1)))
            : (previewTextureOptions_.empty()
                ? std::string{}
                : previewTextureOptions_.at(std::clamp(selectedPreviewTextureIndex_, 0, static_cast<int>(previewTextureOptions_.size()) - 1)));

        document_["Interactives"].push_back({
            {"Type", interactiveType},
            {"Texture", textureName},
            {"Position", toJson(worldPosition)},
            {"Scale", isCustomSign ? toJson(kWorldNameplateScale) : nlohmann::json::array({1.f, 1.f})},
            {"Color", {255, 255, 255, 255}},
            {"AccentColor", {220, 184, 122, 255}},
            {"AutoNameplate", !isCustomSign},
            {"InteractRadius", 120.f},
            {"RewardGold", 0},
            {"SingleUse", true},
            {"GrantsCheckpoint", interactiveType == "RestShrine"},
            {"RestoreVitality", interactiveType == "RestShrine"},
            {"Prompt", isCustomSign ? "Enter to read sign" : "Enter to interact"},
            {"Title", isCustomSign ? "New Sign" : "Forgotten Relic"},
            {"Body", isCustomSign ? "Write your own description here." : "The dead left a trace here."}
        });
        selection_ = {SelectionKind::Interactive, document_["Interactives"].size() - 1u};
        markDirty();
    }

    void placeMiniLocation(const sf::Vector2f worldPosition)
    {
        auto& presets = document_["Presets"];
        const float currentLevelWidth = presets["Size"][0].get<float>();
        const float mainWorldWidth = static_cast<float>(presets.value("MainWorldWidth", static_cast<int>(currentLevelWidth)));
        const float roomX = std::max(currentLevelWidth + 260.f, mainWorldWidth + 640.f);
        const float roomY = 420.f;
        const float roomWidth = 920.f;
        const float roomHeight = 340.f;
        const int locationIndex = static_cast<int>(document_["MiniLocations"].size()) + 1;

        document_["MiniLocations"].push_back({
            {"Id", "mini_location_" + std::to_string(locationIndex)},
            {"Title", "Mini Location " + std::to_string(locationIndex)},
            {"Bounds", {roomX, roomY, roomWidth, roomHeight}},
            {"BarrierEnabled", true},
            {"BarrierWidth", 22.f},
            {"AccentColor", {130, 214, 184, 255}},
            {"Entry", {
                {"Texture", "MossyDecorationHazard_25.png"},
                {"Position", {worldPosition.x, worldPosition.y}},
                {"Scale", {0.22f, 0.33f}},
                {"DestinationSupport", {roomX + 180.f, roomY + roomHeight - 18.f}},
                {"Color", {214, 246, 232, 255}},
                {"AccentColor", {130, 214, 184, 255}},
                {"InteractRadius", 126.f},
                {"Prompt", "Enter the hidden route"}
            }},
            {"Exit", {
                {"Texture", "MossyDecorationHazard_24.png"},
                {"Position", {roomX + 110.f, roomY + roomHeight - 18.f}},
                {"Scale", {0.22f, 0.33f}},
                {"DestinationSupport", {worldPosition.x, worldPosition.y - 18.f}},
                {"Color", {212, 232, 255, 255}},
                {"AccentColor", {130, 214, 184, 255}},
                {"InteractRadius", 126.f},
                {"Prompt", "Enter to return"}
            }},
            {"Hazard", {
                {"Enabled", false},
                {"Rect", {roomX, roomY + roomHeight + 64.f, roomWidth, 64.f}},
                {"CoreColor", {242, 104, 56, 255}},
                {"GlowColor", {255, 182, 96, 255}},
                {"EmberColor", {255, 236, 188, 255}}
            }}
        });

        presets["MainWorldWidth"] = static_cast<int>(std::max(mainWorldWidth, currentLevelWidth));
        presets["Size"][0] = static_cast<int>(std::ceil(roomX + roomWidth + 220.f));
        presets["GenerateMiniLocations"] = false;

        selection_ = {SelectionKind::MiniLocation, document_["MiniLocations"].size() - 1u};
        markDirty();
        focusMiniLocation(selection_.index);
    }

    void deleteSelection()
    {
        if (!selection_.isValid())
        {
            return;
        }

        auto eraseFromArray = [&](const char* arrayName) {
            auto& array = document_[arrayName];
            if (selection_.index < array.size())
            {
                array.erase(array.begin() + static_cast<nlohmann::json::difference_type>(selection_.index));
                spawnInteraction_.clear();
                interactiveInteraction_.clear();
                spawnerInteraction_.clear();
                platformInteraction_.clear();
                decorationInteraction_.clear();
                miniLocationInteraction_.clear();
                groundInteraction_.clear();
                pendingSelectionCycle_.clear();
                openWorldContextMenu_ = false;
                selection_.clear();
                markDirty();
            }
        };

        switch (selection_.kind)
        {
        case SelectionKind::Platform:
            eraseFromArray("Platforms");
            break;
        case SelectionKind::Decoration:
            eraseFromArray("Decorations");
            break;
        case SelectionKind::Background:
            eraseFromArray("Background");
            break;
        case SelectionKind::Ground:
            eraseFromArray("Ground");
            break;
        case SelectionKind::Spawner:
            eraseFromArray("Spawners");
            break;
        case SelectionKind::Interactive:
            eraseFromArray("Interactives");
            break;
        case SelectionKind::MiniLocation:
            eraseFromArray("MiniLocations");
            break;
        default:
            break;
        }
    }

    void drawGrid()
    {
        const sf::Vector2f viewCenter = worldView_.getCenter();
        const sf::Vector2f viewSize = worldView_.getSize();
        const float left = viewCenter.x - viewSize.x * 0.5f;
        const float top = viewCenter.y - viewSize.y * 0.5f;
        const float right = left + viewSize.x;
        const float bottom = top + viewSize.y;

        sf::VertexArray lines(sf::PrimitiveType::Lines);
        const float startX = std::floor(left / kGridSpacing) * kGridSpacing;
        const float startY = std::floor(top / kGridSpacing) * kGridSpacing;

        for (float x = startX; x <= right + kGridSpacing; x += kGridSpacing)
        {
            lines.append(sf::Vertex{{x, top - kGridSpacing}, kGridColor});
            lines.append(sf::Vertex{{x, bottom + kGridSpacing}, kGridColor});
        }

        for (float y = startY; y <= bottom + kGridSpacing; y += kGridSpacing)
        {
            lines.append(sf::Vertex{{left - kGridSpacing, y}, kGridColor});
            lines.append(sf::Vertex{{right + kGridSpacing, y}, kGridColor});
        }

        window_.draw(lines);
    }

    void drawBackgroundPreview(const nlohmann::json& background, const bool selected)
    {
        const std::string textureName = background.value("BgName", std::string{});
        const sf::Texture* texture = findPreviewTexture(textureName);
        if (texture == nullptr)
        {
            return;
        }

        sf::Sprite sprite(*texture);
        sprite.setOrigin(sprite.getGlobalBounds().getCenter());
        sprite.setScale({
            static_cast<float>(WINDOW_WIDTH) / static_cast<float>(texture->getSize().x),
            static_cast<float>(WINDOW_HEIGHT) / static_cast<float>(texture->getSize().y)
        });
        sprite.setPosition(backgroundDisplayPosition(background));
        sprite.setColor(selected ? sf::Color(255, 255, 255, 220) : sf::Color(255, 255, 255, 180));

        const int backgroundType = background.value("Type", 0);
        if (backgroundType == 0)
        {
            const float worldWidth = document_["Presets"]["Size"][0].get<float>();
            const float repeatedWidth = sprite.getGlobalBounds().size.x;
            const int repeatCount = std::max(1, static_cast<int>(std::ceil(worldWidth / repeatedWidth)) + 2);
            for (int index = -1; index < repeatCount; ++index)
            {
                sf::Sprite repeatedSprite(sprite);
                repeatedSprite.setPosition({
                    sprite.getPosition().x + repeatedWidth * static_cast<float>(index),
                    sprite.getPosition().y
                });
                window_.draw(repeatedSprite);
            }
        }
        else
        {
            window_.draw(sprite);
        }
    }

    void drawWorld()
    {
        window_.setView(worldView_);
        drawGrid();

        for (std::size_t index = 0; index < document_["Background"].size(); ++index)
        {
            drawBackgroundPreview(document_["Background"][index], selection_.kind == SelectionKind::Background && selection_.index == index);
        }

        drawGroundPreview();
        drawMiniLocationPreview();
        drawDecorationPreview();
        drawPlatformPreview();
        drawSpawnerPreview();
        drawInteractivePreview();
        drawSpawnPreview();
    }

    void drawGroundPreview()
    {
        for (std::size_t index = 0; index < document_["Ground"].size(); ++index)
        {
            const auto& ground = document_["Ground"][index];
            const sf::FloatRect bounds = groundBounds(ground);
            const bool selected = selection_.kind == SelectionKind::Ground && selection_.index == index;

            sf::RectangleShape groundRect(bounds.size);
            groundRect.setPosition(bounds.position);
            groundRect.setFillColor(selection_.kind == SelectionKind::Ground && selection_.index == index
                ? sf::Color(84, 64, 52, 156)
                : sf::Color(58, 44, 38, 124));
            groundRect.setOutlineThickness(selected ? kSelectionOutlineThickness : 1.5f);
            groundRect.setOutlineColor(selected
                ? sf::Color(228, 194, 138, 224)
                : sf::Color(204, 180, 128, 180));
            window_.draw(groundRect);

            if (selected)
            {
                const sf::FloatRect handleBounds = groundResizeHandleBounds(ground);
                sf::RectangleShape handle(handleBounds.size);
                handle.setPosition(handleBounds.position);
                handle.setFillColor(sf::Color(228, 194, 138, 224));
                handle.setOutlineThickness(1.f);
                handle.setOutlineColor(sf::Color(32, 24, 16, 220));
                window_.draw(handle);
            }
        }
    }

    void drawPlatformPreview()
    {
        for (std::size_t index = 0; index < document_["Platforms"].size(); ++index)
        {
            const auto& platform = document_["Platforms"][index];
            const sf::FloatRect bounds = platformBounds(platform);
            const sf::FloatRect spriteBounds = platformSpriteBounds(platform);
            const std::string typeName = platform.value("Type", std::string{});
            const auto definition = platformDefinition(platform);
            const bool selected = selection_.kind == SelectionKind::Platform && selection_.index == index;
            const bool editHitbox = platformEditHitboxEnabled(platform);

            if (definition.has_value() && !definition->texturePath.empty())
            {
                const sf::Texture* texture = findPreviewTexture(typeName);
                if (texture != nullptr)
                {
                    sf::Sprite sprite(*texture);
                    sprite.setOrigin(sprite.getGlobalBounds().getCenter());
                    sprite.setScale(platformScale(platform));
                    sprite.setColor(definition->tint);
                    sprite.setPosition(platformSpriteCenter(platform) + definition->spriteOffset);
                    window_.draw(sprite);
                }
            }

            if (selected && spriteBounds.size.x > 0.f && spriteBounds.size.y > 0.f)
            {
                sf::RectangleShape spriteOutline(spriteBounds.size);
                spriteOutline.setPosition(spriteBounds.position);
                spriteOutline.setFillColor(kPlatformSpriteFillColor);
                spriteOutline.setOutlineThickness(1.5f);
                spriteOutline.setOutlineColor(kPlatformSpriteOutlineColor);
                window_.draw(spriteOutline);

                const sf::FloatRect scaleHandleBounds = platformScaleHandleBounds(platform);
                sf::RectangleShape scaleHandle(scaleHandleBounds.size);
                scaleHandle.setPosition(scaleHandleBounds.position);
                scaleHandle.setFillColor(kPlatformSpriteOutlineColor);
                scaleHandle.setOutlineThickness(1.f);
                scaleHandle.setOutlineColor(sf::Color(18, 24, 32, 220));
                window_.draw(scaleHandle);
            }

            sf::RectangleShape outline(bounds.size);
            outline.setPosition(bounds.position);
            outline.setFillColor(selected ? kPlatformFillColor : sf::Color::Transparent);
            outline.setOutlineThickness(selected && editHitbox ? kSelectionOutlineThickness : 1.f);
            outline.setOutlineColor(selected
                ? (editHitbox ? kPlatformOutlineColor : sf::Color(kPlatformOutlineColor.r, kPlatformOutlineColor.g, kPlatformOutlineColor.b, 144))
                : sf::Color(255, 255, 255, 66));
            window_.draw(outline);

            if (selected && editHitbox)
            {
                const sf::FloatRect hitboxHandleBounds = platformHitboxHandleBounds(platform);
                sf::RectangleShape hitboxHandle(hitboxHandleBounds.size);
                hitboxHandle.setPosition(hitboxHandleBounds.position);
                hitboxHandle.setFillColor(kPlatformOutlineColor);
                hitboxHandle.setOutlineThickness(1.f);
                hitboxHandle.setOutlineColor(sf::Color(34, 24, 16, 220));
                window_.draw(hitboxHandle);
            }
        }
    }

    void drawDecorationPreview()
    {
        for (std::size_t index = 0; index < document_["Decorations"].size(); ++index)
        {
            const auto& decoration = document_["Decorations"][index];
            const std::string textureName = decoration.value("Name", std::string{});
            const sf::Texture* texture = findPreviewTexture(textureName);
            const sf::Vector2f position = decorationDisplayPosition(decoration);
            const sf::Color color = readColor(decoration.value("Color", nlohmann::json::array()), sf::Color::White);

            if (texture != nullptr)
            {
                sf::Sprite sprite(*texture);
                sprite.setOrigin(sprite.getGlobalBounds().getCenter());
                sprite.setPosition(position);
                sprite.setScale(decorationScale(decoration));
                sprite.setColor(color);
                window_.draw(sprite);
            }

            const sf::FloatRect bounds = decorationBounds(decoration);
            const bool selected = selection_.kind == SelectionKind::Decoration && selection_.index == index;
            sf::RectangleShape outline(bounds.size);
            outline.setPosition(bounds.position);
            outline.setFillColor(selected ? kDecorationFillColor : sf::Color::Transparent);
            outline.setOutlineThickness(selected ? kSelectionOutlineThickness : 1.f);
            outline.setOutlineColor(selected
                ? kDecorationOutlineColor
                : sf::Color(255, 255, 255, 42));
            window_.draw(outline);

            if (selected)
            {
                const sf::FloatRect handleBounds = decorationScaleHandleBounds(decoration);
                sf::RectangleShape handle(handleBounds.size);
                handle.setPosition(handleBounds.position);
                handle.setFillColor(kDecorationOutlineColor);
                handle.setOutlineThickness(1.f);
                handle.setOutlineColor(sf::Color(18, 24, 32, 220));
                window_.draw(handle);
            }
        }
    }

    void drawSpawnerPreview()
    {
        for (std::size_t index = 0; index < document_["Spawners"].size(); ++index)
        {
            const auto& spawner = document_["Spawners"][index];
            const sf::FloatRect bounds = spawnerBounds(spawner);
            const bool selected = selection_.kind == SelectionKind::Spawner && selection_.index == index;

            sf::RectangleShape rect(bounds.size);
            rect.setPosition(bounds.position);
            rect.setFillColor(sf::Color(244, 110, 92, 36));
            rect.setOutlineThickness(selected ? kSelectionOutlineThickness : 1.f);
            rect.setOutlineColor(selected
                ? kSpawnerOutlineColor
                : sf::Color(244, 110, 92, 120));
            window_.draw(rect);

            if (selected)
            {
                const sf::FloatRect handleBounds = spawnerResizeHandleBounds(spawner);
                sf::RectangleShape handle(handleBounds.size);
                handle.setPosition(handleBounds.position);
                handle.setFillColor(kSpawnerOutlineColor);
                handle.setOutlineThickness(1.f);
                handle.setOutlineColor(sf::Color(24, 18, 18, 220));
                window_.draw(handle);
            }

            sf::Text label(font_);
            label.setCharacterSize(18u);
            label.setFillColor(sf::Color(255, 228, 210, 230));
            label.setString(spawner.value("EnemyName", std::string{"Spawner"}));
            label.setPosition({bounds.position.x + 6.f, bounds.position.y - 22.f});
            window_.draw(label);
        }
    }

    void drawInteractivePreview()
    {
        for (std::size_t index = 0; index < document_["Interactives"].size(); ++index)
        {
            const auto& interactive = document_["Interactives"][index];
            const sf::FloatRect bounds = interactiveBounds(interactive);
            const sf::Texture* texture = findPreviewTexture(interactivePreviewTextureName(interactive));
            const sf::Vector2f position = readVector2f(interactive.value("Position", nlohmann::json::array()));
            const sf::Vector2f scale = interactivePreviewScale(interactive);
            const sf::Color color = interactiveUsesAutoNameplate(interactive)
                ? sf::Color::White
                : readColor(interactive.value("Color", nlohmann::json::array()), sf::Color::White);
            const bool selected = selection_.kind == SelectionKind::Interactive && selection_.index == index;

            if (texture != nullptr)
            {
                sf::Sprite sprite(*texture);
                sprite.setOrigin(sprite.getGlobalBounds().getCenter());
                sprite.setPosition(position);
                sprite.setScale(scale);
                sprite.setColor(color);
                window_.draw(sprite);
            }

            sf::RectangleShape outline(bounds.size);
            outline.setPosition(bounds.position);
            outline.setFillColor(selected ? sf::Color(236, 216, 144, 22) : sf::Color::Transparent);
            outline.setOutlineThickness(selected ? kSelectionOutlineThickness : 1.f);
            outline.setOutlineColor(selected
                ? kInteractiveOutlineColor
                : sf::Color(255, 255, 255, 48));
            window_.draw(outline);

            if (selected)
            {
                const sf::FloatRect handleBounds = interactiveScaleHandleBounds(interactive);
                sf::RectangleShape handle(handleBounds.size);
                handle.setPosition(handleBounds.position);
                handle.setFillColor(kInteractiveOutlineColor);
                handle.setOutlineThickness(1.f);
                handle.setOutlineColor(sf::Color(28, 24, 16, 220));
                window_.draw(handle);
            }
        }
    }

    void drawMiniLocationPreview()
    {
        for (std::size_t index = 0; index < document_["MiniLocations"].size(); ++index)
        {
            const auto& location = document_["MiniLocations"][index];
            const sf::FloatRect bounds = miniLocationBounds(location);
            const sf::Color accentColor = location.contains("AccentColor")
                ? readColor(location["AccentColor"], kMiniLocationOutlineColor)
                : kMiniLocationOutlineColor;
            const bool selected = selection_.kind == SelectionKind::MiniLocation && selection_.index == index;

            sf::RectangleShape room(bounds.size);
            room.setPosition(bounds.position);
            room.setFillColor(sf::Color(accentColor.r, accentColor.g, accentColor.b, 22));
            room.setOutlineThickness(selected ? kSelectionOutlineThickness : 1.f);
            room.setOutlineColor(selected
                ? accentColor
                : sf::Color(accentColor.r, accentColor.g, accentColor.b, 148));
            window_.draw(room);

            if (selected)
            {
                const sf::FloatRect handleBounds = miniLocationResizeHandleBounds(location);
                sf::RectangleShape handle(handleBounds.size);
                handle.setPosition(handleBounds.position);
                handle.setFillColor(accentColor);
                handle.setOutlineThickness(1.f);
                handle.setOutlineColor(sf::Color(20, 26, 28, 220));
                window_.draw(handle);
            }

            const sf::Vector2f entryPos = readVector2f(location["Entry"].value("Position", nlohmann::json::array()));
            const sf::Vector2f exitPos = readVector2f(location["Exit"].value("Position", nlohmann::json::array()));
            const sf::Vector2f entryDest = readVector2f(location["Entry"].value("DestinationSupport", nlohmann::json::array()));
            const sf::Vector2f exitDest = readVector2f(location["Exit"].value("DestinationSupport", nlohmann::json::array()));

            drawPortalMarker(entryPos, accentColor, location["Entry"].value("Texture", std::string{}));
            drawPortalMarker(exitPos, sf::Color(212, 232, 255, 255), location["Exit"].value("Texture", std::string{}));
            drawConnectionLine(entryPos, entryDest, accentColor);
            drawConnectionLine(exitPos, exitDest, sf::Color(212, 232, 255, 255));

            sf::Text label(font_);
            label.setCharacterSize(18u);
            label.setFillColor(sf::Color(236, 255, 248, 232));
            label.setString(location.value("Title", std::string{"Mini Location"}));
            label.setPosition({bounds.position.x + 8.f, bounds.position.y + 8.f});
            window_.draw(label);
        }
    }

    void drawPortalMarker(const sf::Vector2f position, const sf::Color color, const std::string& textureName)
    {
        const sf::Texture* texture = findPreviewTexture(textureName);
        if (texture != nullptr)
        {
            sf::Sprite sprite(*texture);
            sprite.setOrigin(sprite.getGlobalBounds().getCenter());
            sprite.setPosition(position);
            sprite.setScale({0.28f, 0.28f});
            sprite.setColor(color);
            window_.draw(sprite);
            return;
        }

        sf::CircleShape circle(14.f);
        circle.setOrigin({14.f, 14.f});
        circle.setPosition(position);
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(2.f);
        circle.setOutlineColor(color);
        window_.draw(circle);
    }

    void drawConnectionLine(const sf::Vector2f start, const sf::Vector2f end, const sf::Color color)
    {
        sf::VertexArray line(sf::PrimitiveType::Lines, 2u);
        line[0].position = start;
        line[0].color = color;
        line[1].position = end;
        line[1].color = sf::Color(color.r, color.g, color.b, 110);
        window_.draw(line);
    }

    void drawSpawnPreview()
    {
        const sf::Vector2f spawn = readVector2f(document_["Presets"].value("PlayerSpawn", nlohmann::json::array()), {200.f, 900.f});

        sf::CircleShape marker(16.f);
        marker.setOrigin({16.f, 16.f});
        marker.setPosition(spawn);
        marker.setFillColor(sf::Color(255, 255, 255, 22));
        marker.setOutlineThickness(selection_.kind == SelectionKind::Spawn ? kSelectionOutlineThickness : 1.5f);
        marker.setOutlineColor(kSpawnOutlineColor);
        window_.draw(marker);

        sf::VertexArray cross(sf::PrimitiveType::Lines, 4u);
        cross[0].position = {spawn.x - 16.f, spawn.y};
        cross[1].position = {spawn.x + 16.f, spawn.y};
        cross[2].position = {spawn.x, spawn.y - 20.f};
        cross[3].position = {spawn.x, spawn.y + 20.f};
        for (std::size_t index = 0; index < cross.getVertexCount(); ++index)
        {
            cross[index].color = kSpawnOutlineColor;
        }
        window_.draw(cross);
    }

    void drawUi()
    {
        drawToolbarWindow();
        drawLevelsWindow();
        drawInspectorWindow();
        drawWorldContextMenu();
    }

    void drawWorldContextMenu()
    {
        const std::vector<std::string> atmosphereOptions(kPlatformAtmosphereModeOptions.begin(), kPlatformAtmosphereModeOptions.end());

        if (openWorldContextMenu_)
        {
            ImGui::OpenPopup("world_context_menu");
            openWorldContextMenu_ = false;
        }

        ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);
        if (!ImGui::BeginPopup("world_context_menu"))
        {
            return;
        }

        bool changed = false;

        if (selection_.kind == SelectionKind::Spawn)
        {
            ImGui::TextUnformatted("Player Spawn");
            ImGui::Separator();
            changed |= editVector2Field("Position##spawn_context", document_["Presets"], "PlayerSpawn", {200.f, 900.f});
            if (ImGui::MenuItem("Focus Inspector Level Tab"))
            {
                activeTab_ = EditorTab::Level;
                ImGui::CloseCurrentPopup();
            }
        }
        else if (selection_.kind == SelectionKind::Platform)
        {
            if (auto* platform = selectedPlatform(); platform != nullptr)
            {
                ImGui::TextUnformatted("Platform");
                ImGui::Separator();

                std::string typeName = platform->value("Type", platformTypes_.empty() ? std::string{} : platformTypes_.front());
                if (comboFromStrings("Type##platform_context", platformTypes_, typeName))
                {
                    (*platform)["Type"] = typeName;
                    changed = true;
                }

                changed |= editVector2Field("Position##platform_context", *platform, "Position");
                changed |= editVector2Field("Scale##platform_context", *platform, "Scale", platformScale(*platform));

                bool editHitbox = platformEditHitboxEnabled(*platform);
                if (ImGui::Checkbox("Edit hitbox##platform_context", &editHitbox))
                {
                    (*platform)["EditHitbox"] = editHitbox;
                    changed = true;
                }

                if (editHitbox)
                {
                    changed |= editVector2Field("Hitbox Offset##platform_context", *platform, "HitboxOffset", platformHitboxOffset(*platform));
                    changed |= editVector2Field("Hitbox##platform_context", *platform, "HitboxSize", platformHitboxSize(*platform));
                }

                std::string atmosphereMode = platformAtmosphereMode(*platform);
                if (comboFromStrings("FX##platform_context", atmosphereOptions, atmosphereMode))
                {
                    (*platform)["AtmosphereStyle"] = atmosphereMode;
                    changed = true;
                }

                if (atmosphereMode != "Default")
                {
                    changed |= editColorField("FX Color##platform_context", *platform, "AtmosphereColor", platformAtmosphereColor(*platform));
                    float density = platformAtmosphereDensity(*platform);
                    if (ImGui::InputFloat("FX Density##platform_context", &density))
                    {
                        (*platform)["AtmosphereDensity"] = std::max(density, 0.f);
                        changed = true;
                    }
                }

                if (changed)
                {
                    (*platform)["HitboxSize"] = toJson(platformHitboxSize(*platform));
                    (*platform)["Scale"] = toJson(platformScale(*platform));
                }

                drawSelectedDrawOrderControls("platform_context");
            }
        }
        else if (selection_.kind == SelectionKind::Decoration)
        {
            if (auto* decoration = selectedDecoration(); decoration != nullptr)
            {
                ImGui::TextUnformatted("Decoration");
                ImGui::Separator();

                std::string decorationName = decoration->value("Name", decorationOptions_.empty() ? std::string{} : decorationOptions_.front());
                if (comboFromStrings("Name##decoration_context", decorationOptions_, decorationName))
                {
                    (*decoration)["Name"] = decorationName;
                    changed = true;
                }

                changed |= editDisplayedParallaxPositionField(
                    "Scene Position##decoration_context",
                    *decoration,
                    decorationDisplayPosition(*decoration),
                    decorationParallax(*decoration)
                );
                changed |= editVector2Field("Scale##decoration_context", *decoration, "Scale", decorationScale(*decoration));
                changed |= drawParallaxControls(
                    "Parallax##decoration_context",
                    "decoration_context",
                    *decoration,
                    decorationDisplayPosition(*decoration),
                    {1.f, 1.f}
                );

                if (changed)
                {
                    (*decoration)["Scale"] = toJson(decorationScale(*decoration));
                }

                drawSelectedDrawOrderControls("decoration_context");
            }
        }
        else if (selection_.kind == SelectionKind::Background)
        {
            if (auto* background = selectedObject(); background != nullptr)
            {
                ImGui::TextUnformatted("Background");
                ImGui::Separator();

                std::string bgName = background->value("BgName", backgroundOptions_.empty() ? std::string{} : backgroundOptions_.front());
                if (comboFromStrings("Texture##background_context", backgroundOptions_, bgName))
                {
                    (*background)["BgName"] = bgName;
                    changed = true;
                }

                changed |= editDisplayedParallaxPositionField(
                    "Scene Position##background_context",
                    *background,
                    backgroundDisplayPosition(*background),
                    backgroundParallax(*background)
                );
                changed |= drawParallaxControls(
                    "Parallax##background_context",
                    "background_context",
                    *background,
                    backgroundDisplayPosition(*background),
                    {0.08f, 0.06f}
                );
                changed |= editStringField("Theme Override##background_context", *background, "Theme", 128u);

                int backgroundType = background->value("Type", 0);
                if (ImGui::Combo("Type##background_context", &backgroundType, "Repeated\0Single\0"))
                {
                    (*background)["Type"] = backgroundType;
                    changed = true;
                }

                drawSelectedDrawOrderControls("background_context");
            }
        }
        else if (selection_.kind == SelectionKind::Interactive)
        {
            if (auto* interactive = selectedInteractive(); interactive != nullptr)
            {
                ImGui::TextUnformatted("Interactive / Sign");
                ImGui::Separator();

                std::string typeName = interactive->value("Type", interactiveTypeOptions_.empty() ? std::string{} : interactiveTypeOptions_.front());
                if (comboFromStrings("Type##interactive_context", interactiveTypeOptions_, typeName))
                {
                    (*interactive)["Type"] = typeName;
                    if (typeName == "CustomSign")
                    {
                        (*interactive)["AutoNameplate"] = false;
                        if (interactive->value("Texture", std::string{}).empty() && !nameplateTextureOptions_.empty())
                        {
                            (*interactive)["Texture"] = nameplateTextureOptions_.front();
                        }
                        (*interactive)["Scale"] = toJson(kWorldNameplateScale);
                    }
                    changed = true;
                }

                bool autoNameplate = interactiveUsesAutoNameplate(*interactive);
                if (ImGui::Checkbox("Auto Nameplate##interactive_context", &autoNameplate))
                {
                    (*interactive)["AutoNameplate"] = autoNameplate;
                    changed = true;
                }

                changed |= editVector2Field("Position##interactive_context", *interactive, "Position");

                if (!autoNameplate)
                {
                    const std::vector<std::string>& textureOptions = (typeName == "CustomSign" || interactive->value("Texture", std::string{}).rfind("nameplate_", 0) == 0) && !nameplateTextureOptions_.empty()
                        ? nameplateTextureOptions_
                        : previewTextureOptions_;
                    std::string textureName = interactive->value("Texture", textureOptions.empty() ? std::string{} : textureOptions.front());
                    if (comboFromStrings("Texture##interactive_context", textureOptions, textureName))
                    {
                        (*interactive)["Texture"] = textureName;
                        changed = true;
                    }
                    changed |= editVector2Field(
                        "Scale##interactive_context",
                        *interactive,
                        "Scale",
                        typeName == "CustomSign" ? kWorldNameplateScale : sf::Vector2f{1.f, 1.f}
                    );
                }

                changed |= editStringField("Prompt##interactive_context", *interactive, "Prompt", 256u);
                changed |= editStringField("Title##interactive_context", *interactive, "Title", 256u);
                changed |= editMultilineStringField("Body##interactive_context", *interactive, "Body", ImVec2(320.f, 96.f), 2048u);

                drawSelectedDrawOrderControls("interactive_context");
            }
        }
        else if (selection_.kind == SelectionKind::Spawner)
        {
            if (auto* spawner = selectedSpawner(); spawner != nullptr)
            {
                ImGui::TextUnformatted("Spawner");
                ImGui::Separator();

                std::string enemyName = spawner->value("EnemyName", enemyTypeOptions_.empty() ? std::string{} : enemyTypeOptions_.front());
                if (comboFromStrings("Enemy##spawner_context", enemyTypeOptions_, enemyName))
                {
                    (*spawner)["EnemyName"] = enemyName;
                    changed = true;
                }

                int enemyAmount = spawner->value("EnemyAmount", 1);
                if (ImGui::InputInt("Enemy Amount##spawner_context", &enemyAmount))
                {
                    (*spawner)["EnemyAmount"] = std::max(enemyAmount, 0);
                    changed = true;
                }

                int spawnCooldown = spawner->value("SpawnCooldown", 5000);
                if (ImGui::InputInt("Cooldown##spawner_context", &spawnCooldown))
                {
                    (*spawner)["SpawnCooldown"] = std::max(spawnCooldown, 0);
                    changed = true;
                }

                int enemyPerSpawn = spawner->value("EnemyPerSpawn", 1);
                if (ImGui::InputInt("Per Spawn##spawner_context", &enemyPerSpawn))
                {
                    (*spawner)["EnemyPerSpawn"] = std::max(enemyPerSpawn, 1);
                    changed = true;
                }

                drawSelectedDrawOrderControls("spawner_context");
            }
        }
        else if (selection_.kind == SelectionKind::Ground)
        {
            if (auto* ground = selectedGround(); ground != nullptr)
            {
                ImGui::TextUnformatted("Ground");
                ImGui::Separator();

                std::string groundTile = ground->value("GroundName", groundTileOptions_.empty() ? std::string{} : groundTileOptions_.front());
                if (comboFromStrings("Ground Tile##ground_context", groundTileOptions_, groundTile))
                {
                    (*ground)["GroundName"] = groundTile;
                    changed = true;
                }

                std::string groundStyle = ground->value("GroundStyle", groundStyleOptions_.empty() ? std::string{} : groundStyleOptions_.front());
                if (comboFromStrings("Style##ground_context", groundStyleOptions_, groundStyle))
                {
                    (*ground)["GroundStyle"] = groundStyle;
                    changed = true;
                }

                int points[2]{(*ground)["Points"][0].get<int>(), (*ground)["Points"][1].get<int>()};
                if (ImGui::InputInt2("Span##ground_context", points))
                {
                    (*ground)["Points"] = nlohmann::json::array({std::min(points[0], points[1]), std::max(points[0], points[1])});
                    changed = true;
                }

                int yPos = ground->value("YPos", 980);
                if (ImGui::InputInt("Y Pos##ground_context", &yPos))
                {
                    (*ground)["YPos"] = yPos;
                    changed = true;
                }

                int depthRows = ground->value("DepthRows", 2);
                if (ImGui::InputInt("Depth Rows##ground_context", &depthRows))
                {
                    (*ground)["DepthRows"] = std::max(depthRows, 0);
                    changed = true;
                }

                float offset = ground->value("Offset", 8.f);
                if (ImGui::InputFloat("Offset##ground_context", &offset))
                {
                    (*ground)["Offset"] = offset;
                    changed = true;
                }

                drawSelectedDrawOrderControls("ground_context");
            }
        }
        else if (selection_.kind == SelectionKind::MiniLocation)
        {
            if (auto* location = selectedObject(); location != nullptr)
            {
                ImGui::TextUnformatted("Mini Location");
                ImGui::Separator();
                ImGui::Text("Title: %s", location->value("Title", std::string{"Mini Location"}).c_str());
                ImGui::Text("Id: %s", location->value("Id", std::string{"mini_location"}).c_str());
                drawSelectedDrawOrderControls("mini_location_context");
            }
        }
        else if (const nlohmann::json* object = selectedObject(); object != nullptr)
        {
            ImGui::TextDisabled("Selected object");

            if (selection_.kind == SelectionKind::Background)
            {
                ImGui::Text("Background: %s", object->value("BgName", std::string{"Background"}).c_str());
            }
            else if (selection_.kind == SelectionKind::Ground)
            {
                ImGui::Text("Ground: %s", object->value("GroundStyle", std::string{"Ground"}).c_str());
            }
            else if (selection_.kind == SelectionKind::Spawner)
            {
                ImGui::Text("Spawner: %s", object->value("EnemyName", std::string{"Spawner"}).c_str());
            }
            else if (selection_.kind == SelectionKind::Interactive)
            {
                ImGui::Text("Interactive: %s", object->value("Title", object->value("Type", std::string{"Interactive"})).c_str());
            }
            else if (selection_.kind == SelectionKind::MiniLocation)
            {
                ImGui::Text("Mini location: %s", object->value("Title", std::string{"Mini Location"}).c_str());
            }
        }

        if (changed)
        {
            markDirty();
        }

        if (selection_.isValid())
        {
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
            {
                deleteSelection();
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    void drawToolbarWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(12.f, 12.f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(640.f, 118.f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Level Editor"))
        {
            ImGui::End();
            return;
        }

        ImGui::TextUnformatted(dirty_ ? "Unsaved changes" : "All changes saved");
        ImGui::SameLine();
        ImGui::TextDisabled("%s", currentFilePath_.empty() ? "unsaved document" : currentFilePath_.filename().string().c_str());

        if (ImGui::Button("Save"))
        {
            saveLevel();
        }
        ImGui::SameLine();
        if (ImGui::Button("Play In Game"))
        {
            if (saveLevel())
            {
                playCurrentLevel();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload Registry"))
        {
            refreshRegistry();
        }
        ImGui::SameLine();
        if (ImGui::Button("Focus Spawn"))
        {
            const sf::Vector2f spawn = readVector2f(document_["Presets"].value("PlayerSpawn", nlohmann::json::array()));
            worldView_.setCenter(spawn);
        }

        ImGui::Separator();
        ImGui::Text("Mode: %s", placementModeLabel().c_str());
        ImGui::Text("Camera: %.0f, %.0f", worldView_.getCenter().x, worldView_.getCenter().y);
        ImGui::TextWrapped("Controls: arrow keys move the camera, middle drag also pans, wheel zooms, left click selects or places, drag spawn/interactives/spawners/decorations/ground/mini-locations directly in the scene, right click any object for quick settings, and enable Edit hitbox on a platform if you want to move or resize its hitbox separately.");
        ImGui::End();
    }

    void drawLevelsWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(12.f, 142.f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(340.f, 680.f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Levels"))
        {
            ImGui::End();
            return;
        }

        if (ImGui::Button("New Level"))
        {
            createDefaultLevel();
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh"))
        {
            refreshRegistry();
        }

        ImGui::SeparatorText("Create");
        if (fileNameBuffer_[0] == '\0')
        {
            copyStringToBuffer("new_level.json", fileNameBuffer_, sizeof(fileNameBuffer_));
        }
        if (levelIdBuffer_[0] == '\0')
        {
            copyStringToBuffer("new_level.json", levelIdBuffer_, sizeof(levelIdBuffer_));
        }
        if (levelTitleBuffer_[0] == '\0')
        {
            copyStringToBuffer("New Level", levelTitleBuffer_, sizeof(levelTitleBuffer_));
        }

        ImGui::InputText("File", fileNameBuffer_, sizeof(fileNameBuffer_));
        ImGui::InputText("Level ID", levelIdBuffer_, sizeof(levelIdBuffer_));
        ImGui::InputText("Title", levelTitleBuffer_, sizeof(levelTitleBuffer_));
        if (ImGui::Button("Start Blank Level"))
        {
            createDefaultLevel();
            document_["Presets"]["LevelId"] = std::string(levelIdBuffer_);
            document_["Presets"]["Title"] = std::string(levelTitleBuffer_);
            syncMetadataBuffers();
        }

        ImGui::SeparatorText("Existing");
        if (ImGui::BeginChild("level_registry_list", ImVec2(0.f, 0.f), true))
        {
            for (const LevelDescriptor& descriptor : registry_.getLevels())
            {
                const std::string label = descriptor.title + "##" + descriptor.id;
                if (ImGui::Selectable(label.c_str(), currentFilePath_ == descriptor.filePath))
                {
                    loadLevel(descriptor.filePath);
                }
                ImGui::TextDisabled("%s", descriptor.fileName.c_str());
                ImGui::Separator();
            }
        }
        ImGui::EndChild();
        ImGui::End();
    }

    void drawInspectorWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(1240.f, 12.f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(348.f, 810.f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Inspector"))
        {
            ImGui::End();
            return;
        }

        if (ImGui::BeginTabBar("editor_tabs"))
        {
            if (ImGui::BeginTabItem("Level"))
            {
                activeTab_ = EditorTab::Level;
                drawLevelInspector();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Platforms"))
            {
                activeTab_ = EditorTab::Platforms;
                drawPlatformsInspector();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Decorations"))
            {
                activeTab_ = EditorTab::Decorations;
                drawDecorationsInspector();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Background"))
            {
                activeTab_ = EditorTab::Backgrounds;
                drawBackgroundInspector();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Ground"))
            {
                activeTab_ = EditorTab::Ground;
                drawGroundInspector();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Spawners"))
            {
                activeTab_ = EditorTab::Spawners;
                drawSpawnersInspector();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Interactives"))
            {
                activeTab_ = EditorTab::Interactives;
                drawInteractivesInspector();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Mini Locations"))
            {
                activeTab_ = EditorTab::MiniLocations;
                drawMiniLocationsInspector();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    void drawLevelInspector()
    {
        bool changed = false;
        changed |= ImGui::InputText("File Name", fileNameBuffer_, sizeof(fileNameBuffer_));
        changed |= ImGui::InputText("Level ID", levelIdBuffer_, sizeof(levelIdBuffer_));
        changed |= ImGui::InputText("Title", levelTitleBuffer_, sizeof(levelTitleBuffer_));

        auto& presets = document_["Presets"];
        int size[2]{presets["Size"][0].get<int>(), presets["Size"][1].get<int>()};
        if (ImGui::InputInt2("Level Size", size))
        {
            presets["Size"] = nlohmann::json::array({std::max(size[0], 640), std::max(size[1], 360)});
            changed = true;
        }

        int mainWorldWidth = presets.value("MainWorldWidth", presets["Size"][0].get<int>());
        if (ImGui::InputInt("Main World Width", &mainWorldWidth))
        {
            presets["MainWorldWidth"] = std::max(mainWorldWidth, 640);
            changed = true;
        }

        sf::Vector2f spawn = readVector2f(presets.value("PlayerSpawn", nlohmann::json::array()), {200.f, 900.f});
        float spawnRaw[2]{spawn.x, spawn.y};
        if (ImGui::InputFloat2("Player Spawn", spawnRaw))
        {
            presets["PlayerSpawn"] = nlohmann::json::array({spawnRaw[0], spawnRaw[1]});
            changed = true;
        }

        std::string weatherTheme = presets.value("BackgroundTheme", std::string{"VerdantDawn"});
        if (comboFromStrings("Weather Theme", weatherThemeOptions_, weatherTheme))
        {
            presets["BackgroundTheme"] = weatherTheme;
            changed = true;
        }
        changed |= editStringField("Weather Id", presets, "BackgroundTheme", 128u);

        bool isAvailable = presets.value("isAvaiable", true);
        if (ImGui::Checkbox("Available", &isAvailable))
        {
            presets["isAvaiable"] = isAvailable;
            changed = true;
        }

        bool isConstant = presets.value("isConstant", true);
        if (ImGui::Checkbox("Constant Level", &isConstant))
        {
            presets["isConstant"] = isConstant;
            changed = true;
        }

        bool proceduralMiniLocations = presets.value("GenerateMiniLocations", false);
        if (ImGui::Checkbox("Procedural Mini Locations", &proceduralMiniLocations))
        {
            presets["GenerateMiniLocations"] = proceduralMiniLocations;
            changed = true;
        }

        if (ImGui::Button("Pick Spawn From Scene"))
        {
            placementMode_ = SelectionKind::Spawn;
        }
        ImGui::SameLine();
        if (ImGui::Button("Select Mode"))
        {
            placementMode_ = SelectionKind::None;
        }

        if (changed)
        {
            markDirty();
        }

        ImGui::TextWrapped("You can also drag the spawn marker directly with the mouse in the scene.");
    }

    void drawPlatformsInspector()
    {
        const std::vector<std::string> atmosphereOptions(kPlatformAtmosphereModeOptions.begin(), kPlatformAtmosphereModeOptions.end());

        if (!platformTypes_.empty())
        {
            drawPlacementCombo("Platform Type", platformTypes_, selectedPlatformTypeIndex_);
        }
        drawPlacementButtons(SelectionKind::Platform);
        ImGui::TextWrapped("In Select Mode: drag the platform sprite to move it, drag the blue handle to resize the sprite, and enable Edit hitbox in RMB if you want to move or resize the hitbox separately.");
        drawPlatformList();

        if (selection_.kind == SelectionKind::Platform && selection_.index < document_["Platforms"].size())
        {
            auto& platform = document_["Platforms"][selection_.index];
            bool changed = false;
            std::string typeName = platform.value("Type", platformTypes_.empty() ? std::string{} : platformTypes_.front());
            if (comboFromStrings("Type", platformTypes_, typeName))
            {
                platform["Type"] = typeName;
                changed = true;
            }
            changed |= editVector2Field("Position", platform, "Position");
            changed |= editVector2Field("Scale", platform, "Scale", platformScale(platform));

            bool editHitbox = platformEditHitboxEnabled(platform);
            if (ImGui::Checkbox("Edit Hitbox", &editHitbox))
            {
                platform["EditHitbox"] = editHitbox;
                changed = true;
            }

            if (editHitbox)
            {
                changed |= editVector2Field("Hitbox Offset", platform, "HitboxOffset", platformHitboxOffset(platform));
                changed |= editVector2Field("Hitbox Size", platform, "HitboxSize", platformHitboxSize(platform));
            }
            else
            {
                ImGui::TextDisabled("Enable Edit Hitbox in RMB to move the hitbox separately.");
            }

            std::string atmosphereMode = platformAtmosphereMode(platform);
            if (comboFromStrings("Platform FX", atmosphereOptions, atmosphereMode))
            {
                platform["AtmosphereStyle"] = atmosphereMode;
                changed = true;
            }

            if (atmosphereMode != "Default")
            {
                changed |= editColorField("FX Color", platform, "AtmosphereColor", platformAtmosphereColor(platform));

                float density = platformAtmosphereDensity(platform);
                if (ImGui::InputFloat("FX Density", &density))
                {
                    platform["AtmosphereDensity"] = std::max(density, 0.f);
                    changed = true;
                }
            }

            if (changed)
            {
                platform["HitboxSize"] = toJson(platformHitboxSize(platform));
                platform["Scale"] = toJson(platformScale(platform));
            }

            drawSelectedDrawOrderControls("platform_inspector");

            if (changed)
            {
                markDirty();
            }
        }
    }

    void drawDecorationsInspector()
    {
        if (!decorationOptions_.empty())
        {
            drawPlacementCombo("Decoration", decorationOptions_, selectedDecorationIndex_);
        }
        drawPlacementButtons(SelectionKind::Decoration);
        ImGui::TextWrapped("In Select Mode: drag a decoration to move it, or drag the green corner handle to resize it directly in the scene.");
        drawDecorationList();

        if (selection_.kind == SelectionKind::Decoration && selection_.index < document_["Decorations"].size())
        {
            auto& decoration = document_["Decorations"][selection_.index];
            bool changed = false;
            std::string name = decoration.value("Name", decorationOptions_.empty() ? std::string{} : decorationOptions_.front());
            if (comboFromStrings("Name", decorationOptions_, name))
            {
                decoration["Name"] = name;
                changed = true;
            }
            changed |= editDisplayedParallaxPositionField(
                "Scene Position",
                decoration,
                decorationDisplayPosition(decoration),
                decorationParallax(decoration)
            );
            changed |= editVector2Field("Scale", decoration, "Scale", {1.f, 1.f});
            changed |= drawParallaxControls(
                "Parallax",
                "decoration_inspector",
                decoration,
                decorationDisplayPosition(decoration),
                {1.f, 1.f}
            );
            changed |= editColorField("Color", decoration, "Color", sf::Color::White);

            int zDepth = decoration.value("Z", 0);
            if (ImGui::InputInt("Z", &zDepth))
            {
                decoration["Z"] = zDepth;
                changed = true;
            }

            if (changed)
            {
                markDirty();
            }

            drawSelectedDrawOrderControls("decoration_inspector");
        }
    }

    void drawBackgroundInspector()
    {
        if (!backgroundOptions_.empty())
        {
            drawPlacementCombo("Background", backgroundOptions_, selectedBackgroundIndex_);
        }
        drawPlacementButtons(SelectionKind::Background);
        drawBackgroundList();

        if (selection_.kind == SelectionKind::Background && selection_.index < document_["Background"].size())
        {
            auto& background = document_["Background"][selection_.index];
            bool changed = false;
            std::string bgName = background.value("BgName", backgroundOptions_.empty() ? std::string{} : backgroundOptions_.front());
            if (comboFromStrings("Texture", backgroundOptions_, bgName))
            {
                background["BgName"] = bgName;
                changed = true;
            }
            changed |= editDisplayedParallaxPositionField(
                "Scene Position",
                background,
                backgroundDisplayPosition(background),
                backgroundParallax(background)
            );
            changed |= drawParallaxControls(
                "Parallax",
                "background_inspector",
                background,
                backgroundDisplayPosition(background),
                {0.08f, 0.06f}
            );
            changed |= editStringField("Theme Override", background, "Theme", 128u);

            int backgroundType = background.value("Type", 0);
            if (ImGui::Combo("Type", &backgroundType, "Repeated\0Single\0"))
            {
                background["Type"] = backgroundType;
                changed = true;
            }

            if (changed)
            {
                markDirty();
            }

            drawSelectedDrawOrderControls("background_inspector");
        }
    }

    void drawGroundInspector()
    {
        if (ImGui::Button("Add Ground Strip"))
        {
            document_["Ground"].push_back({
                {"GroundStyle", groundStyleOptions_.empty() ? "VerdantKeep" : groundStyleOptions_.front()},
                {"GroundName", groundTileOptions_.empty() ? "TileSetGreen_02.png" : groundTileOptions_.front()},
                {"Points", {0, document_["Presets"]["MainWorldWidth"].get<int>()}},
                {"YPos", 980},
                {"DepthRows", 2},
                {"Offset", 8.f}
            });
            selection_ = {SelectionKind::Ground, document_["Ground"].size() - 1u};
            markDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Select Mode"))
        {
            placementMode_ = SelectionKind::None;
        }

        ImGui::TextWrapped("In Select Mode: drag ground to move it, or drag the upper-right handle to change its span and height.");
        drawGroundList();

        if (selection_.kind == SelectionKind::Ground && selection_.index < document_["Ground"].size())
        {
            auto& ground = document_["Ground"][selection_.index];
            bool changed = false;
            std::string groundTile = ground.value("GroundName", groundTileOptions_.empty() ? std::string{} : groundTileOptions_.front());
            if (comboFromStrings("Ground Tile", groundTileOptions_, groundTile))
            {
                ground["GroundName"] = groundTile;
                changed = true;
            }

            std::string groundStyle = ground.value("GroundStyle", groundStyleOptions_.empty() ? std::string{} : groundStyleOptions_.front());
            if (comboFromStrings("Style", groundStyleOptions_, groundStyle))
            {
                ground["GroundStyle"] = groundStyle;
                changed = true;
            }

            int points[2]{ground["Points"][0].get<int>(), ground["Points"][1].get<int>()};
            if (ImGui::InputInt2("Span", points))
            {
                ground["Points"] = nlohmann::json::array({std::min(points[0], points[1]), std::max(points[0], points[1])});
                changed = true;
            }

            int yPos = ground.value("YPos", 980);
            if (ImGui::InputInt("Y Pos", &yPos))
            {
                ground["YPos"] = yPos;
                changed = true;
            }

            int depthRows = ground.value("DepthRows", 2);
            if (ImGui::InputInt("Depth Rows", &depthRows))
            {
                ground["DepthRows"] = std::max(depthRows, 0);
                changed = true;
            }

            float offset = ground.value("Offset", 8.f);
            if (ImGui::InputFloat("Offset", &offset))
            {
                ground["Offset"] = offset;
                changed = true;
            }

            if (changed)
            {
                markDirty();
            }

            drawSelectedDrawOrderControls("ground_inspector");
        }
    }

    void drawSpawnersInspector()
    {
        if (!enemyTypeOptions_.empty())
        {
            drawPlacementCombo("Enemy", enemyTypeOptions_, selectedEnemyTypeIndex_);
        }
        drawPlacementButtons(SelectionKind::Spawner);
        ImGui::TextWrapped("In Select Mode: drag a spawner area to move it, or drag its corner handle to resize the spawn zone.");
        drawSpawnerList();

        if (selection_.kind == SelectionKind::Spawner && selection_.index < document_["Spawners"].size())
        {
            auto& spawner = document_["Spawners"][selection_.index];
            bool changed = false;
            std::string enemyName = spawner.value("EnemyName", enemyTypeOptions_.empty() ? std::string{} : enemyTypeOptions_.front());
            if (comboFromStrings("Enemy", enemyTypeOptions_, enemyName))
            {
                spawner["EnemyName"] = enemyName;
                changed = true;
            }

            int enemyAmount = spawner.value("EnemyAmount", 1);
            if (ImGui::InputInt("Enemy Amount", &enemyAmount))
            {
                spawner["EnemyAmount"] = std::max(enemyAmount, 0);
                changed = true;
            }

            int spawnCooldown = spawner.value("SpawnCooldown", 5000);
            if (ImGui::InputInt("Spawn Cooldown", &spawnCooldown))
            {
                spawner["SpawnCooldown"] = std::max(spawnCooldown, 0);
                changed = true;
            }

            int enemyPerSpawn = spawner.value("EnemyPerSpawn", 1);
            if (ImGui::InputInt("Enemy Per Spawn", &enemyPerSpawn))
            {
                spawner["EnemyPerSpawn"] = std::max(enemyPerSpawn, 1);
                changed = true;
            }

            sf::FloatRect bounds = spawnerBounds(spawner);
            float rectRaw[4]{bounds.position.x, bounds.position.y, bounds.size.x, bounds.size.y};
            if (ImGui::InputFloat4("Spawn Area", rectRaw))
            {
                spawner["SpawnArea"] = nlohmann::json::array({
                    nlohmann::json::array({rectRaw[0], rectRaw[0] + rectRaw[2]}),
                    nlohmann::json::array({rectRaw[1], rectRaw[1] + rectRaw[3]})
                });
                changed = true;
            }

            if (changed)
            {
                markDirty();
            }

            drawSelectedDrawOrderControls("spawner_inspector");
        }
    }

    void drawInteractivesInspector()
    {
        if (!interactiveTypeOptions_.empty())
        {
            drawPlacementCombo("Type", interactiveTypeOptions_, selectedInteractiveTypeIndex_);
        }
        if (!nameplateTextureOptions_.empty())
        {
            drawPlacementCombo("Sign Texture", nameplateTextureOptions_, selectedNameplateTextureIndex_);
        }
        if (!previewTextureOptions_.empty())
        {
            drawPlacementCombo("Texture", previewTextureOptions_, selectedPreviewTextureIndex_);
        }
        drawPlacementButtons(SelectionKind::Interactive);
        ImGui::TextWrapped("Use CustomSign for your own tablichka with Title and Body text. Existing signs and interactives can be edited via RMB.");
        drawInteractiveList();

        if (selection_.kind == SelectionKind::Interactive && selection_.index < document_["Interactives"].size())
        {
            auto& interactive = document_["Interactives"][selection_.index];
            bool changed = false;

            std::string typeName = interactive.value("Type", interactiveTypeOptions_.empty() ? std::string{} : interactiveTypeOptions_.front());
            if (comboFromStrings("Interactive Type", interactiveTypeOptions_, typeName))
            {
                interactive["Type"] = typeName;
                if (typeName == "CustomSign")
                {
                    interactive["AutoNameplate"] = false;
                    if (interactive.value("Texture", std::string{}).empty() && !nameplateTextureOptions_.empty())
                    {
                        interactive["Texture"] = nameplateTextureOptions_.front();
                    }
                    interactive["Scale"] = toJson(kWorldNameplateScale);
                }
                changed = true;
            }

            bool autoNameplate = interactiveUsesAutoNameplate(interactive);
            if (ImGui::Checkbox("Auto Nameplate", &autoNameplate))
            {
                interactive["AutoNameplate"] = autoNameplate;
                changed = true;
            }

            changed |= editVector2Field("Position", interactive, "Position");

            if (!autoNameplate)
            {
                const std::vector<std::string>& textureOptions = (typeName == "CustomSign" || interactive.value("Texture", std::string{}).rfind("nameplate_", 0) == 0) && !nameplateTextureOptions_.empty()
                    ? nameplateTextureOptions_
                    : previewTextureOptions_;
                std::string textureName = interactive.value("Texture", textureOptions.empty() ? std::string{} : textureOptions.front());
                if (comboFromStrings("Texture", textureOptions, textureName))
                {
                    interactive["Texture"] = textureName;
                    changed = true;
                }

                const sf::Vector2f fallbackScale = typeName == "CustomSign" ? kWorldNameplateScale : sf::Vector2f{1.f, 1.f};
                changed |= editVector2Field("Scale", interactive, "Scale", fallbackScale);
            }
            else
            {
                ImGui::TextDisabled("Automatic nameplate picks a plaque sprite and uses the shared sign scale.");
            }

            changed |= editColorField("Color", interactive, "Color", sf::Color::White);
            changed |= editColorField("Accent", interactive, "AccentColor", sf::Color(220, 184, 122, 255));

            float interactRadius = interactive.value("InteractRadius", 120.f);
            if (ImGui::InputFloat("Interact Radius", &interactRadius))
            {
                interactive["InteractRadius"] = std::max(interactRadius, 0.f);
                changed = true;
            }

            int rewardGold = interactive.value("RewardGold", 0);
            if (ImGui::InputInt("Reward Gold", &rewardGold))
            {
                interactive["RewardGold"] = std::max(rewardGold, 0);
                changed = true;
            }

            bool singleUse = interactive.value("SingleUse", true);
            if (ImGui::Checkbox("Single Use", &singleUse))
            {
                interactive["SingleUse"] = singleUse;
                changed = true;
            }

            bool grantsCheckpoint = interactive.value("GrantsCheckpoint", typeName == "RestShrine");
            if (ImGui::Checkbox("Checkpoint", &grantsCheckpoint))
            {
                interactive["GrantsCheckpoint"] = grantsCheckpoint;
                changed = true;
            }

            bool restoreVitality = interactive.value("RestoreVitality", typeName == "RestShrine");
            if (ImGui::Checkbox("Restore Vitality", &restoreVitality))
            {
                interactive["RestoreVitality"] = restoreVitality;
                changed = true;
            }

            changed |= editVector2Field("Spawn Offset", interactive, "SpawnOffset", {0.f, 0.f});
            changed |= editStringField("Prompt", interactive, "Prompt", 256u);
            changed |= editStringField("Title", interactive, "Title", 256u);
            changed |= editMultilineStringField("Body", interactive, "Body", ImVec2(-1.f, 84.f), 2048u);

            if (changed)
            {
                markDirty();
            }

            drawSelectedDrawOrderControls("interactive_inspector");
        }
    }

    void drawMiniLocationsInspector()
    {
        drawPlacementButtons(SelectionKind::MiniLocation);
        ImGui::TextWrapped("In Select Mode: drag the room to move the whole mini-location, or drag its corner handle to resize the room.");
        drawMiniLocationList();

        if (selection_.kind == SelectionKind::MiniLocation && selection_.index < document_["MiniLocations"].size())
        {
            auto& location = document_["MiniLocations"][selection_.index];
            if (!location.contains("Entry") || !location["Entry"].is_object())
            {
                location["Entry"] = nlohmann::json::object();
            }
            if (!location.contains("Exit") || !location["Exit"].is_object())
            {
                location["Exit"] = nlohmann::json::object();
            }
            bool changed = false;
            changed |= editStringField("Id", location, "Id", 256u);
            changed |= editStringField("Title", location, "Title", 256u);

            sf::FloatRect bounds = readRect(location.value("Bounds", nlohmann::json::array()), sf::FloatRect({0.f, 0.f}, {920.f, 340.f}));
            float rectRaw[4]{bounds.position.x, bounds.position.y, bounds.size.x, bounds.size.y};
            if (ImGui::InputFloat4("Bounds", rectRaw))
            {
                location["Bounds"] = nlohmann::json::array({rectRaw[0], rectRaw[1], std::max(rectRaw[2], 64.f), std::max(rectRaw[3], 64.f)});
                changed = true;
            }

            bool barrierEnabled = location.value("BarrierEnabled", true);
            if (ImGui::Checkbox("Barriers Enabled", &barrierEnabled))
            {
                location["BarrierEnabled"] = barrierEnabled;
                changed = true;
            }

            float barrierWidth = location.value("BarrierWidth", 22.f);
            if (ImGui::InputFloat("Barrier Width", &barrierWidth))
            {
                location["BarrierWidth"] = std::max(barrierWidth, 0.f);
                changed = true;
            }

            changed |= editColorField("Accent Color", location, "AccentColor", sf::Color(130, 214, 184, 255));

            float deathY = location.value("DeathY", std::numeric_limits<float>::max());
            if (ImGui::InputFloat("Death Y", &deathY))
            {
                location["DeathY"] = deathY;
                changed = true;
            }

            ImGui::SeparatorText("Entry");
            {
                std::string entryTexture = location["Entry"].value("Texture", std::string{});
                if (comboFromStrings("Entry Texture", previewTextureOptions_, entryTexture))
                {
                    location["Entry"]["Texture"] = entryTexture;
                    changed = true;
                }
            }
            changed |= editVector2Field("Entry Position", location["Entry"], "Position");
            changed |= editVector2Field("Entry Scale", location["Entry"], "Scale", {0.22f, 0.33f});
            changed |= editVector2Field("Entry Support", location["Entry"], "DestinationSupport");
            changed |= editColorField("Entry Color", location["Entry"], "Color", sf::Color(214, 246, 232, 255));
            changed |= editColorField("Entry Accent", location["Entry"], "AccentColor", sf::Color(130, 214, 184, 255));
            changed |= editStringField("Entry Prompt", location["Entry"], "Prompt", 256u);

            ImGui::SeparatorText("Exit");
            {
                std::string exitTexture = location["Exit"].value("Texture", std::string{});
                if (comboFromStrings("Exit Texture", previewTextureOptions_, exitTexture))
                {
                    location["Exit"]["Texture"] = exitTexture;
                    changed = true;
                }
            }
            changed |= editVector2Field("Exit Position", location["Exit"], "Position");
            changed |= editVector2Field("Exit Scale", location["Exit"], "Scale", {0.22f, 0.33f});
            changed |= editVector2Field("Exit Support", location["Exit"], "DestinationSupport");
            changed |= editColorField("Exit Color", location["Exit"], "Color", sf::Color(212, 232, 255, 255));
            changed |= editColorField("Exit Accent", location["Exit"], "AccentColor", sf::Color(130, 214, 184, 255));
            changed |= editStringField("Exit Prompt", location["Exit"], "Prompt", 256u);

            ImGui::SeparatorText("Hazard");
            if (!location.contains("Hazard") || !location["Hazard"].is_object())
            {
                location["Hazard"] = nlohmann::json::object();
            }
            bool hazardEnabled = location["Hazard"].value("Enabled", false);
            if (ImGui::Checkbox("Hazard Enabled", &hazardEnabled))
            {
                location["Hazard"]["Enabled"] = hazardEnabled;
                changed = true;
            }

            sf::FloatRect hazardRect = readRect(location["Hazard"].value("Rect", nlohmann::json::array()), sf::FloatRect({bounds.position.x, bounds.position.y + bounds.size.y + 64.f}, {bounds.size.x, 64.f}));
            float hazardRaw[4]{hazardRect.position.x, hazardRect.position.y, hazardRect.size.x, hazardRect.size.y};
            if (ImGui::InputFloat4("Hazard Rect", hazardRaw))
            {
                location["Hazard"]["Rect"] = nlohmann::json::array({hazardRaw[0], hazardRaw[1], std::max(hazardRaw[2], 1.f), std::max(hazardRaw[3], 1.f)});
                changed = true;
            }

            changed |= editColorField("Hazard Core", location["Hazard"], "CoreColor", sf::Color(242, 104, 56, 255));
            changed |= editColorField("Hazard Glow", location["Hazard"], "GlowColor", sf::Color(255, 182, 96, 255));
            changed |= editColorField("Hazard Ember", location["Hazard"], "EmberColor", sf::Color(255, 236, 188, 255));

            if (ImGui::Button("Focus Room"))
            {
                focusMiniLocation(selection_.index);
            }

            if (changed)
            {
                markDirty();
            }

            drawSelectedDrawOrderControls("mini_location_inspector");
        }
    }

    void drawPlacementCombo(const char* label, const std::vector<std::string>& options, int& selectedIndex)
    {
        if (options.empty())
        {
            ImGui::TextDisabled("%s: no assets", label);
            return;
        }

        selectedIndex = std::clamp(selectedIndex, 0, static_cast<int>(options.size()) - 1);
        std::string currentValue = options[selectedIndex];
        if (comboFromStrings(label, options, currentValue))
        {
            const auto currentIt = std::find(options.begin(), options.end(), currentValue);
            selectedIndex = currentIt != options.end() ? static_cast<int>(std::distance(options.begin(), currentIt)) : 0;
        }
    }

    void drawPlacementButtons(const SelectionKind placementKind)
    {
        if (ImGui::Button("Place In Scene"))
        {
            placementMode_ = placementKind;
        }
        ImGui::SameLine();
        if (ImGui::Button("Select Mode"))
        {
            placementMode_ = SelectionKind::None;
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete Selected"))
        {
            deleteSelection();
        }
    }

    void drawPlatformList()
    {
        if (ImGui::BeginChild("platform_list", ImVec2(0.f, 148.f), true))
        {
            for (std::size_t index = 0; index < document_["Platforms"].size(); ++index)
            {
                const auto& platform = document_["Platforms"][index];
                const std::string label = std::to_string(index + 1u) + ". " + platform.value("Type", std::string{"Platform"}) + " @ " + formatPositionLabel(readVector2f(platform.value("Position", nlohmann::json::array())));
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::Platform && selection_.index == index))
                {
                    selection_ = {SelectionKind::Platform, index};
                }
            }
        }
        ImGui::EndChild();
    }

    void drawDecorationList()
    {
        if (ImGui::BeginChild("decoration_list", ImVec2(0.f, 148.f), true))
        {
            for (std::size_t index = 0; index < document_["Decorations"].size(); ++index)
            {
                const auto& decoration = document_["Decorations"][index];
                const std::string label = std::to_string(index + 1u) + ". " + decoration.value("Name", std::string{"Decoration"}) + " @ " + formatPositionLabel(decorationDisplayPosition(decoration));
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::Decoration && selection_.index == index))
                {
                    selection_ = {SelectionKind::Decoration, index};
                }
            }
        }
        ImGui::EndChild();
    }

    void drawBackgroundList()
    {
        if (ImGui::BeginChild("background_list", ImVec2(0.f, 148.f), true))
        {
            for (std::size_t index = 0; index < document_["Background"].size(); ++index)
            {
                const auto& background = document_["Background"][index];
                const std::string label = std::to_string(index + 1u) + ". " + background.value("BgName", std::string{"Background"}) + " @ " + formatPositionLabel(backgroundDisplayPosition(background));
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::Background && selection_.index == index))
                {
                    selection_ = {SelectionKind::Background, index};
                }
            }
        }
        ImGui::EndChild();
    }

    void drawGroundList()
    {
        if (ImGui::BeginChild("ground_list", ImVec2(0.f, 120.f), true))
        {
            for (std::size_t index = 0; index < document_["Ground"].size(); ++index)
            {
                const auto& ground = document_["Ground"][index];
                const std::string label = std::to_string(index + 1u) + ". " + ground.value("GroundStyle", std::string{"Ground"});
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::Ground && selection_.index == index))
                {
                    selection_ = {SelectionKind::Ground, index};
                }
            }
        }
        ImGui::EndChild();
    }

    void drawSpawnerList()
    {
        if (ImGui::BeginChild("spawner_list", ImVec2(0.f, 148.f), true))
        {
            for (std::size_t index = 0; index < document_["Spawners"].size(); ++index)
            {
                const auto& spawner = document_["Spawners"][index];
                const std::string label = std::to_string(index + 1u) + ". " + spawner.value("EnemyName", std::string{"Spawner"});
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::Spawner && selection_.index == index))
                {
                    selection_ = {SelectionKind::Spawner, index};
                }
            }
        }
        ImGui::EndChild();
    }

    void drawInteractiveList()
    {
        if (ImGui::BeginChild("interactive_list", ImVec2(0.f, 148.f), true))
        {
            for (std::size_t index = 0; index < document_["Interactives"].size(); ++index)
            {
                const auto& interactive = document_["Interactives"][index];
                const std::string label = std::to_string(index + 1u) + ". " + interactive.value("Title", interactive.value("Type", std::string{"Interactive"}));
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::Interactive && selection_.index == index))
                {
                    selection_ = {SelectionKind::Interactive, index};
                }
            }
        }
        ImGui::EndChild();
    }

    void drawMiniLocationList()
    {
        if (ImGui::BeginChild("mini_location_list", ImVec2(0.f, 148.f), true))
        {
            for (std::size_t index = 0; index < document_["MiniLocations"].size(); ++index)
            {
                const auto& location = document_["MiniLocations"][index];
                const std::string label = std::to_string(index + 1u) + ". " + location.value("Title", std::string{"Mini Location"});
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::MiniLocation && selection_.index == index))
                {
                    selection_ = {SelectionKind::MiniLocation, index};
                }
            }
        }
        ImGui::EndChild();
    }

    void focusMiniLocation(const std::size_t index)
    {
        if (index >= document_["MiniLocations"].size())
        {
            return;
        }

        const sf::FloatRect bounds = readRect(document_["MiniLocations"][index].value("Bounds", nlohmann::json::array()));
        worldView_.setCenter(bounds.getCenter());
    }

    std::string placementModeLabel() const
    {
        switch (placementMode_)
        {
        case SelectionKind::Spawn:
            return "Pick spawn";
        case SelectionKind::Platform:
            return "Place platforms";
        case SelectionKind::Decoration:
            return "Place decorations";
        case SelectionKind::Background:
            return "Place backgrounds";
        case SelectionKind::Spawner:
            return "Place spawners";
        case SelectionKind::Interactive:
            return "Place interactives";
        case SelectionKind::MiniLocation:
            return "Place mini locations";
        default:
            return "Select";
        }
    }

    void playCurrentLevel()
    {
        const std::string levelId = document_["Presets"].value("LevelId", std::string{});
        if (levelId.empty())
        {
            return;
        }

#ifdef _WIN32
        const std::filesystem::path executablePath = std::filesystem::current_path() / "main.exe";
#else
        const std::filesystem::path executablePath = std::filesystem::current_path() / "main";
#endif
        if (!std::filesystem::exists(executablePath))
        {
            return;
        }

        const std::string command = "\"" + executablePath.string() + "\" --level \"" + levelId + "\"";
        std::thread([command]() {
            std::system(command.c_str());
        }).detach();
    }
};
}

int main()
{
    try
    {
        LevelEditorApp app;
        return app.run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Level editor error: " << exception.what() << std::endl;
        return 1;
    }
}
