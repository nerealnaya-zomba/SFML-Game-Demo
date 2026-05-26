#include <Defines.h>
#include <GameData.h>
#include <Ground.h>
#include <LevelRegistry.h>
#include <Platform.h>
#include <AppIcon.h>
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
#include <chrono>
#include <filesystem>
#include <future>
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
constexpr sf::Vector2f kBackgroundTileSize{
    static_cast<float>(WINDOW_WIDTH),
    static_cast<float>(WINDOW_HEIGHT)
};
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
constexpr sf::Color kPortalOutlineColor(112, 208, 255, 255);
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

std::string lowercaseAscii(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

std::string backgroundRepeatTypeName(const nlohmann::json& background)
{
    if (!background.contains("Type"))
    {
        return "repeatedXY";
    }

    const auto& value = background["Type"];
    if (value.is_number_integer())
    {
        return value.get<int>() == 1 ? "single" : "repeatedXY";
    }
    if (!value.is_string())
    {
        return "repeatedXY";
    }

    const std::string typeName = lowercaseAscii(value.get<std::string>());
    if (typeName == "single")
    {
        return "single";
    }
    if (typeName == "repeatedx" || typeName == "repeated_x")
    {
        return "repeatedX";
    }
    if (typeName == "repeatedy" || typeName == "repeated_y")
    {
        return "repeatedY";
    }
    return "repeatedXY";
}

bool backgroundTypeIsSingle(const nlohmann::json& background)
{
    return backgroundRepeatTypeName(background) == "single";
}

bool backgroundTypeRepeatsX(const nlohmann::json& background)
{
    const std::string typeName = backgroundRepeatTypeName(background);
    return typeName == "repeatedX" || typeName == "repeatedXY";
}

bool backgroundTypeRepeatsY(const nlohmann::json& background)
{
    const std::string typeName = backgroundRepeatTypeName(background);
    return typeName == "repeatedY" || typeName == "repeatedXY";
}

int backgroundTypeComboIndex(const nlohmann::json& background)
{
    const std::string typeName = backgroundRepeatTypeName(background);
    if (typeName == "single")
    {
        return 0;
    }
    if (typeName == "repeatedX")
    {
        return 1;
    }
    if (typeName == "repeatedY")
    {
        return 2;
    }
    return 3;
}

void setBackgroundTypeFromCombo(nlohmann::json& background, const int comboIndex)
{
    static constexpr std::array<const char*, 4> kTypeValues{
        "single",
        "repeatedX",
        "repeatedY",
        "repeatedXY"
    };
    background["Type"] = kTypeValues[std::clamp(comboIndex, 0, static_cast<int>(kTypeValues.size()) - 1)];
}

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

const std::array<const char*, 23> kAnimatedDecorationNames{
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
    "portalViolet",
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
    Portals,
    Interactives,
    MiniLocations,
    Hazards
};

constexpr std::size_t kEditorTabCount = 10u;

enum class SelectionKind
{
    None,
    Spawn,
    Trader,
    Platform,
    Decoration,
    Background,
    Ground,
    Actors,
    Spawner,
    Portal,
    Interactive,
    MiniLocation,
    DeadArea,
    Barrier
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

struct TraderInteractionState
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
    Resize,
    MoveActivation,
    ResizeActivation
};

struct SpawnerInteractionState
{
    SpawnerInteractionMode mode = SpawnerInteractionMode::None;
    std::size_t index = 0u;
    sf::Vector2f startWorld{0.f, 0.f};
    sf::FloatRect startBounds{};
    sf::FloatRect startActivationBounds{};

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
        startActivationBounds = {};
    }
};

enum class PortalInteractionMode
{
    None,
    Move,
    MoveActivation,
    ResizeActivation
};

struct PortalInteractionState
{
    PortalInteractionMode mode = PortalInteractionMode::None;
    std::size_t index = 0u;
    sf::Vector2f startWorld{0.f, 0.f};
    sf::Vector2f startPosition{0.f, 0.f};
    sf::FloatRect startActivationBounds{};

    bool active() const
    {
        return mode != PortalInteractionMode::None;
    }

    void clear()
    {
        mode = PortalInteractionMode::None;
        index = 0u;
        startWorld = {0.f, 0.f};
        startPosition = {0.f, 0.f};
        startActivationBounds = {};
    }
};

enum class HazardInteractionMode
{
    None,
    Move,
    Resize
};

struct HazardInteractionState
{
    HazardInteractionMode mode = HazardInteractionMode::None;
    SelectionKind kind = SelectionKind::None;
    std::size_t index = 0u;
    sf::Vector2f startWorld{0.f, 0.f};
    sf::FloatRect startBounds{};

    bool active() const
    {
        return mode != HazardInteractionMode::None;
    }

    void clear()
    {
        mode = HazardInteractionMode::None;
        kind = SelectionKind::None;
        index = 0u;
        startWorld = {0.f, 0.f};
        startBounds = {};
    }
};

enum class MiniLocationInteractionMode
{
    None,
    Move,
    Resize,
    MoveSpawn,
    MoveEntry,
    MoveExit,
    MoveNestedPlatform,
    MoveNestedDecoration,
    MoveNestedInteractive,
    MoveNestedPortal,
    ScaleNestedDecoration,
    ScaleNestedInteractive,
    ScaleNestedPortal,
    MoveDeadArea,
    ResizeDeadArea,
    MoveBarrier,
    ResizeBarrier
};

enum class MiniLocationContentKind
{
    None,
    Spawn,
    Platform,
    Decoration,
    Interactive,
    Portal,
    DeadArea
};

enum class MiniLocationContentCollection
{
    None,
    Platforms,
    Decorations,
    Interactives,
    Portals,
    DeadAreas,
    Barriers,
    SpawnPosition,
    Entry,
    Exit
};

enum class MiniLocationSubTarget
{
    Body,
    ResizeHandle
};

enum class MiniLocationTool
{
    SelectMove,
    Platform,
    Decoration,
    Interactive,
    Portal,
    DeadArea,
    Barrier,
    Spawn
};

struct MiniLocationObjectRef
{
    std::size_t locationIndex = 0u;
    MiniLocationContentCollection collection = MiniLocationContentCollection::None;
    std::size_t index = 0u;
    MiniLocationSubTarget subTarget = MiniLocationSubTarget::Body;

    bool isValid() const
    {
        return collection != MiniLocationContentCollection::None;
    }

    void clear()
    {
        locationIndex = 0u;
        collection = MiniLocationContentCollection::None;
        index = 0u;
        subTarget = MiniLocationSubTarget::Body;
    }

    bool sameObject(const MiniLocationObjectRef& other) const
    {
        return locationIndex == other.locationIndex &&
            collection == other.collection &&
            index == other.index;
    }
};

struct MiniLocationHit
{
    MiniLocationObjectRef ref{};
    sf::FloatRect bounds{};
    int priority = 0;
};

struct MiniLocationEditorState
{
    bool contentMode = false;
    MiniLocationTool tool = MiniLocationTool::SelectMove;
    MiniLocationObjectRef selected{};
    MiniLocationObjectRef hover{};
    std::vector<MiniLocationHit> lastHits{};
    sf::Vector2f lastHitWorld{0.f, 0.f};
    std::size_t cycleIndex = 0u;

    void clearSelection()
    {
        selected.clear();
        hover.clear();
        cycleIndex = 0u;
        lastHits.clear();
    }
};

struct MiniLocationInteractionState
{
    MiniLocationInteractionMode mode = MiniLocationInteractionMode::None;
    std::size_t index = 0u;
    sf::Vector2f startWorld{0.f, 0.f};
    sf::FloatRect startBounds{};
    sf::Vector2f startSpawnPosition{0.f, 0.f};
    sf::Vector2f startNestedPosition{0.f, 0.f};
    sf::Vector2f startNestedScale{1.f, 1.f};
    sf::FloatRect startDeadAreaRect{};
    std::size_t nestedIndex = 0u;
    nlohmann::json startLocation{};

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
        startSpawnPosition = {0.f, 0.f};
        startNestedPosition = {0.f, 0.f};
        startNestedScale = {1.f, 1.f};
        startDeadAreaRect = {};
        nestedIndex = 0u;
        startLocation = {};
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
    ImGui::SetNextItemWidth(220.f);
    if (ImGui::InputFloat2(label, raw, "%.4f", ImGuiInputTextFlags_EnterReturnsTrue))
    {
        object["ParallaxFactor"] = nlohmann::json::array({raw[0], raw[1]});
        return true;
    }

    bool changed = false;
    ImGui::PushID(label);
    ImGui::TextDisabled("X");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.f);
    changed |= ImGui::DragFloat("##parallax_x", &raw[0], 0.001f, -10.f, 10.f, "%.4f");
    ImGui::SameLine();
    ImGui::TextDisabled("Y");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.f);
    changed |= ImGui::DragFloat("##parallax_y", &raw[1], 0.001f, -10.f, 10.f, "%.4f");
    ImGui::PopID();

    if (changed)
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
            "Dark Gate Editor",
            (sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize)
        )
        , worldView_({0.f, 0.f}, {1600.f, 900.f})
    {
        window_.setFramerateLimit(60u);
        applyEmbeddedWindowIcon(window_);

        if (!font_.openFromFile("fonts/Roboto_Condensed-Black.ttf"))
        {
            throw std::runtime_error("Editor font was not loaded");
        }

        showLoadingScreen("Loading editor assets...", 0.08f);
        auto gameDataFuture = std::async(std::launch::async, [this]() {
            return std::make_unique<GameData>(&font_);
        });
        gameData_ = waitForLoadingTask(gameDataFuture, "Loading editor assets...", 0.08f, 0.34f);
        showLoadingScreen("Initializing editor UI...", 0.34f);
        if (!ImGui::SFML::Init(window_))
        {
            throw std::runtime_error("Failed to initialize ImGui-SFML for level editor");
        }

        loadEditorImGuiFont();
        ImGui::GetIO().IniFilename = "data/level_editor_imgui.ini";
        applyEditorStyle();
        showLoadingScreen("Building asset catalogs...", 0.52f);
        buildCatalogs();
        showLoadingScreen("Scanning levels...", 0.68f);
        refreshRegistry();

        if (!registry_.getLevels().empty())
        {
            showLoadingScreen("Opening first level...", 0.84f);
            loadLevel(registry_.getLevels().front().filePath);
        }
        else
        {
            showLoadingScreen("Creating default level...", 0.84f);
            createDefaultLevel();
        }
        showLoadingScreen("Ready", 1.f);
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
            uiDeltaSeconds_ = deltaTime.asSeconds();
            ImGui::SFML::Update(window_, deltaTime);
            updateUiAnimations();
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
    bool placingPlatformHitbox_ = false;
    bool topDrawerExpanded_ = false;
    bool leftDrawerExpanded_ = true;
    bool rightDrawerExpanded_ = true;
    float uiDeltaSeconds_ = 1.f / 60.f;
    float topDrawerReveal_ = 0.f;
    float leftDrawerReveal_ = 1.f;
    float rightDrawerReveal_ = 1.f;
    std::array<float, kEditorTabCount> tabEmphasis_{};

    bool draggingView_ = false;
    sf::Vector2i lastDragPixel_{};
    PendingSelectionCycleState pendingSelectionCycle_{};
    SpawnInteractionState spawnInteraction_{};
    TraderInteractionState traderInteraction_{};
    PlatformInteractionState platformInteraction_{};
    DecorationInteractionState decorationInteraction_{};
    InteractiveInteractionState interactiveInteraction_{};
    SpawnerInteractionState spawnerInteraction_{};
    PortalInteractionState portalInteraction_{};
    HazardInteractionState hazardInteraction_{};
    MiniLocationInteractionState miniLocationInteraction_{};
    GroundInteractionState groundInteraction_{};
    bool openWorldContextMenu_ = false;
    bool openMiniLocationContextMenu_ = false;
    sf::Vector2f lastMouseWorldPosition_{0.f, 0.f};
    nlohmann::json miniLocationClipboard_{};
    MiniLocationContentCollection miniLocationClipboardCollection_ = MiniLocationContentCollection::None;
    MiniLocationEditorState miniEditor_{};

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

    ImVec2 editorDisplaySize() const
    {
        const sf::Vector2u size = window_.getSize();
        return ImVec2(static_cast<float>(size.x), static_cast<float>(size.y));
    }

    void loadEditorImGuiFont() const
    {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        const std::filesystem::path fontPath = "fonts/Roboto_Condensed-Black.ttf";
        ImFontConfig fontConfig;
        fontConfig.OversampleH = 3;
        fontConfig.OversampleV = 2;

        if (std::filesystem::exists(fontPath))
        {
            if (io.Fonts->AddFontFromFileTTF(
                    fontPath.string().c_str(),
                    18.f,
                    &fontConfig,
                    io.Fonts->GetGlyphRangesCyrillic()) == nullptr)
            {
                io.Fonts->AddFontDefault();
            }
        }
        else
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

    float smoothApproach(const float current, const float target, const float speed) const
    {
        const float blend = 1.f - std::exp(-speed * std::max(uiDeltaSeconds_, 0.0001f));
        return current + (target - current) * blend;
    }

    template <typename T>
    T waitForLoadingTask(std::future<T>& future, const std::string& message, const float startProgress, const float endProgress)
    {
        sf::Clock loadingClock;
        while (future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        {
            while (const std::optional event = window_.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window_.close();
                }
            }

            const float elapsed = loadingClock.getElapsedTime().asSeconds();
            const float drift = 1.f - std::exp(-elapsed * 1.35f);
            const float progress = startProgress + (endProgress - startProgress) * std::min(drift, 0.96f);
            showLoadingScreen(message, progress);
            sf::sleep(sf::milliseconds(16));
        }

        return future.get();
    }

    void showLoadingScreen(const std::string& message, const float progress)
    {
        window_.setView(window_.getDefaultView());
        window_.clear(sf::Color(13, 12, 16, 255));

        const sf::Vector2u size = window_.getSize();
        const float clampedProgress = std::clamp(progress, 0.f, 1.f);
        sf::Text title(font_);
        title.setCharacterSize(34u);
        title.setFillColor(sf::Color(236, 232, 220, 255));
        title.setString("Dark Gate Editor");
        setTextOriginToMiddle(title);
        title.setPosition({static_cast<float>(size.x) * 0.5f, static_cast<float>(size.y) * 0.5f - 32.f});
        window_.draw(title);

        sf::Text status(font_);
        status.setCharacterSize(18u);
        status.setFillColor(sf::Color(164, 202, 214, 230));
        status.setString(message);
        setTextOriginToMiddle(status);
        status.setPosition({static_cast<float>(size.x) * 0.5f, static_cast<float>(size.y) * 0.5f + 18.f});
        window_.draw(status);

        sf::Text percent(font_);
        percent.setCharacterSize(15u);
        percent.setFillColor(sf::Color(236, 232, 220, 210));
        percent.setString(std::to_string(static_cast<int>(std::round(clampedProgress * 100.f))) + "%");
        setTextOriginToMiddle(percent);
        percent.setPosition({static_cast<float>(size.x) * 0.5f, static_cast<float>(size.y) * 0.5f + 48.f});
        window_.draw(percent);

        constexpr float kBarWidth = 320.f;
        sf::RectangleShape barBack({kBarWidth, 5.f});
        barBack.setOrigin({kBarWidth * 0.5f, 2.5f});
        barBack.setPosition({static_cast<float>(size.x) * 0.5f, static_cast<float>(size.y) * 0.5f + 58.f});
        barBack.setFillColor(sf::Color(255, 255, 255, 36));
        window_.draw(barBack);

        sf::RectangleShape barFill({kBarWidth * clampedProgress, 5.f});
        barFill.setOrigin({0.f, 2.5f});
        barFill.setPosition({
            static_cast<float>(size.x) * 0.5f - kBarWidth * 0.5f,
            static_cast<float>(size.y) * 0.5f + 58.f
        });
        barFill.setFillColor(sf::Color(128, 216, 214, 238));
        window_.draw(barFill);

        window_.display();
    }

    static float saturate(const float value)
    {
        return std::clamp(value, 0.f, 1.f);
    }

    static ImVec4 mixColor(const ImVec4& a, const ImVec4& b, const float t)
    {
        const float blend = saturate(t);
        return ImVec4(
            a.x + (b.x - a.x) * blend,
            a.y + (b.y - a.y) * blend,
            a.z + (b.z - a.z) * blend,
            a.w + (b.w - a.w) * blend
        );
    }

    static float easeOutCubic(const float t)
    {
        const float clamped = saturate(t);
        const float inv = 1.f - clamped;
        return 1.f - inv * inv * inv;
    }

    float toolbarHeight() const
    {
        return 122.f;
    }

    float drawerHeaderHeight() const
    {
        return 34.f;
    }

    float sideHandleWidth() const
    {
        return 44.f;
    }

    float sidebarWidth() const
    {
        return std::clamp(editorDisplaySize().x * 0.19f, 272.f, 336.f);
    }

    float inspectorWidth() const
    {
        return std::clamp(editorDisplaySize().x * 0.24f, 336.f, 430.f);
    }

    float topVisibleHeight() const
    {
        return drawerHeaderHeight() + (toolbarHeight() - drawerHeaderHeight()) * easeOutCubic(topDrawerReveal_);
    }

    float currentSidebarWidth() const
    {
        return sideHandleWidth() + (sidebarWidth() - sideHandleWidth()) * easeOutCubic(leftDrawerReveal_);
    }

    float currentInspectorWidth() const
    {
        return sideHandleWidth() + (inspectorWidth() - sideHandleWidth()) * easeOutCubic(rightDrawerReveal_);
    }

    float panelTop() const
    {
        return topVisibleHeight() + 8.f;
    }

    float panelHeight() const
    {
        return std::max(260.f, editorDisplaySize().y - panelTop() - 8.f);
    }

    float objectListHeight() const
    {
        return std::clamp(panelHeight() * 0.24f, 128.f, 240.f);
    }

    void drawWindowHeadline(const char* title, const char* subtitle) const
    {
        ImGui::TextColored(ImVec4(0.96f, 0.82f, 0.58f, 1.f), "%s", title);
        if (subtitle != nullptr && subtitle[0] != '\0')
        {
            ImGui::TextWrapped("%s", subtitle);
        }
        ImGui::Spacing();
    }

    void drawInfoLine(const char* label, const std::string& value) const
    {
        ImGui::TextDisabled("%s", label);
        ImGui::SameLine(92.f);
        ImGui::TextWrapped("%s", value.c_str());
    }

    const char* editorTabLabel(const EditorTab tab) const
    {
        switch (tab)
        {
        case EditorTab::Level:
            return "Level";
        case EditorTab::Platforms:
            return "Platforms";
        case EditorTab::Decorations:
            return "Decorations";
        case EditorTab::Backgrounds:
            return "Background";
        case EditorTab::Ground:
            return "Ground";
        case EditorTab::Spawners:
            return "Spawners";
        case EditorTab::Portals:
            return "Portals";
        case EditorTab::Interactives:
            return "Interactives";
        case EditorTab::Hazards:
            return "Hazards";
        case EditorTab::MiniLocations:
            return "Mini Locations";
        default:
            return "Level";
        }
    }

    std::size_t editorTabIndex(const EditorTab tab) const
    {
        switch (tab)
        {
        case EditorTab::Level:
            return 0u;
        case EditorTab::Platforms:
            return 1u;
        case EditorTab::Decorations:
            return 2u;
        case EditorTab::Backgrounds:
            return 3u;
        case EditorTab::Ground:
            return 4u;
        case EditorTab::Spawners:
            return 5u;
        case EditorTab::Portals:
            return 6u;
        case EditorTab::Interactives:
            return 7u;
        case EditorTab::Hazards:
            return 8u;
        case EditorTab::MiniLocations:
            return 9u;
        default:
            return 0u;
        }
    }

    void updateUiAnimations()
    {
        topDrawerReveal_ = smoothApproach(topDrawerReveal_, topDrawerExpanded_ ? 1.f : 0.f, 12.f);
        leftDrawerReveal_ = smoothApproach(leftDrawerReveal_, leftDrawerExpanded_ ? 1.f : 0.f, 11.f);
        rightDrawerReveal_ = smoothApproach(rightDrawerReveal_, rightDrawerExpanded_ ? 1.f : 0.f, 11.f);

        for (std::size_t index = 0; index < tabEmphasis_.size(); ++index)
        {
            const float target = index == editorTabIndex(activeTab_) ? 1.f : 0.f;
            tabEmphasis_[index] = smoothApproach(tabEmphasis_[index], target, 14.f);
        }
    }

    void drawPanelBackdrop(const ImVec2& accentSize = ImVec2(220.f, 64.f))
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 min = ImGui::GetWindowPos();
        const ImVec2 max = ImVec2(min.x + ImGui::GetWindowSize().x, min.y + ImGui::GetWindowSize().y);
        drawList->AddRectFilled(min, max, ImGui::GetColorU32(ImVec4(0.055f, 0.065f, 0.085f, 0.90f)), 16.f);
        drawList->AddRect(min, max, ImGui::GetColorU32(ImVec4(0.24f, 0.35f, 0.48f, 0.70f)), 16.f, 0, 1.3f);
        drawList->AddRectFilledMultiColor(
            min,
            ImVec2(min.x + accentSize.x, min.y + accentSize.y),
            ImGui::GetColorU32(ImVec4(0.28f, 0.48f, 0.88f, 0.20f)),
            ImGui::GetColorU32(ImVec4(0.08f, 0.18f, 0.36f, 0.03f)),
            ImGui::GetColorU32(ImVec4(0.08f, 0.18f, 0.36f, 0.00f)),
            ImGui::GetColorU32(ImVec4(0.28f, 0.48f, 0.88f, 0.08f))
        );

        const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(ImGui::GetTime()) * 1.35f);
        drawList->AddLine(
            ImVec2(min.x + 18.f, min.y + 18.f),
            ImVec2(max.x - 18.f, min.y + 18.f),
            ImGui::GetColorU32(mixColor(ImVec4(0.26f, 0.58f, 1.0f, 0.12f), ImVec4(0.98f, 0.63f, 0.28f, 0.38f), pulse)),
            2.f
        );
    }

    bool drawSectionChip(const EditorTab tab)
    {
        const std::size_t index = editorTabIndex(tab);
        const float emphasis = tabEmphasis_[index];
        const bool selected = activeTab_ == tab;
        const char* label = editorTabLabel(tab);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        const ImVec2 textSize = ImGui::CalcTextSize(label);
        const ImVec2 size(textSize.x + 26.f, 34.f);

        ImGui::PushID(static_cast<int>(index));
        const bool pressed = ImGui::InvisibleButton("section_chip", size);
        const bool hovered = ImGui::IsItemHovered();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        const float hoverMix = hovered ? 1.f : 0.f;
        const ImVec4 fill = mixColor(
            mixColor(ImVec4(0.10f, 0.12f, 0.16f, 0.88f), ImVec4(0.13f, 0.18f, 0.26f, 0.94f), emphasis),
            ImVec4(0.20f, 0.28f, 0.38f, 0.96f),
            hoverMix * 0.45f
        );
        const ImVec4 border = mixColor(ImVec4(0.19f, 0.26f, 0.35f, 0.80f), ImVec4(0.93f, 0.58f, 0.24f, 0.96f), emphasis);
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImGui::GetColorU32(fill), 10.f);
        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImGui::GetColorU32(border), 10.f, 0, 1.25f);
        drawList->AddRectFilled(
            ImVec2(pos.x, pos.y + size.y - 3.f),
            ImVec2(pos.x + size.x * (0.18f + 0.82f * emphasis), pos.y + size.y),
            ImGui::GetColorU32(mixColor(ImVec4(0.20f, 0.58f, 1.f, 0.28f), ImVec4(0.97f, 0.62f, 0.24f, 0.95f), emphasis)),
            10.f
        );
        drawList->AddText(
            ImVec2(pos.x + 13.f, pos.y + (size.y - textSize.y) * 0.5f),
            ImGui::GetColorU32(selected ? ImVec4(0.98f, 0.97f, 0.94f, 1.f) : ImVec4(0.72f, 0.78f, 0.88f, 1.f)),
            label
        );
        ImGui::PopID();

        if (pressed)
        {
            activeTab_ = tab;
        }
        return pressed;
    }

    void drawSectionSelector()
    {
        ImGui::TextColored(ImVec4(0.67f, 0.76f, 0.91f, 1.f), "Workspace");
        ImGui::Spacing();
        const std::array<EditorTab, kEditorTabCount> tabs{
            EditorTab::Level,
            EditorTab::Platforms,
            EditorTab::Decorations,
            EditorTab::Backgrounds,
            EditorTab::Ground,
            EditorTab::Spawners,
            EditorTab::Portals,
            EditorTab::Interactives,
            EditorTab::Hazards,
            EditorTab::MiniLocations
        };

        float rowWidth = 0.f;
        const float available = ImGui::GetContentRegionAvail().x;
        for (const EditorTab tab : tabs)
        {
            const ImVec2 textSize = ImGui::CalcTextSize(editorTabLabel(tab));
            const float width = textSize.x + 26.f;
            if (rowWidth > 0.f && rowWidth + 8.f + width > available)
            {
                rowWidth = 0.f;
            }
            else if (rowWidth > 0.f)
            {
                ImGui::SameLine();
                rowWidth += 8.f;
            }
            drawSectionChip(tab);
            rowWidth += width;
        }
        ImGui::Spacing();
    }

    void drawDrawerCaption(const char* arrowLabel, bool* expanded, const char* title, const ImGuiDir openDir, const ImGuiDir closedDir)
    {
        if (ImGui::ArrowButton(arrowLabel, *expanded ? openDir : closedDir))
        {
            *expanded = !*expanded;
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.96f, 0.82f, 0.58f, 1.f), "%s", title);
        ImGui::SameLine();
        ImGui::TextDisabled("panel");
    }

    std::string selectedObjectLabel() const
    {
        switch (selection_.kind)
        {
        case SelectionKind::Spawn:
            return "Player spawn";
        case SelectionKind::Trader:
            return "Trader";
        case SelectionKind::Platform:
            if (selection_.index < document_["Platforms"].size())
            {
                return document_["Platforms"][selection_.index].value("Type", std::string{"Platform"});
            }
            return "Platform";
        case SelectionKind::Decoration:
            if (selection_.index < document_["Decorations"].size())
            {
                return document_["Decorations"][selection_.index].value("Name", std::string{"Decoration"});
            }
            return "Decoration";
        case SelectionKind::Background:
            if (selection_.index < document_["Background"].size())
            {
                return document_["Background"][selection_.index].value("BgName", std::string{"Background"});
            }
            return "Background";
        case SelectionKind::Ground:
            if (selection_.index < document_["Ground"].size())
            {
                return document_["Ground"][selection_.index].value("GroundStyle", std::string{"Ground"});
            }
            return "Ground";
        case SelectionKind::Actors:
            return "Actors";
        case SelectionKind::Spawner:
            if (selection_.index < document_["Spawners"].size())
            {
                return document_["Spawners"][selection_.index].value("EnemyName", std::string{"Spawner"});
            }
            return "Spawner";
        case SelectionKind::Portal:
            if (selection_.index < document_["Portals"].size())
            {
                return document_["Portals"][selection_.index].value("Title", std::string{"Portal"});
            }
            return "Portal";
        case SelectionKind::Interactive:
            if (selection_.index < document_["Interactives"].size())
            {
                const auto& interactive = document_["Interactives"][selection_.index];
                return interactive.value("Title", interactive.value("Type", std::string{"Interactive"}));
            }
            return "Interactive";
        case SelectionKind::MiniLocation:
            if (selection_.index < document_["MiniLocations"].size())
            {
                return document_["MiniLocations"][selection_.index].value("Title", std::string{"Mini Location"});
            }
            return "Mini Location";
        case SelectionKind::DeadArea:
            if (selection_.index < document_["DeadAreas"].size())
            {
                return document_["DeadAreas"][selection_.index].value("Id", std::string{"DeadArea"});
            }
            return "DeadArea";
        case SelectionKind::Barrier:
            if (selection_.index < document_["Barriers"].size())
            {
                return document_["Barriers"][selection_.index].value("Id", std::string{"Barrier"});
            }
            return "Barrier";
        default:
            return "Nothing selected";
        }
    }

    void applyEditorStyle()
    {
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 16.f;
        style.ChildRounding = 14.f;
        style.FrameRounding = 11.f;
        style.GrabRounding = 10.f;
        style.PopupRounding = 12.f;
        style.ScrollbarRounding = 12.f;
        style.TabRounding = 10.f;
        style.WindowBorderSize = 0.f;
        style.ChildBorderSize = 1.f;
        style.FrameBorderSize = 0.f;
        style.PopupBorderSize = 1.f;
        style.FramePadding = ImVec2(12.f, 8.f);
        style.ItemSpacing = ImVec2(10.f, 10.f);
        style.ItemInnerSpacing = ImVec2(8.f, 6.f);
        style.WindowPadding = ImVec2(14.f, 14.f);
        style.CellPadding = ImVec2(8.f, 6.f);
        style.WindowTitleAlign = ImVec2(0.03f, 0.5f);
        style.ScrollbarSize = 12.f;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.03f, 0.05f, 0.08f, 0.84f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.09f, 0.13f, 0.78f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.08f, 0.12f, 0.98f);
        colors[ImGuiCol_Border] = ImVec4(0.24f, 0.35f, 0.48f, 0.72f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.11f, 0.16f, 0.96f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.16f, 0.23f, 0.98f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.22f, 0.31f, 1.f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.08f, 0.11f, 0.96f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.13f, 0.18f, 0.98f);
        colors[ImGuiCol_Header] = ImVec4(0.14f, 0.21f, 0.30f, 0.86f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.30f, 0.43f, 0.92f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.92f, 0.58f, 0.23f, 0.88f);
        colors[ImGuiCol_Button] = ImVec4(0.12f, 0.20f, 0.31f, 0.94f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.30f, 0.46f, 0.96f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.94f, 0.58f, 0.23f, 0.92f);
        colors[ImGuiCol_Text] = ImVec4(0.92f, 0.95f, 1.f, 1.f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.56f, 0.63f, 0.74f, 1.f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.70f, 0.30f, 1.f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.38f, 0.70f, 1.f, 0.90f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.70f, 0.30f, 1.f);
        colors[ImGuiCol_Separator] = ImVec4(0.22f, 0.33f, 0.45f, 0.76f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.65f, 1.f, 0.34f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.47f, 0.74f, 1.f, 0.58f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.70f, 0.30f, 0.80f);
        colors[ImGuiCol_Tab] = ImVec4(0.09f, 0.13f, 0.18f, 0.96f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.16f, 0.24f, 0.35f, 0.98f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.24f, 0.38f, 0.58f, 0.98f);
        colors[ImGuiCol_TabDimmed] = ImVec4(0.06f, 0.09f, 0.12f, 0.96f);
        colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.13f, 0.19f, 0.28f, 0.98f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.03f, 0.05f, 0.08f, 0.66f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.16f, 0.24f, 0.34f, 0.90f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.23f, 0.34f, 0.48f, 0.95f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.94f, 0.58f, 0.23f, 0.95f);
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
        registerAnimatedPreview("portalViolet", gameData_->portalVioletTextures);
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
        if (!presets.contains("TraderPosition") && presets.value("LevelId", currentFilePath_.filename().string()) == "level1.json")
        {
            presets["TraderPosition"] = nlohmann::json::array({800.f, 940.f});
        }
        if (!presets.contains("BackgroundTheme"))
        {
            presets["BackgroundTheme"] = "VerdantDawn";
        }
        if (!presets.contains("BackgroundTileOffsetY"))
        {
            presets["BackgroundTileOffsetY"] = 0.f;
        }
        if (!presets.contains("ActorDrawOrder"))
        {
            presets["ActorDrawOrder"] = 3;
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

        for (const char* arrayName : {"Platforms", "Decorations", "Background", "Ground", "Spawners", "Portals", "Interactives", "DeadAreas", "Barriers", "MiniLocations"})
        {
            if (!document_.contains(arrayName) || !document_[arrayName].is_array())
            {
                document_[arrayName] = nlohmann::json::array();
            }
        }

        if (document_.contains("Ground") && document_["Ground"].is_array())
        {
            for (std::size_t index = 0; index < document_["Ground"].size(); ++index)
            {
                auto& ground = document_["Ground"][index];
                if (!ground.contains("EditorDrawOrder") || !ground["EditorDrawOrder"].is_number_integer())
                {
                    ground["EditorDrawOrder"] = fallbackWorldDrawOrder(SelectionKind::Ground, index);
                }
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
                {"BackgroundTheme", "VerdantDawn"},
                {"BackgroundTileOffsetY", 0.f},
                {"ActorDrawOrder", 3}
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
                    {"Type", "repeatedXY"}
                }
            })},
            {"Ground", nlohmann::json::array({
                {
                    {"EditorDrawOrder", 2},
                    {"GroundStyle", groundStyleOptions_.empty() ? "VerdantKeep" : groundStyleOptions_.front()},
                    {"GroundName", groundTileOptions_.empty() ? "TileSetGreen_02.png" : groundTileOptions_.front()},
                    {"Points", {0, 3840}},
                    {"YPos", 980},
                    {"DepthRows", 2},
                    {"Offset", 8.f}
                }
            })},
            {"Spawners", nlohmann::json::array()},
            {"Portals", nlohmann::json::array()},
            {"Interactives", nlohmann::json::array()},
            {"DeadAreas", nlohmann::json::array()},
            {"Barriers", nlohmann::json::array()},
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
        normalizeMiniLocations();
        normalizePortalTargets();
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

    std::string firstMiniLocationId() const
    {
        if (!document_.contains("MiniLocations") || !document_["MiniLocations"].is_array() || document_["MiniLocations"].empty())
        {
            return {};
        }

        const auto& location = document_["MiniLocations"].front();
        return location.value("Id", location.value("Title", std::string{"mini_location_1"}));
    }

    bool miniLocationIdExists(const std::string& id) const
    {
        if (id.empty() || !document_.contains("MiniLocations") || !document_["MiniLocations"].is_array())
        {
            return false;
        }

        for (const auto& location : document_["MiniLocations"])
        {
            if (location.value("Id", location.value("Title", std::string{})) == id)
            {
                return true;
            }
        }
        return false;
    }

    void normalizePortalTargets()
    {
        const sf::Vector2f playerSpawn = readVector2f(document_["Presets"].value("PlayerSpawn", nlohmann::json::array()), {200.f, 900.f});
        const auto normalizeArray = [&](nlohmann::json& portals) {
            if (!portals.is_array())
            {
                return;
            }

            for (auto& portal : portals)
            {
                if (!portal.contains("Target") || !portal["Target"].is_object())
                {
                    continue;
                }

                auto& target = portal["Target"];
                if (target.value("Type", std::string{"Position"}) != "MiniLocation")
                {
                    continue;
                }

                const std::string currentId = target.value("MiniLocationId", std::string{});
                if (!miniLocationIdExists(currentId))
                {
                    target["MiniLocationId"] = firstMiniLocationId();
                }
                target.erase("Position");
                if (target.contains("SpawnPosition"))
                {
                    const sf::Vector2f spawn = readVector2f(target["SpawnPosition"]);
                    if (std::abs(spawn.x - playerSpawn.x) < 0.01f && std::abs(spawn.y - playerSpawn.y) < 0.01f)
                    {
                        target.erase("SpawnPosition");
                    }
                }
            }
        };

        normalizeArray(document_["Portals"]);
        for (auto& location : document_["MiniLocations"])
        {
            if (location.contains("Portals"))
            {
                normalizeArray(location["Portals"]);
            }
        }
    }

    bool saveLevel()
    {
        syncMetadataFromBuffers();
        normalizeMiniLocations();
        normalizePortalTargets();

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

            if (miniEditor_.contentMode &&
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl)))
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::C)
                {
                    copyMiniLocationContentSelection();
                    return;
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::V)
                {
                    pasteMiniLocationContentAt(lastMouseWorldPosition_);
                    return;
                }
            }

            if (keyPressed->scancode == sf::Keyboard::Scancode::Tab && miniEditor_.contentMode && !miniEditor_.lastHits.empty())
            {
                miniEditor_.cycleIndex = (miniEditor_.cycleIndex + 1u) % miniEditor_.lastHits.size();
                miniEditor_.selected = miniEditor_.lastHits[miniEditor_.cycleIndex].ref;
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
            lastMouseWorldPosition_ = worldPosition;

            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                if (selectedMiniLocationForContent() != nullptr)
                {
                    if (miniEditor_.tool != MiniLocationTool::SelectMove)
                    {
                        handleMiniLocationContentLeftClick(worldPosition);
                        return;
                    }
                    if (beginMiniLocationInteraction(worldPosition))
                    {
                        return;
                    }
                    handleMiniLocationContentLeftClick(worldPosition);
                    return;
                }
                if (startPendingSelectionCycle(worldPosition, mousePressed->position))
                {
                    return;
                }
                if (selection_.isValid() &&
                    selectionContainsPoint(selection_, worldPosition) &&
                    beginInteractionForSelection(selection_, worldPosition))
                {
                    return;
                }
                if (beginSpawnInteraction(worldPosition))
                {
                    return;
                }
                if (beginTraderInteraction(worldPosition))
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
                if (beginPortalInteraction(worldPosition))
                {
                    return;
                }
                if (beginHazardInteraction(worldPosition))
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
                if (selectedMiniLocationForContent() != nullptr)
                {
                    selectMiniLocationContentAt(worldPosition);
                    openMiniLocationContextMenu_ = true;
                    return;
                }
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
                finishTraderInteraction();
                finishInteractiveInteraction();
                finishSpawnerInteraction();
                finishPortalInteraction();
                finishHazardInteraction();
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
            lastMouseWorldPosition_ = window_.mapPixelToCoords(mouseMoved->position, worldView_);
            if (pendingSelectionCycle_.active)
            {
                const sf::Vector2i pixelDelta = mouseMoved->position - pendingSelectionCycle_.startPixel;
                if ((pixelDelta.x * pixelDelta.x + pixelDelta.y * pixelDelta.y) >= 9)
                {
                    pendingSelectionCycle_.clear();

                    const sf::Vector2f movedWorldPosition = window_.mapPixelToCoords(mouseMoved->position, worldView_);
                    if (beginSpawnInteraction(movedWorldPosition) ||
                        beginTraderInteraction(movedWorldPosition) ||
                        beginInteractiveInteraction(movedWorldPosition) ||
                        beginSpawnerInteraction(movedWorldPosition) ||
                        beginPortalInteraction(movedWorldPosition) ||
                        beginHazardInteraction(movedWorldPosition) ||
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
            if (traderInteraction_.active)
            {
                updateTraderInteraction(window_.mapPixelToCoords(mouseMoved->position, worldView_));
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
            if (portalInteraction_.active())
            {
                updatePortalInteraction(window_.mapPixelToCoords(mouseMoved->position, worldView_));
                return;
            }
            if (hazardInteraction_.active())
            {
                updateHazardInteraction(window_.mapPixelToCoords(mouseMoved->position, worldView_));
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

            if (selectedMiniLocationForContent() != nullptr)
            {
                const sf::Vector2f hoverWorld = window_.mapPixelToCoords(mouseMoved->position, worldView_);
                const auto hits = hitTestMiniLocationContent(selection_.index, hoverWorld);
                miniEditor_.hover = hits.empty() ? MiniLocationObjectRef{} : hits.front().ref;
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
        if (handleMiniLocationContentLeftClick(worldPosition))
        {
            return;
        }

        if (placingPlatformHitbox_)
        {
            placePlatformHitbox(worldPosition);
            return;
        }

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
        case SelectionKind::Portal:
            placePortal(worldPosition);
            break;
        case SelectionKind::Interactive:
            placeInteractive(worldPosition);
            break;
        case SelectionKind::DeadArea:
            placeDeadArea(worldPosition);
            break;
        case SelectionKind::Barrier:
            placeBarrier(worldPosition);
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
        const float topY = levelTopY();

        if (document_.contains("MiniLocations") && document_["MiniLocations"].is_array())
        {
            for (const auto& location : document_["MiniLocations"])
            {
                const sf::FloatRect bounds = readRect(location.value("Bounds", nlohmann::json::array()));
                if (position.x >= bounds.position.x && position.x <= bounds.position.x + bounds.size.x)
                {
                    return sf::FloatRect(
                        {bounds.position.x, topY},
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

        return sf::FloatRect({0.f, topY}, {mainWorldRight, levelHeight});
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
        case SelectionKind::Actors:
            return nullptr;
        case SelectionKind::Spawner:
            array = &document_["Spawners"];
            break;
        case SelectionKind::Portal:
            array = &document_["Portals"];
            break;
        case SelectionKind::Interactive:
            array = &document_["Interactives"];
            break;
        case SelectionKind::MiniLocation:
            array = &document_["MiniLocations"];
            break;
        case SelectionKind::DeadArea:
            array = &document_["DeadAreas"];
            break;
        case SelectionKind::Barrier:
            array = &document_["Barriers"];
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
        case SelectionKind::Actors:
            return nullptr;
        case SelectionKind::Spawner:
            return &document_["Spawners"];
        case SelectionKind::Portal:
            return &document_["Portals"];
        case SelectionKind::Interactive:
            return &document_["Interactives"];
        case SelectionKind::MiniLocation:
            return &document_["MiniLocations"];
        case SelectionKind::DeadArea:
            return &document_["DeadAreas"];
        case SelectionKind::Barrier:
            return &document_["Barriers"];
        default:
            return nullptr;
        }
    }

    nlohmann::json* selectedMiniLocationForContent()
    {
        if (!miniEditor_.contentMode || selection_.kind != SelectionKind::MiniLocation || selection_.index >= document_["MiniLocations"].size())
        {
            return nullptr;
        }
        return &document_["MiniLocations"][selection_.index];
    }

    const nlohmann::json* selectedMiniLocationForContent() const
    {
        if (!miniEditor_.contentMode || selection_.kind != SelectionKind::MiniLocation || selection_.index >= document_["MiniLocations"].size())
        {
            return nullptr;
        }
        return &document_["MiniLocations"][selection_.index];
    }

    sf::Vector2f contentRelativePosition(const sf::Vector2f worldPosition, const nlohmann::json& location) const
    {
        return worldPosition - miniLocationBounds(location).position;
    }

    nlohmann::json absoluteNestedObject(const nlohmann::json& nestedObject, const nlohmann::json& location) const
    {
        nlohmann::json absoluteObject = nestedObject;
        absoluteObject["Position"] = toJson(miniLocationBounds(location).position + readVector2f(nestedObject.value("Position", nlohmann::json::array())));
        return absoluteObject;
    }

    sf::FloatRect nestedDecorationBounds(const nlohmann::json& nestedDecoration, const nlohmann::json& location) const
    {
        const sf::Vector2f position = miniLocationBounds(location).position +
            readVector2f(nestedDecoration.value("Position", nlohmann::json::array()));
        return decorationSpriteBounds(
            nestedDecoration.value("Name", std::string{}),
            position,
            decorationScale(nestedDecoration),
            decorationRotation(nestedDecoration)
        );
    }

    sf::FloatRect nestedPlatformHitBounds(const nlohmann::json& nestedPlatform, const nlohmann::json& location) const
    {
        const nlohmann::json absolutePlatform = absoluteNestedObject(nestedPlatform, location);
        const sf::FloatRect physics = platformBounds(absolutePlatform);
        const sf::FloatRect sprite = platformSpriteBounds(absolutePlatform);
        const float left = std::min(physics.position.x, sprite.position.x);
        const float top = std::min(physics.position.y, sprite.position.y);
        const float right = std::max(physics.position.x + physics.size.x, sprite.position.x + sprite.size.x);
        const float bottom = std::max(physics.position.y + physics.size.y, sprite.position.y + sprite.size.y);
        return {{left, top}, {right - left, bottom - top}};
    }

    sf::FloatRect deadAreaBounds(const nlohmann::json& deadArea, const nlohmann::json& location) const
    {
        const sf::FloatRect rect = readRect(deadArea.value("Rect", nlohmann::json::array()));
        return sf::FloatRect(miniLocationBounds(location).position + rect.position, rect.size);
    }

    sf::FloatRect deadAreaResizeHandleBounds(const nlohmann::json& deadArea, const nlohmann::json& location) const
    {
        const sf::FloatRect bounds = deadAreaBounds(deadArea, location);
        return platformHandleBounds(bounds.position + bounds.size);
    }

    sf::FloatRect worldHazardBounds(const nlohmann::json& hazard) const
    {
        return readRect(hazard.value("Rect", nlohmann::json::array()), {{0.f, 0.f}, {160.f, 80.f}});
    }

    sf::FloatRect worldHazardResizeHandleBounds(const nlohmann::json& hazard) const
    {
        const sf::FloatRect bounds = worldHazardBounds(hazard);
        return platformHandleBounds(bounds.position + bounds.size);
    }

    static const char* miniLocationCollectionKey(const MiniLocationContentCollection collection)
    {
        switch (collection)
        {
        case MiniLocationContentCollection::Platforms:
            return "Platforms";
        case MiniLocationContentCollection::Decorations:
            return "Decorations";
        case MiniLocationContentCollection::Interactives:
            return "Interactives";
        case MiniLocationContentCollection::Portals:
            return "Portals";
        case MiniLocationContentCollection::DeadAreas:
            return "DeadAreas";
        case MiniLocationContentCollection::Barriers:
            return "Barriers";
        default:
            return "";
        }
    }

    static bool miniLocationCollectionUsesDrawOrder(const MiniLocationContentCollection collection)
    {
        return collection == MiniLocationContentCollection::Platforms ||
            collection == MiniLocationContentCollection::Decorations ||
            collection == MiniLocationContentCollection::Interactives ||
            collection == MiniLocationContentCollection::Portals;
    }

    static int miniLocationDrawOrderKindRank(const MiniLocationContentCollection collection)
    {
        switch (collection)
        {
        case MiniLocationContentCollection::Decorations:
            return 0;
        case MiniLocationContentCollection::Interactives:
            return 1;
        case MiniLocationContentCollection::Portals:
            return 2;
        case MiniLocationContentCollection::Platforms:
            return 3;
        default:
            return 4;
        }
    }

    int fallbackMiniLocationDrawOrder(const MiniLocationContentCollection collection, const std::size_t index) const
    {
        int order = nextWorldDrawOrder();
        if (collection == MiniLocationContentCollection::Decorations)
        {
            return order + static_cast<int>(index);
        }
        if (document_.contains("MiniLocations") && document_["MiniLocations"].is_array())
        {
            for (const auto& location : document_["MiniLocations"])
            {
                if (location.contains("Decorations") && location["Decorations"].is_array())
                {
                    order += static_cast<int>(location["Decorations"].size());
                }
            }
        }
        if (collection == MiniLocationContentCollection::Interactives)
        {
            return order + static_cast<int>(index);
        }
        if (document_.contains("MiniLocations") && document_["MiniLocations"].is_array())
        {
            for (const auto& location : document_["MiniLocations"])
            {
                if (location.contains("Interactives") && location["Interactives"].is_array())
                {
                    order += static_cast<int>(location["Interactives"].size());
                }
            }
        }
        if (collection == MiniLocationContentCollection::Portals)
        {
            return order + static_cast<int>(index);
        }
        if (document_.contains("MiniLocations") && document_["MiniLocations"].is_array())
        {
            for (const auto& location : document_["MiniLocations"])
            {
                if (location.contains("Portals") && location["Portals"].is_array())
                {
                    order += static_cast<int>(location["Portals"].size());
                }
            }
        }
        return order + static_cast<int>(index);
    }

    int miniLocationDrawOrder(const nlohmann::json& location, const MiniLocationContentCollection collection, const std::size_t index) const
    {
        const char* key = miniLocationCollectionKey(collection);
        if (key[0] == '\0' || !location.contains(key) || !location[key].is_array() || index >= location[key].size())
        {
            return fallbackMiniLocationDrawOrder(collection, index);
        }

        const auto& object = location[key][index];
        if (object.contains("EditorDrawOrder") && object["EditorDrawOrder"].is_number_integer())
        {
            return object["EditorDrawOrder"].get<int>();
        }

        return fallbackMiniLocationDrawOrder(collection, index);
    }

    void setMiniLocationDrawOrder(nlohmann::json& location, const MiniLocationContentCollection collection, const std::size_t index, const int order)
    {
        const char* key = miniLocationCollectionKey(collection);
        if (key[0] == '\0' || !location.contains(key) || !location[key].is_array() || index >= location[key].size())
        {
            return;
        }

        location[key][index]["EditorDrawOrder"] = order;
    }

    std::vector<MiniLocationObjectRef> miniLocationDrawOrderEntries(const std::size_t locationIndex) const
    {
        std::vector<MiniLocationObjectRef> entries;
        if (!document_.contains("MiniLocations") || !document_["MiniLocations"].is_array() ||
            locationIndex >= document_["MiniLocations"].size())
        {
            return entries;
        }

        const nlohmann::json& location = document_["MiniLocations"][locationIndex];
        const auto pushCollection = [&](const MiniLocationContentCollection collection) {
            const char* key = miniLocationCollectionKey(collection);
            if (key[0] == '\0' || !location.contains(key) || !location[key].is_array())
            {
                return;
            }
            for (std::size_t index = 0; index < location[key].size(); ++index)
            {
                entries.push_back({locationIndex, collection, index, MiniLocationSubTarget::Body});
            }
        };

        pushCollection(MiniLocationContentCollection::Decorations);
        pushCollection(MiniLocationContentCollection::Interactives);
        pushCollection(MiniLocationContentCollection::Portals);
        pushCollection(MiniLocationContentCollection::Platforms);

        std::stable_sort(entries.begin(), entries.end(), [&](const MiniLocationObjectRef& lhs, const MiniLocationObjectRef& rhs) {
            const int lhsOrder = miniLocationDrawOrder(location, lhs.collection, lhs.index);
            const int rhsOrder = miniLocationDrawOrder(location, rhs.collection, rhs.index);
            if (lhsOrder != rhsOrder)
            {
                return lhsOrder < rhsOrder;
            }
            if (lhs.collection != rhs.collection)
            {
                return miniLocationDrawOrderKindRank(lhs.collection) < miniLocationDrawOrderKindRank(rhs.collection);
            }
            return lhs.index < rhs.index;
        });

        return entries;
    }

    int nextMiniLocationDrawOrder(const nlohmann::json& location) const
    {
        int nextOrder = nextWorldDrawOrder();
        const auto scanCollection = [&](const MiniLocationContentCollection collection) {
            const char* key = miniLocationCollectionKey(collection);
            if (key[0] == '\0' || !location.contains(key) || !location[key].is_array())
            {
                return;
            }
            for (std::size_t index = 0; index < location[key].size(); ++index)
            {
                nextOrder = std::max(nextOrder, miniLocationDrawOrder(location, collection, index) + 1);
            }
        };

        scanCollection(MiniLocationContentCollection::Decorations);
        scanCollection(MiniLocationContentCollection::Interactives);
        scanCollection(MiniLocationContentCollection::Portals);
        scanCollection(MiniLocationContentCollection::Platforms);
        return nextOrder;
    }

    bool moveMiniLocationDrawOrder(nlohmann::json& location, const MiniLocationObjectRef& ref, const int delta)
    {
        if (!miniLocationCollectionUsesDrawOrder(ref.collection) ||
            !document_.contains("MiniLocations") || !document_["MiniLocations"].is_array() ||
            ref.locationIndex >= document_["MiniLocations"].size())
        {
            return false;
        }

        std::vector<MiniLocationObjectRef> entries = miniLocationDrawOrderEntries(ref.locationIndex);
        const auto currentIt = std::find_if(entries.begin(), entries.end(), [&](const MiniLocationObjectRef& entry) {
            return entry.sameObject(ref);
        });
        if (currentIt == entries.end())
        {
            return false;
        }

        const long long currentIndex = static_cast<long long>(std::distance(entries.begin(), currentIt));
        const long long newIndex = std::clamp(
            currentIndex + static_cast<long long>(delta),
            0ll,
            static_cast<long long>(entries.size()) - 1ll
        );
        if (newIndex == currentIndex)
        {
            return false;
        }

        const MiniLocationObjectRef target = entries[static_cast<std::size_t>(newIndex)];
        const int currentOrder = miniLocationDrawOrder(location, ref.collection, ref.index);
        const int targetOrder = miniLocationDrawOrder(location, target.collection, target.index);
        setMiniLocationDrawOrder(location, ref.collection, ref.index, targetOrder);
        setMiniLocationDrawOrder(location, target.collection, target.index, currentOrder);
        return true;
    }

    bool moveMiniLocationToDrawOrderEdge(nlohmann::json& location, const MiniLocationObjectRef& ref, const bool toFront)
    {
        if (!miniLocationCollectionUsesDrawOrder(ref.collection))
        {
            return false;
        }

        std::vector<MiniLocationObjectRef> entries = miniLocationDrawOrderEntries(ref.locationIndex);
        if (entries.empty())
        {
            return false;
        }

        const auto currentIt = std::find_if(entries.begin(), entries.end(), [&](const MiniLocationObjectRef& entry) {
            return entry.sameObject(ref);
        });
        if (currentIt == entries.end())
        {
            return false;
        }

        const std::size_t currentIndex = static_cast<std::size_t>(std::distance(entries.begin(), currentIt));
        const std::size_t targetIndex = toFront ? entries.size() - 1u : 0u;
        if (currentIndex == targetIndex)
        {
            return false;
        }

        const int edgeOrder = miniLocationDrawOrder(
            location,
            entries[targetIndex].collection,
            entries[targetIndex].index
        );
        setMiniLocationDrawOrder(location, ref.collection, ref.index, toFront ? edgeOrder + 1 : edgeOrder - 1);
        return true;
    }

    void selectMiniLocationContent(const MiniLocationContentCollection collection, const std::size_t nestedIndex = 0u, const MiniLocationSubTarget subTarget = MiniLocationSubTarget::Body)
    {
        if (selection_.kind != SelectionKind::MiniLocation || selection_.index >= document_["MiniLocations"].size())
        {
            miniEditor_.clearSelection();
            return;
        }

        miniEditor_.selected = {selection_.index, collection, nestedIndex, subTarget};
    }

    sf::FloatRect miniLocationContentBounds(const MiniLocationObjectRef& ref) const
    {
        if (!ref.isValid() || !document_.contains("MiniLocations") || !document_["MiniLocations"].is_array() ||
            ref.locationIndex >= document_["MiniLocations"].size())
        {
            return {};
        }

        const auto& location = document_["MiniLocations"][ref.locationIndex];
        switch (ref.collection)
        {
        case MiniLocationContentCollection::SpawnPosition:
            return miniLocationSpawnMarkerBounds(location);
        case MiniLocationContentCollection::Entry:
        {
            if (!location.contains("Entry") || !location["Entry"].is_object())
            {
                return {};
            }
            const auto& entry = location["Entry"];
            return textureBoundsAt(
                entry.value("Texture", std::string{}),
                readVector2f(entry.value("Position", nlohmann::json::array())),
                readVector2f(entry.value("Scale", nlohmann::json::array()), {0.22f, 0.33f}),
                {72.f, 96.f});
        }
        case MiniLocationContentCollection::Exit:
        {
            if (!location.contains("Exit") || !location["Exit"].is_object())
            {
                return {};
            }
            const auto& exit = location["Exit"];
            return textureBoundsAt(
                exit.value("Texture", std::string{}),
                readVector2f(exit.value("Position", nlohmann::json::array())),
                readVector2f(exit.value("Scale", nlohmann::json::array()), {0.22f, 0.33f}),
                {72.f, 96.f});
        }
        case MiniLocationContentCollection::Platforms:
            if (location.contains("Platforms") && location["Platforms"].is_array() && ref.index < location["Platforms"].size())
            {
                return nestedPlatformHitBounds(location["Platforms"][ref.index], location);
            }
            break;
        case MiniLocationContentCollection::Decorations:
            if (location.contains("Decorations") && location["Decorations"].is_array() && ref.index < location["Decorations"].size())
            {
                return nestedDecorationBounds(location["Decorations"][ref.index], location);
            }
            break;
        case MiniLocationContentCollection::Interactives:
            if (location.contains("Interactives") && location["Interactives"].is_array() && ref.index < location["Interactives"].size())
            {
                return interactiveBounds(absoluteNestedObject(location["Interactives"][ref.index], location));
            }
            break;
        case MiniLocationContentCollection::Portals:
            if (location.contains("Portals") && location["Portals"].is_array() && ref.index < location["Portals"].size())
            {
                return portalBounds(absoluteNestedObject(location["Portals"][ref.index], location));
            }
            break;
        case MiniLocationContentCollection::DeadAreas:
            if (location.contains("DeadAreas") && location["DeadAreas"].is_array() && ref.index < location["DeadAreas"].size())
            {
                return deadAreaBounds(location["DeadAreas"][ref.index], location);
            }
            break;
        case MiniLocationContentCollection::Barriers:
            if (location.contains("Barriers") && location["Barriers"].is_array() && ref.index < location["Barriers"].size())
            {
                return deadAreaBounds(location["Barriers"][ref.index], location);
            }
            break;
        default:
            break;
        }
        return {};
    }

    void placeNestedPlatform(nlohmann::json& location, const sf::Vector2f worldPosition)
    {
        if (platformTypes_.empty())
        {
            return;
        }
        const std::string typeName = platformTypes_.at(std::clamp(selectedPlatformTypeIndex_, 0, static_cast<int>(platformTypes_.size()) - 1));
        ensureMiniLocationNestedArrays(location);
        location["Platforms"].push_back({
            {"Type", typeName},
            {"Position", toJson(contentRelativePosition(worldPosition, location))},
            {"EditorDrawOrder", nextMiniLocationDrawOrder(location)}
        });
        selectMiniLocationContent(MiniLocationContentCollection::Platforms, location["Platforms"].size() - 1u);
        markDirty();
    }

    void placeNestedDecoration(nlohmann::json& location, const sf::Vector2f worldPosition)
    {
        if (decorationOptions_.empty())
        {
            return;
        }
        const std::string name = decorationOptions_.at(std::clamp(selectedDecorationIndex_, 0, static_cast<int>(decorationOptions_.size()) - 1));
        ensureMiniLocationNestedArrays(location);
        location["Decorations"].push_back({
            {"Name", name},
            {"Position", toJson(contentRelativePosition(worldPosition, location))},
            {"Scale", {1.f, 1.f}},
            {"Rotation", 0.f},
            {"Color", {255, 255, 255, 255}},
            {"ParallaxFactor", {1.f, 1.f}},
            {"Z", 0},
            {"EditorDrawOrder", nextMiniLocationDrawOrder(location)}
        });
        selectMiniLocationContent(MiniLocationContentCollection::Decorations, location["Decorations"].size() - 1u);
        markDirty();
    }

    void placeNestedInteractive(nlohmann::json& location, const sf::Vector2f worldPosition)
    {
        ensureMiniLocationNestedArrays(location);
        location["Interactives"].push_back({
            {"Type", "EchoTablet"},
            {"Texture", previewTextureOptions_.empty() ? std::string{} : previewTextureOptions_.front()},
            {"Position", toJson(contentRelativePosition(worldPosition, location))},
            {"Scale", {1.f, 1.f}},
            {"Color", {255, 255, 255, 255}},
            {"AccentColor", {220, 184, 122, 255}},
            {"InteractRadius", 120.f},
            {"RewardGold", 0},
            {"SingleUse", true},
            {"Prompt", "Enter to interact"},
            {"Title", "Pocket Relic"},
            {"Body", "The pocket space keeps this memory close."},
            {"EditorDrawOrder", nextMiniLocationDrawOrder(location)}
        });
        selectMiniLocationContent(MiniLocationContentCollection::Interactives, location["Interactives"].size() - 1u);
        markDirty();
    }

    void placeNestedPortal(nlohmann::json& location, const sf::Vector2f worldPosition)
    {
        ensureMiniLocationNestedArrays(location);
        const int portalIndex = static_cast<int>(location["Portals"].size()) + 1;
        location["Portals"].push_back({
            {"Id", "pocket_portal_" + std::to_string(portalIndex)},
            {"Title", "Pocket Portal " + std::to_string(portalIndex)},
            {"Position", toJson(contentRelativePosition(worldPosition, location))},
            {"Scale", {0.36f, 0.36f}},
            {"PortalTexture", "portalGreen"},
            {"Color", {212, 236, 255, 245}},
            {"AccentColor", {112, 208, 255, 255}},
            {"InteractRadius", 130.f},
            {"Prompt", "Enter portal"},
            {"Target", {{"Type", "Position"}, {"Position", toJson(worldPosition)}}},
            {"EditorDrawOrder", nextMiniLocationDrawOrder(location)}
        });
        selectMiniLocationContent(MiniLocationContentCollection::Portals, location["Portals"].size() - 1u);
        markDirty();
    }

    bool handleMiniLocationContentLeftClick(const sf::Vector2f worldPosition)
    {
        nlohmann::json* location = selectedMiniLocationForContent();
        if (location == nullptr)
        {
            return false;
        }

        switch (miniEditor_.tool)
        {
        case MiniLocationTool::Platform:
            placeNestedPlatform(*location, worldPosition);
            return true;
        case MiniLocationTool::Decoration:
            placeNestedDecoration(*location, worldPosition);
            return true;
        case MiniLocationTool::Interactive:
            placeNestedInteractive(*location, worldPosition);
            return true;
        case MiniLocationTool::Portal:
            placeNestedPortal(*location, worldPosition);
            return true;
        case MiniLocationTool::DeadArea:
        {
            ensureMiniLocationNestedArrays(*location);
            const sf::Vector2f relative = contentRelativePosition(worldPosition, *location);
            const int deadAreaIndex = static_cast<int>((*location)["DeadAreas"].size()) + 1;
            (*location)["DeadAreas"].push_back({
                {"Id", "dead_area_" + std::to_string(deadAreaIndex)},
                {"Enabled", true},
                {"Rect", {relative.x - 120.f, relative.y - 24.f, 240.f, 48.f}},
                {"CoreColor", {242, 104, 56, 255}},
                {"GlowColor", {255, 182, 96, 255}},
                {"EmberColor", {255, 236, 188, 255}}
            });
            selectMiniLocationContent(MiniLocationContentCollection::DeadAreas, (*location)["DeadAreas"].size() - 1u);
            markDirty();
            return true;
        }
        case MiniLocationTool::Barrier:
        {
            ensureMiniLocationNestedArrays(*location);
            const sf::Vector2f relative = contentRelativePosition(worldPosition, *location);
            const int barrierIndex = static_cast<int>((*location)["Barriers"].size()) + 1;
            (*location)["Barriers"].push_back({
                {"Id", "barrier_" + std::to_string(barrierIndex)},
                {"Enabled", true},
                {"BlocksPlayer", true},
                {"Rect", {relative.x - 12.f, relative.y - 110.f, 24.f, 220.f}},
                {"CoreColor", {130, 214, 184, 255}},
                {"GlowColor", {156, 238, 208, 255}}
            });
            selectMiniLocationContent(MiniLocationContentCollection::Barriers, (*location)["Barriers"].size() - 1u);
            markDirty();
            return true;
        }
        case MiniLocationTool::Spawn:
            (*location)["SpawnPosition"] = toJson(contentRelativePosition(worldPosition, *location));
            selectMiniLocationContent(MiniLocationContentCollection::SpawnPosition);
            markDirty();
            return true;
        default:
            break;
        }

        selectMiniLocationContentAt(worldPosition);
        return true;
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
        case SelectionKind::Actors:
            return nullptr;
        case SelectionKind::Spawner:
            return &document_["Spawners"];
        case SelectionKind::Portal:
            return &document_["Portals"];
        case SelectionKind::Interactive:
            return &document_["Interactives"];
        case SelectionKind::MiniLocation:
            return &document_["MiniLocations"];
        case SelectionKind::DeadArea:
            return &document_["DeadAreas"];
        case SelectionKind::Barrier:
            return &document_["Barriers"];
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

    bool usesSharedWorldDrawOrder(const SelectionKind kind) const
    {
        return kind == SelectionKind::Background
            || kind == SelectionKind::Decoration
            || kind == SelectionKind::Ground
            || kind == SelectionKind::Actors
            || kind == SelectionKind::Interactive
            || kind == SelectionKind::Portal
            || kind == SelectionKind::Platform;
    }

    int fallbackWorldDrawOrder(const SelectionKind kind, const std::size_t index) const
    {
        int order = 0;
        if (kind == SelectionKind::Background)
        {
            return static_cast<int>(index);
        }
        order += static_cast<int>(document_["Background"].size());
        if (kind == SelectionKind::Decoration)
        {
            return order + static_cast<int>(index);
        }
        order += static_cast<int>(document_["Decorations"].size());
        if (kind == SelectionKind::Ground)
        {
            return order + static_cast<int>(index);
        }
        order += static_cast<int>(document_["Ground"].size());
        if (kind == SelectionKind::Actors)
        {
            return order;
        }
        ++order;
        if (kind == SelectionKind::Interactive)
        {
            return order + static_cast<int>(index);
        }
        order += static_cast<int>(document_["Interactives"].size());
        if (kind == SelectionKind::Portal)
        {
            return order + static_cast<int>(index);
        }
        order += static_cast<int>(document_["Portals"].size());
        if (kind == SelectionKind::Platform)
        {
            return order + static_cast<int>(index);
        }

        return order;
    }

    int sharedWorldDrawOrder(const SelectionKind kind, const std::size_t index) const
    {
        if (kind == SelectionKind::Actors)
        {
            return document_["Presets"].value("ActorDrawOrder", fallbackWorldDrawOrder(kind, index));
        }

        const nlohmann::json* array = arrayForSelectionKind(kind);
        if (array == nullptr || index >= array->size())
        {
            return fallbackWorldDrawOrder(kind, index);
        }

        const nlohmann::json& object = (*array)[index];
        if (object.contains("EditorDrawOrder") && object["EditorDrawOrder"].is_number_integer())
        {
            return object["EditorDrawOrder"].get<int>();
        }

        return fallbackWorldDrawOrder(kind, index);
    }

    void setSharedWorldDrawOrder(const SelectionKind kind, const std::size_t index, const int order)
    {
        if (kind == SelectionKind::Actors)
        {
            document_["Presets"]["ActorDrawOrder"] = order;
            return;
        }

        nlohmann::json* array = arrayForSelectionKind(kind);
        if (array == nullptr || index >= array->size())
        {
            return;
        }

        (*array)[index]["EditorDrawOrder"] = order;
    }

    std::vector<EditorSelection> sharedWorldDrawOrderSelections() const
    {
        std::vector<EditorSelection> selections;
        selections.reserve(
            document_["Background"].size()
            + document_["Decorations"].size()
            + document_["Ground"].size()
            + document_["Interactives"].size()
            + document_["Portals"].size()
            + document_["Platforms"].size()
            + 1u
        );

        for (std::size_t index = 0; index < document_["Background"].size(); ++index)
        {
            selections.push_back({SelectionKind::Background, index});
        }
        for (std::size_t index = 0; index < document_["Decorations"].size(); ++index)
        {
            selections.push_back({SelectionKind::Decoration, index});
        }
        for (std::size_t index = 0; index < document_["Ground"].size(); ++index)
        {
            selections.push_back({SelectionKind::Ground, index});
        }
        selections.push_back({SelectionKind::Actors, 0u});
        for (std::size_t index = 0; index < document_["Interactives"].size(); ++index)
        {
            selections.push_back({SelectionKind::Interactive, index});
        }
        for (std::size_t index = 0; index < document_["Portals"].size(); ++index)
        {
            selections.push_back({SelectionKind::Portal, index});
        }
        for (std::size_t index = 0; index < document_["Platforms"].size(); ++index)
        {
            selections.push_back({SelectionKind::Platform, index});
        }

        std::stable_sort(
            selections.begin(),
            selections.end(),
            [&](const EditorSelection& lhs, const EditorSelection& rhs) {
                return sharedWorldDrawOrder(lhs.kind, lhs.index) <
                    sharedWorldDrawOrder(rhs.kind, rhs.index);
            }
        );

        return selections;
    }

    void normalizeSharedWorldDrawOrder()
    {
        std::vector<EditorSelection> orderedSelections = sharedWorldDrawOrderSelections();
        for (std::size_t index = 0; index < orderedSelections.size(); ++index)
        {
            setSharedWorldDrawOrder(
                orderedSelections[index].kind,
                orderedSelections[index].index,
                static_cast<int>(index)
            );
        }
    }

    int nextWorldDrawOrder() const
    {
        int nextOrder = 0;
        for (const EditorSelection& entry : sharedWorldDrawOrderSelections())
        {
            nextOrder = std::max(nextOrder, sharedWorldDrawOrder(entry.kind, entry.index) + 1);
        }
        return nextOrder;
    }

    bool moveSharedWorldDrawOrder(const int delta)
    {
        if (!usesSharedWorldDrawOrder(selection_.kind))
        {
            return false;
        }

        std::vector<EditorSelection> orderedSelections = sharedWorldDrawOrderSelections();
        const auto currentIt = std::find(orderedSelections.begin(), orderedSelections.end(), selection_);
        if (currentIt == orderedSelections.end())
        {
            return false;
        }

        const long long currentIndex = static_cast<long long>(std::distance(orderedSelections.begin(), currentIt));
        const long long newIndex = std::clamp(
            currentIndex + static_cast<long long>(delta),
            0ll,
            static_cast<long long>(orderedSelections.size()) - 1ll
        );
        if (newIndex == currentIndex)
        {
            return false;
        }

        const EditorSelection currentSelection = orderedSelections[static_cast<std::size_t>(currentIndex)];
        const EditorSelection targetSelection = orderedSelections[static_cast<std::size_t>(newIndex)];
        const int currentOrder = sharedWorldDrawOrder(currentSelection.kind, currentSelection.index);
        const int targetOrder = sharedWorldDrawOrder(targetSelection.kind, targetSelection.index);

        setSharedWorldDrawOrder(currentSelection.kind, currentSelection.index, targetOrder);
        setSharedWorldDrawOrder(targetSelection.kind, targetSelection.index, currentOrder);

        markDirty();
        return true;
    }

    bool moveSharedWorldToDrawOrderEdge(const bool toFront)
    {
        if (!usesSharedWorldDrawOrder(selection_.kind))
        {
            return false;
        }

        std::vector<EditorSelection> orderedSelections = sharedWorldDrawOrderSelections();
        const auto currentIt = std::find(orderedSelections.begin(), orderedSelections.end(), selection_);
        if (currentIt == orderedSelections.end())
        {
            return false;
        }

        const std::size_t currentIndex = static_cast<std::size_t>(std::distance(orderedSelections.begin(), currentIt));
        const std::size_t targetIndex = toFront ? orderedSelections.size() - 1u : 0u;
        if (currentIndex == targetIndex)
        {
            return false;
        }

        int edgeOrder = sharedWorldDrawOrder(orderedSelections.front().kind, orderedSelections.front().index);
        if (toFront)
        {
            edgeOrder = sharedWorldDrawOrder(orderedSelections.back().kind, orderedSelections.back().index);
        }
        setSharedWorldDrawOrder(selection_.kind, selection_.index, toFront ? edgeOrder + 1 : edgeOrder - 1);

        markDirty();
        return true;
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

    bool platformBounceEnabled(const nlohmann::json& platform) const
    {
        return platform.value("BounceEnabled", true);
    }

    bool isHazardPlatformHitbox(const nlohmann::json& platform) const
    {
        return platform.value("Type", std::string{}) == "Invisible-wall" ||
            platform.value("HazardTool", std::string{}) == "PlatformHitbox";
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

    bool isTraderEditable() const
    {
        return document_.contains("Presets") &&
            document_["Presets"].is_object() &&
            document_["Presets"].value("LevelId", std::string{}) == "level1.json";
    }

    sf::Vector2f traderPosition() const
    {
        return readVector2f(
            document_["Presets"].value("TraderPosition", nlohmann::json::array()),
            {800.f, 940.f}
        );
    }

    sf::FloatRect traderMarkerBounds() const
    {
        const sf::Vector2f position = traderPosition();
        return sf::FloatRect({position.x - 24.f, position.y - 48.f}, {48.f, 64.f});
    }

    bool beginTraderInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None || !isTraderEditable())
        {
            return false;
        }

        const bool canEditTrader = activeTab_ == EditorTab::Level || selection_.kind == SelectionKind::Trader;
        if (!canEditTrader || !traderMarkerBounds().contains(worldPosition))
        {
            return false;
        }

        selection_ = {SelectionKind::Trader, 0u};
        traderInteraction_.active = true;
        return true;
    }

    void updateTraderInteraction(const sf::Vector2f worldPosition)
    {
        if (!traderInteraction_.active)
        {
            return;
        }

        document_["Presets"]["TraderPosition"] = toJson(worldPosition);
        markDirty();
    }

    void finishTraderInteraction()
    {
        traderInteraction_.clear();
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

    float decorationRotation(const nlohmann::json& decoration) const
    {
        return decoration.value("Rotation", 0.f);
    }

    sf::FloatRect decorationSpriteBounds(
        const std::string& textureName,
        const sf::Vector2f position,
        const sf::Vector2f scale,
        const float rotation,
        const sf::Vector2f fallbackSize = {72.f, 72.f}) const
    {
        if (const sf::Texture* texture = findPreviewTexture(textureName); texture != nullptr)
        {
            sf::Sprite sprite(*texture);
            sprite.setOrigin(sprite.getGlobalBounds().getCenter());
            sprite.setPosition(position);
            sprite.setScale(scale);
            sprite.setRotation(sf::degrees(rotation));
            return sprite.getGlobalBounds();
        }

        return sf::FloatRect(
            {position.x - fallbackSize.x * std::abs(scale.x) * 0.5f, position.y - fallbackSize.y * std::abs(scale.y) * 0.5f},
            {fallbackSize.x * std::abs(scale.x), fallbackSize.y * std::abs(scale.y)}
        );
    }

    sf::FloatRect decorationBounds(const nlohmann::json& decoration) const
    {
        return decorationSpriteBounds(
            decoration.value("Name", std::string{}),
            decorationDisplayPosition(decoration),
            decorationScale(decoration),
            decorationRotation(decoration)
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
            else if (decorationContainsPoint(*currentDecoration, worldPosition))
            {
                decorationInteraction_.mode = DecorationInteractionMode::Move;
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

    sf::Vector2f backgroundPreviewParallax(const nlohmann::json& background) const
    {
        const sf::Vector2f parallax = backgroundParallax(background);
        return {1.f - parallax.x, 1.f - parallax.y};
    }

    sf::Vector2i backgroundTileCount() const
    {
        const sf::Vector2f levelSize = readVector2f(
            document_["Presets"].value("Size", nlohmann::json::array()),
            {static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)}
        );
        return {
            std::max(1, static_cast<int>(std::ceil(std::max(levelSize.x, 1.f) / kBackgroundTileSize.x))),
            std::max(1, static_cast<int>(std::ceil(std::max(levelSize.y, 1.f) / kBackgroundTileSize.y)))
        };
    }

    float levelTopY() const
    {
        const float levelHeight = document_["Presets"]["Size"][1].get<float>();
        return std::min(0.f, static_cast<float>(WINDOW_HEIGHT) - levelHeight);
    }

    float backgroundTileOffsetY() const
    {
        return document_.contains("Presets") && document_["Presets"].is_object()
            ? document_["Presets"].value("BackgroundTileOffsetY", 0.f)
            : 0.f;
    }

    sf::Vector2i backgroundTileMinIndex() const
    {
        return {
            0,
            static_cast<int>(std::floor((levelTopY() - backgroundTileOffsetY()) / kBackgroundTileSize.y))
        };
    }

    sf::Vector2i backgroundTileMaxIndex() const
    {
        const sf::Vector2f levelSize = readVector2f(
            document_["Presets"].value("Size", nlohmann::json::array()),
            {static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)}
        );
        const float levelBottom = levelTopY() + std::max(levelSize.y, 1.f);
        return {
            std::max(0, static_cast<int>(std::ceil(std::max(levelSize.x, 1.f) / kBackgroundTileSize.x)) - 1),
            static_cast<int>(std::ceil((levelBottom - backgroundTileOffsetY()) / kBackgroundTileSize.y)) - 1
        };
    }

    sf::Vector2i clampBackgroundTile(sf::Vector2i tileIndex) const
    {
        const sf::Vector2i minIndex = backgroundTileMinIndex();
        const sf::Vector2i maxIndex = backgroundTileMaxIndex();
        tileIndex.x = std::clamp(tileIndex.x, minIndex.x, maxIndex.x);
        tileIndex.y = std::clamp(tileIndex.y, minIndex.y, maxIndex.y);
        return tileIndex;
    }

    sf::Vector2i backgroundTileIndexForWorldPosition(const sf::Vector2f worldPosition) const
    {
        return clampBackgroundTile({
            static_cast<int>(std::floor(std::max(worldPosition.x, 0.f) / kBackgroundTileSize.x)),
            static_cast<int>(std::floor(worldPosition.y / kBackgroundTileSize.y))
        });
    }

    sf::Vector2f backgroundTileCenter(const sf::Vector2i tileIndex) const
    {
        const sf::Vector2i clampedTile = clampBackgroundTile(tileIndex);
        return {
            (static_cast<float>(clampedTile.x) + 0.5f) * kBackgroundTileSize.x,
            (static_cast<float>(clampedTile.y) + 0.5f) * kBackgroundTileSize.y
        };
    }

    sf::Vector2f backgroundTileDisplayCenter(const sf::Vector2i tileIndex) const
    {
        sf::Vector2f center = backgroundTileCenter(tileIndex);
        center.y += backgroundTileOffsetY();
        return center;
    }

    sf::Vector2i backgroundTileIndex(const nlohmann::json& background) const
    {
        return backgroundTileIndexForWorldPosition(readVector2f(background.value("Position", nlohmann::json::array()), {960.f, 540.f}));
    }

    void setBackgroundTile(nlohmann::json& background, sf::Vector2i tileIndex)
    {
        background["Position"] = toJson(backgroundTileCenter(tileIndex));
    }

    sf::Vector2f backgroundDisplayPosition(const nlohmann::json& background) const
    {
        return applyParallaxPreview(
            backgroundTileDisplayCenter(backgroundTileIndex(background)),
            backgroundPreviewParallax(background)
        );
    }

    bool hasSingleBackgroundInTile(const std::string& backgroundName, const sf::Vector2i tileIndex, const std::size_t ignoredIndex = std::numeric_limits<std::size_t>::max()) const
    {
        for (std::size_t index = 0; index < document_["Background"].size(); ++index)
        {
            if (index == ignoredIndex)
            {
                continue;
            }

            const auto& background = document_["Background"][index];
            if (!backgroundTypeIsSingle(background) || background.value("BgName", std::string{}) != backgroundName)
            {
                continue;
            }

            if (backgroundTileIndex(background) == tileIndex)
            {
                return true;
            }
        }

        return false;
    }

    std::optional<sf::Vector2i> firstFreeSingleBackgroundTile(
        const std::string& backgroundName,
        const std::size_t ignoredIndex = std::numeric_limits<std::size_t>::max()) const
    {
        const sf::Vector2i minTile = backgroundTileMinIndex();
        const sf::Vector2i maxTile = backgroundTileMaxIndex();
        for (int y = minTile.y; y <= maxTile.y; ++y)
        {
            for (int x = minTile.x; x <= maxTile.x; ++x)
            {
                const sf::Vector2i tile{x, y};
                if (!hasSingleBackgroundInTile(backgroundName, tile, ignoredIndex))
                {
                    return tile;
                }
            }
        }

        return std::nullopt;
    }

    bool resolveBackgroundTilePlacement(nlohmann::json& background, const std::size_t index)
    {
        const bool repeatX = backgroundTypeRepeatsX(background);
        const bool repeatY = backgroundTypeRepeatsY(background);
        if (!backgroundTypeIsSingle(background))
        {
            if (repeatX && repeatY)
            {
                setBackgroundTile(background, {0, 0});
                return true;
            }

            sf::Vector2i tile = backgroundTileIndex(background);
            if (repeatX)
            {
                tile.x = 0;
            }
            if (repeatY)
            {
                tile.y = 0;
            }
            setBackgroundTile(background, tile);
            return true;
        }

        const std::string backgroundName = background.value("BgName", std::string{});
        sf::Vector2i tile = backgroundTileIndex(background);
        if (hasSingleBackgroundInTile(backgroundName, tile, index))
        {
            const std::optional<sf::Vector2i> freeTile = firstFreeSingleBackgroundTile(backgroundName, index);
            if (!freeTile.has_value())
            {
                return false;
            }
            tile = *freeTile;
        }

        setBackgroundTile(background, tile);
        return true;
    }

    bool editBackgroundTileField(const char* label, nlohmann::json& background, const std::size_t index)
    {
        int raw[2]{backgroundTileIndex(background).x, backgroundTileIndex(background).y};
        const sf::Vector2i minTile = backgroundTileMinIndex();
        const sf::Vector2i maxTile = backgroundTileMaxIndex();
        ImGui::TextDisabled(
            "Background tiles are 1920 x 1080. Level grid: X %d..%d, Y %d..%d.",
            minTile.x,
            maxTile.x,
            minTile.y,
            maxTile.y
        );
        if (!backgroundTypeIsSingle(background))
        {
            const bool repeatX = backgroundTypeRepeatsX(background);
            const bool repeatY = backgroundTypeRepeatsY(background);
            if (repeatX && repeatY)
            {
                ImGui::TextUnformatted("Repeated XY covers the whole level grid.");
                const bool changed = backgroundTileIndex(background) != sf::Vector2i{0, 0};
                setBackgroundTile(background, {0, 0});
                return changed;
            }

            ImGui::TextUnformatted(repeatX
                ? "Repeated X uses Tile Y and fills the level horizontally."
                : "Repeated Y uses Tile X and fills the level vertically.");
            if (!ImGui::InputInt2(label, raw))
            {
                return false;
            }

            sf::Vector2i requestedTile = clampBackgroundTile({raw[0], raw[1]});
            if (repeatX)
            {
                requestedTile.x = 0;
            }
            if (repeatY)
            {
                requestedTile.y = 0;
            }
            const bool changed = requestedTile != backgroundTileIndex(background);
            setBackgroundTile(background, requestedTile);
            return changed;
        }

        if (!ImGui::InputInt2(label, raw))
        {
            return false;
        }

        sf::Vector2i requestedTile = clampBackgroundTile({raw[0], raw[1]});
        const std::string backgroundName = background.value("BgName", std::string{});
        if (hasSingleBackgroundInTile(backgroundName, requestedTile, index))
        {
            const std::optional<sf::Vector2i> freeTile = firstFreeSingleBackgroundTile(backgroundName, index);
            if (!freeTile.has_value())
            {
                return false;
            }
            requestedTile = *freeTile;
        }

        setBackgroundTile(background, requestedTile);
        return true;
    }

    sf::FloatRect backgroundBounds(const nlohmann::json& background) const
    {
        if (!backgroundTypeIsSingle(background))
        {
            const sf::Vector2f levelSize = readVector2f(
                document_["Presets"].value("Size", nlohmann::json::array()),
                {static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)}
            );
            const sf::Vector2i tile = backgroundTileIndex(background);
            const bool repeatX = backgroundTypeRepeatsX(background);
            const bool repeatY = backgroundTypeRepeatsY(background);
            const sf::Vector2f baseTopLeft{
                repeatX ? 0.f : static_cast<float>(tile.x) * kBackgroundTileSize.x,
                repeatY ? levelTopY() : static_cast<float>(tile.y) * kBackgroundTileSize.y + backgroundTileOffsetY()
            };
            const sf::Vector2f size{
                repeatX ? std::max(levelSize.x, kBackgroundTileSize.x) : kBackgroundTileSize.x,
                repeatY ? std::max(levelSize.y, kBackgroundTileSize.y) : kBackgroundTileSize.y
            };
            const sf::Vector2f cameraOffset = applyParallaxPreview(baseTopLeft, backgroundPreviewParallax(background));
            return sf::FloatRect(
                cameraOffset,
                size
            );
        }

        const sf::Vector2f center = backgroundDisplayPosition(background);
        return sf::FloatRect(
            {center.x - kBackgroundTileSize.x * 0.5f, center.y - kBackgroundTileSize.y * 0.5f},
            kBackgroundTileSize
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

    sf::FloatRect portalBounds(const nlohmann::json& portal) const
    {
        return textureBoundsAt(
            portal.value("PortalTexture", portal.value("Texture", std::string{"portalGreen"})),
            readVector2f(portal.value("Position", nlohmann::json::array())),
            readVector2f(portal.value("Scale", nlohmann::json::array()), {0.36f, 0.36f}),
            {96.f, 128.f}
        );
    }

    sf::FloatRect portalActivationBounds(const nlohmann::json& portal) const
    {
        if (portal.contains("ActivationArea") && portal["ActivationArea"].is_array())
        {
            return readRect(portal["ActivationArea"]);
        }
        const sf::Vector2f position = readVector2f(portal.value("Position", nlohmann::json::array()));
        const float radius = portal.value("InteractRadius", 130.f);
        return {{position.x - radius, position.y - radius}, {radius * 2.f, radius * 2.f}};
    }

    sf::FloatRect portalActivationHandleBounds(const nlohmann::json& portal) const
    {
        const sf::FloatRect bounds = portalActivationBounds(portal);
        return platformHandleBounds(bounds.position + bounds.size);
    }

    sf::FloatRect combinedPortalEditBounds(const nlohmann::json& portal) const
    {
        const sf::FloatRect sprite = portalBounds(portal);
        const sf::FloatRect activation = portalActivationBounds(portal);
        const float left = std::min(sprite.position.x, activation.position.x);
        const float top = std::min(sprite.position.y, activation.position.y);
        const float right = std::max(sprite.position.x + sprite.size.x, activation.position.x + activation.size.x);
        const float bottom = std::max(sprite.position.y + sprite.size.y, activation.position.y + activation.size.y);
        return {{left, top}, {std::max(right - left, 0.f), std::max(bottom - top, 0.f)}};
    }

    std::optional<std::size_t> findPortalAt(const sf::Vector2f worldPosition) const
    {
        for (std::size_t index = document_["Portals"].size(); index > 0u; --index)
        {
            const auto& portal = document_["Portals"][index - 1u];
            if (portalBounds(portal).contains(worldPosition) ||
                portalActivationBounds(portal).contains(worldPosition))
            {
                return index - 1u;
            }
        }

        return std::nullopt;
    }

    bool beginPortalInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None)
        {
            return false;
        }

        std::optional<std::size_t> portalIndex;
        if (selection_.kind == SelectionKind::Portal && selection_.index < document_["Portals"].size() &&
            (portalBounds(document_["Portals"][selection_.index]).contains(worldPosition) ||
             portalActivationBounds(document_["Portals"][selection_.index]).contains(worldPosition)))
        {
            portalIndex = selection_.index;
        }
        else
        {
            portalIndex = findPortalAt(worldPosition);
        }

        if (!portalIndex.has_value())
        {
            return false;
        }

        selection_ = {SelectionKind::Portal, *portalIndex};
        auto& portal = document_["Portals"][*portalIndex];
        if (portalActivationHandleBounds(portal).contains(worldPosition) ||
            resizeZoneContains(portalActivationBounds(portal), worldPosition))
        {
            portalInteraction_.mode = PortalInteractionMode::ResizeActivation;
        }
        else if (portalActivationBounds(portal).contains(worldPosition) && !portalBounds(portal).contains(worldPosition))
        {
            portalInteraction_.mode = PortalInteractionMode::MoveActivation;
        }
        else
        {
            portalInteraction_.mode = PortalInteractionMode::Move;
        }
        portalInteraction_.index = *portalIndex;
        portalInteraction_.startWorld = worldPosition;
        portalInteraction_.startPosition = readVector2f(portal.value("Position", nlohmann::json::array()));
        portalInteraction_.startActivationBounds = portalActivationBounds(portal);
        return true;
    }

    void updatePortalInteraction(const sf::Vector2f worldPosition)
    {
        if (!portalInteraction_.active() || portalInteraction_.index >= document_["Portals"].size())
        {
            portalInteraction_.clear();
            return;
        }

        auto& portal = document_["Portals"][portalInteraction_.index];
        const sf::Vector2f delta = worldPosition - portalInteraction_.startWorld;
        if (portalInteraction_.mode == PortalInteractionMode::Move)
        {
            portal["Position"] = toJson(portalInteraction_.startPosition + delta);
        }
        else
        {
            sf::FloatRect bounds = portalInteraction_.startActivationBounds;
            if (portalInteraction_.mode == PortalInteractionMode::MoveActivation)
            {
                bounds.position += delta;
            }
            else if (portalInteraction_.mode == PortalInteractionMode::ResizeActivation)
            {
                bounds.size = {std::max(bounds.size.x + delta.x, 24.f), std::max(bounds.size.y + delta.y, 24.f)};
            }
            portal["ActivationArea"] = toJson(bounds);
        }
        markDirty();
    }

    void finishPortalInteraction()
    {
        portalInteraction_.clear();
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

    sf::FloatRect spawnerActivationBounds(const nlohmann::json& spawner) const
    {
        if (spawner.contains("ActivationArea") && spawner["ActivationArea"].is_array())
        {
            return readRect(spawner["ActivationArea"]);
        }
        const sf::FloatRect spawn = spawnerBounds(spawner);
        const float padding = spawner.value("ActivationPadding", 120.f);
        return {{spawn.position.x - padding, spawn.position.y - padding}, {spawn.size.x + padding * 2.f, spawn.size.y + padding * 2.f}};
    }

    sf::FloatRect spawnerActivationResizeHandleBounds(const nlohmann::json& spawner) const
    {
        const sf::FloatRect bounds = spawnerActivationBounds(spawner);
        return platformHandleBounds(bounds.position + bounds.size);
    }

    sf::FloatRect combinedSpawnerEditBounds(const nlohmann::json& spawner) const
    {
        const sf::FloatRect spawn = spawnerBounds(spawner);
        const sf::FloatRect activation = spawnerActivationBounds(spawner);
        const float left = std::min(spawn.position.x, activation.position.x);
        const float top = std::min(spawn.position.y, activation.position.y);
        const float right = std::max(spawn.position.x + spawn.size.x, activation.position.x + activation.size.x);
        const float bottom = std::max(spawn.position.y + spawn.size.y, activation.position.y + activation.size.y);
        return {{left, top}, {std::max(right - left, 0.f), std::max(bottom - top, 0.f)}};
    }

    std::optional<std::size_t> findSpawnerAt(const sf::Vector2f worldPosition) const
    {
        for (std::size_t index = document_["Spawners"].size(); index > 0u; --index)
        {
            const auto& spawner = document_["Spawners"][index - 1u];
            if (spawnerBounds(spawner).contains(worldPosition) ||
                spawnerActivationBounds(spawner).contains(worldPosition))
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
            if (spawnerActivationResizeHandleBounds(*currentSpawner).contains(worldPosition) ||
                resizeZoneContains(spawnerActivationBounds(*currentSpawner), worldPosition))
            {
                spawnerInteraction_.mode = SpawnerInteractionMode::ResizeActivation;
                spawnerInteraction_.index = selection_.index;
            }
            else if (spawnerActivationBounds(*currentSpawner).contains(worldPosition) &&
                !spawnerBounds(*currentSpawner).contains(worldPosition))
            {
                spawnerInteraction_.mode = SpawnerInteractionMode::MoveActivation;
                spawnerInteraction_.index = selection_.index;
            }
            else if (spawnerResizeHandleBounds(*currentSpawner).contains(worldPosition) ||
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
            const auto& spawner = document_["Spawners"][*spawnerIndex];
            if (spawnerActivationResizeHandleBounds(spawner).contains(worldPosition) ||
                resizeZoneContains(spawnerActivationBounds(spawner), worldPosition))
            {
                spawnerInteraction_.mode = SpawnerInteractionMode::ResizeActivation;
            }
            else if (spawnerActivationBounds(spawner).contains(worldPosition) &&
                !spawnerBounds(spawner).contains(worldPosition))
            {
                spawnerInteraction_.mode = SpawnerInteractionMode::MoveActivation;
            }
            else if (spawnerResizeHandleBounds(spawner).contains(worldPosition) ||
                resizeZoneContains(spawnerBounds(spawner), worldPosition))
            {
                spawnerInteraction_.mode = SpawnerInteractionMode::Resize;
            }
            else
            {
                spawnerInteraction_.mode = SpawnerInteractionMode::Move;
            }
            spawnerInteraction_.index = *spawnerIndex;
        }

        spawnerInteraction_.startWorld = worldPosition;
        spawnerInteraction_.startBounds = spawnerBounds(document_["Spawners"][spawnerInteraction_.index]);
        spawnerInteraction_.startActivationBounds = spawnerActivationBounds(document_["Spawners"][spawnerInteraction_.index]);
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

        if (spawnerInteraction_.mode == SpawnerInteractionMode::Move || spawnerInteraction_.mode == SpawnerInteractionMode::Resize)
        {
            spawner["SpawnArea"] = nlohmann::json::array({
                nlohmann::json::array({bounds.position.x, bounds.position.x + bounds.size.x}),
                nlohmann::json::array({bounds.position.y, bounds.position.y + bounds.size.y})
            });
        }
        else
        {
            sf::FloatRect activation = spawnerInteraction_.startActivationBounds;
            if (spawnerInteraction_.mode == SpawnerInteractionMode::MoveActivation)
            {
                activation.position += delta;
            }
            else if (spawnerInteraction_.mode == SpawnerInteractionMode::ResizeActivation)
            {
                activation.size = {std::max(activation.size.x + delta.x, 24.f), std::max(activation.size.y + delta.y, 24.f)};
            }
            spawner["ActivationArea"] = toJson(activation);
            spawner["ActivationMode"] = "OnEnter";
        }
        markDirty();
    }

    void finishSpawnerInteraction()
    {
        spawnerInteraction_.clear();
    }

    bool drawSpawnerAdvancedControls(nlohmann::json& spawner, const char* idSuffix, const bool includeSpawnArea)
    {
        bool changed = false;
        const std::string suffix = std::string{"##"} + idSuffix;

        std::string enemyName = spawner.value("EnemyName", enemyTypeOptions_.empty() ? std::string{} : enemyTypeOptions_.front());
        if (comboFromStrings(("Enemy" + suffix).c_str(), enemyTypeOptions_, enemyName))
        {
            spawner["EnemyName"] = enemyName;
            changed = true;
        }

        int enemyAmount = spawner.value("EnemyAmount", 1);
        if (ImGui::InputInt(("Enemy Amount" + suffix).c_str(), &enemyAmount))
        {
            spawner["EnemyAmount"] = std::max(enemyAmount, 0);
            changed = true;
        }

        int spawnCooldown = spawner.value("SpawnCooldown", 5000);
        if (ImGui::InputInt(("Spawn Cooldown" + suffix).c_str(), &spawnCooldown))
        {
            spawner["SpawnCooldown"] = std::max(spawnCooldown, 0);
            changed = true;
        }

        int enemyPerSpawn = spawner.value("EnemyPerSpawn", 1);
        if (ImGui::InputInt(("Enemy Per Spawn" + suffix).c_str(), &enemyPerSpawn))
        {
            spawner["EnemyPerSpawn"] = std::max(enemyPerSpawn, 1);
            changed = true;
        }

        std::string activationMode = spawner.value("ActivationMode", std::string{"Immediate"});
        const std::vector<std::string> activationModes{"Immediate", "OnEnter"};
        if (comboFromStrings(("Activation" + suffix).c_str(), activationModes, activationMode))
        {
            spawner["ActivationMode"] = activationMode;
            changed = true;
        }

        if (activationMode == "OnEnter")
        {
            float activationPadding = spawner.value("ActivationPadding", 120.f);
            if (ImGui::InputFloat(("Activation Padding" + suffix).c_str(), &activationPadding, 8.f, 32.f, "%.1f"))
            {
                spawner["ActivationPadding"] = std::max(activationPadding, 0.f);
                changed = true;
            }
        }

        int firstSpawnDelayMs = spawner.value("FirstSpawnDelayMs", 0);
        if (ImGui::InputInt(("First Spawn Delay Ms" + suffix).c_str(), &firstSpawnDelayMs))
        {
            spawner["FirstSpawnDelayMs"] = std::max(firstSpawnDelayMs, 0);
            changed = true;
        }

        int enemyHp = spawner.value("EnemyHP", 0);
        if (ImGui::InputInt(("Enemy HP Override" + suffix).c_str(), &enemyHp))
        {
            spawner["EnemyHP"] = std::max(enemyHp, 0);
            changed = true;
        }

        int enemyDamage = spawner.value("EnemyDamage", 0);
        if (ImGui::InputInt(("Enemy Damage Override" + suffix).c_str(), &enemyDamage))
        {
            spawner["EnemyDamage"] = std::max(enemyDamage, 0);
            changed = true;
        }

        int goldReward = spawner.value("GoldReward", 0);
        if (ImGui::InputInt(("Gold Reward Override" + suffix).c_str(), &goldReward))
        {
            spawner["GoldReward"] = std::max(goldReward, 0);
            changed = true;
        }
        ImGui::TextDisabled("0 keeps the enemy default value.");

        std::string archetype = spawner.value("Archetype", std::string{"Default"});
        if (archetype.empty())
        {
            archetype = "Default";
        }
        const std::vector<std::string> archetypes{"Default", "Ambush", "Siege", "Swarm", "Duel"};
        if (comboFromStrings(("Archetype" + suffix).c_str(), archetypes, archetype))
        {
            if (archetype == "Default")
            {
                spawner.erase("Archetype");
            }
            else
            {
                spawner["Archetype"] = archetype;
            }
            changed = true;
        }

        if (includeSpawnArea)
        {
            sf::FloatRect bounds = spawnerBounds(spawner);
            float rectRaw[4]{bounds.position.x, bounds.position.y, bounds.size.x, bounds.size.y};
            if (ImGui::InputFloat4(("Spawn Area" + suffix).c_str(), rectRaw))
            {
                spawner["SpawnArea"] = nlohmann::json::array({
                    nlohmann::json::array({rectRaw[0], rectRaw[0] + std::max(rectRaw[2], 1.f)}),
                    nlohmann::json::array({rectRaw[1], rectRaw[1] + std::max(rectRaw[3], 1.f)})
                });
                changed = true;
            }

            sf::FloatRect activation = spawnerActivationBounds(spawner);
            float activationRaw[4]{activation.position.x, activation.position.y, activation.size.x, activation.size.y};
            if (ImGui::InputFloat4(("Activation Area" + suffix).c_str(), activationRaw))
            {
                spawner["ActivationArea"] = toJson(sf::FloatRect(
                    {activationRaw[0], activationRaw[1]},
                    {std::max(activationRaw[2], 1.f), std::max(activationRaw[3], 1.f)}
                ));
                spawner["ActivationMode"] = "OnEnter";
                changed = true;
            }
        }

        if (ImGui::TreeNode(("Encounter Notification" + suffix).c_str()))
        {
            changed |= editStringField(("Title" + suffix).c_str(), spawner, "EncounterTitle", 256u);
            changed |= editMultilineStringField(("Body" + suffix).c_str(), spawner, "EncounterBody", ImVec2(320.f, 84.f), 2048u);
            std::string tone = spawner.value("EncounterTone", std::string{"Warning"});
            const std::vector<std::string> tones{"Info", "Success", "Warning", "Danger"};
            if (comboFromStrings(("Tone" + suffix).c_str(), tones, tone))
            {
                spawner["EncounterTone"] = tone;
                changed = true;
            }
            bool fireOnce = spawner.value("EncounterFireOnce", true);
            if (ImGui::Checkbox(("Fire Once" + suffix).c_str(), &fireOnce))
            {
                spawner["EncounterFireOnce"] = fireOnce;
                changed = true;
            }
            ImGui::TreePop();
        }

        return changed;
    }

    sf::FloatRect miniLocationBounds(const nlohmann::json& location) const
    {
        return readRect(location.value("Bounds", nlohmann::json::array()));
    }

    sf::FloatRect miniLocationVisibleCameraRect(const nlohmann::json& location) const
    {
        const sf::FloatRect bounds = miniLocationBounds(location);
        const sf::Vector2f viewSize = runtimePreviewViewSize();
        const sf::Vector2f center = clampPreviewCameraCenterToBounds(bounds.getCenter(), bounds);
        return sf::FloatRect(center - viewSize * 0.5f, viewSize);
    }

    sf::FloatRect miniLocationResizeHandleBounds(const nlohmann::json& location) const
    {
        const sf::FloatRect bounds = miniLocationBounds(location);
        return platformHandleBounds(bounds.position + bounds.size);
    }

    sf::Vector2f miniLocationSpawnPositionAbsolute(const nlohmann::json& location) const
    {
        const sf::FloatRect bounds = miniLocationBounds(location);
        return bounds.position + readVector2f(
            location.value("SpawnPosition", nlohmann::json::array()),
            {bounds.size.x * 0.5f, bounds.size.y - 42.f}
        );
    }

    sf::FloatRect miniLocationSpawnMarkerBounds(const nlohmann::json& location) const
    {
        const sf::Vector2f spawn = miniLocationSpawnPositionAbsolute(location);
        return sf::FloatRect({spawn.x - 18.f, spawn.y - 18.f}, {36.f, 36.f});
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

    bool miniLocationOverlapsAnother(const sf::FloatRect& bounds, const std::size_t selfIndex) const
    {
        if (!document_.contains("MiniLocations") || !document_["MiniLocations"].is_array())
        {
            return false;
        }

        for (std::size_t index = 0; index < document_["MiniLocations"].size(); ++index)
        {
            if (index == selfIndex)
            {
                continue;
            }

            if (bounds.findIntersection(miniLocationBounds(document_["MiniLocations"][index])).has_value())
            {
                return true;
            }
        }

        return false;
    }

    std::vector<MiniLocationHit> hitTestMiniLocationContent(const std::size_t locationIndex, const sf::Vector2f worldPosition) const
    {
        std::vector<MiniLocationHit> hits;
        if (!document_.contains("MiniLocations") || !document_["MiniLocations"].is_array() ||
            locationIndex >= document_["MiniLocations"].size())
        {
            return hits;
        }

        const auto& location = document_["MiniLocations"][locationIndex];
        const sf::FloatRect bounds = miniLocationBounds(location);
        const auto drawOrderPriority = [&](const MiniLocationContentCollection collection, const std::size_t index) {
            if (!miniLocationCollectionUsesDrawOrder(collection))
            {
                return 0;
            }
            return miniLocationDrawOrder(location, collection, index) * 10 + miniLocationDrawOrderKindRank(collection);
        };

        const auto pushHit = [&](const MiniLocationContentCollection collection,
                                 const std::size_t index,
                                 const MiniLocationSubTarget subTarget,
                                 const sf::FloatRect hitBounds,
                                 const int priority) {
            if (hitBounds.contains(worldPosition))
            {
                hits.push_back({{locationIndex, collection, index, subTarget}, hitBounds, priority});
            }
        };

        pushHit(MiniLocationContentCollection::SpawnPosition, 0u, MiniLocationSubTarget::Body,
            miniLocationSpawnMarkerBounds(location), 9000);

        if (location.contains("Entry") && location["Entry"].is_object())
        {
            const auto& entry = location["Entry"];
            const sf::Vector2f position = readVector2f(entry.value("Position", nlohmann::json::array()));
            const sf::Vector2f scale = readVector2f(entry.value("Scale", nlohmann::json::array()), {0.22f, 0.33f});
            const std::string texture = entry.value("Texture", std::string{});
            pushHit(MiniLocationContentCollection::Entry, 0u, MiniLocationSubTarget::Body,
                textureBoundsAt(texture, position, scale, {72.f, 96.f}), 8600);
        }
        if (location.contains("Exit") && location["Exit"].is_object())
        {
            const auto& exit = location["Exit"];
            const sf::Vector2f position = readVector2f(exit.value("Position", nlohmann::json::array()));
            const sf::Vector2f scale = readVector2f(exit.value("Scale", nlohmann::json::array()), {0.22f, 0.33f});
            const std::string texture = exit.value("Texture", std::string{});
            pushHit(MiniLocationContentCollection::Exit, 0u, MiniLocationSubTarget::Body,
                textureBoundsAt(texture, position, scale, {72.f, 96.f}), 8500);
        }

        if (location.contains("DeadAreas") && location["DeadAreas"].is_array())
        {
            for (std::size_t index = 0; index < location["DeadAreas"].size(); ++index)
            {
                const auto& deadArea = location["DeadAreas"][index];
                pushHit(MiniLocationContentCollection::DeadAreas, index, MiniLocationSubTarget::ResizeHandle,
                    deadAreaResizeHandleBounds(deadArea, location), 10000 + static_cast<int>(index));
                pushHit(MiniLocationContentCollection::DeadAreas, index, MiniLocationSubTarget::Body,
                    deadAreaBounds(deadArea, location), 300000 + static_cast<int>(index));
            }
        }

        if (location.contains("Barriers") && location["Barriers"].is_array())
        {
            for (std::size_t index = 0; index < location["Barriers"].size(); ++index)
            {
                const auto& barrier = location["Barriers"][index];
                pushHit(MiniLocationContentCollection::Barriers, index, MiniLocationSubTarget::ResizeHandle,
                    deadAreaResizeHandleBounds(barrier, location), 10050 + static_cast<int>(index));
                pushHit(MiniLocationContentCollection::Barriers, index, MiniLocationSubTarget::Body,
                    deadAreaBounds(barrier, location), 300500 + static_cast<int>(index));
            }
        }

        if (location.contains("Decorations") && location["Decorations"].is_array())
        {
            for (std::size_t index = 0; index < location["Decorations"].size(); ++index)
            {
                const sf::FloatRect bounds = nestedDecorationBounds(location["Decorations"][index], location);
                pushHit(MiniLocationContentCollection::Decorations, index, MiniLocationSubTarget::ResizeHandle,
                    platformHandleBounds(bounds.position + bounds.size), 10020 + static_cast<int>(index));
                pushHit(MiniLocationContentCollection::Decorations, index, MiniLocationSubTarget::Body,
                    bounds, 100000 + drawOrderPriority(MiniLocationContentCollection::Decorations, index));
            }
        }

        if (location.contains("Platforms") && location["Platforms"].is_array())
        {
            for (std::size_t index = 0; index < location["Platforms"].size(); ++index)
            {
                const nlohmann::json absolutePlatform = absoluteNestedObject(location["Platforms"][index], location);
                const sf::FloatRect platformRect = nestedPlatformHitBounds(location["Platforms"][index], location);
                if (platformContainsPoint(absolutePlatform, worldPosition) || platformRect.contains(worldPosition))
                {
                    hits.push_back({{locationIndex, MiniLocationContentCollection::Platforms, index, MiniLocationSubTarget::Body},
                        platformRect, 100000 + drawOrderPriority(MiniLocationContentCollection::Platforms, index)});
                }
            }
        }

        if (location.contains("Interactives") && location["Interactives"].is_array())
        {
            for (std::size_t index = 0; index < location["Interactives"].size(); ++index)
            {
                const sf::FloatRect bounds = interactiveBounds(absoluteNestedObject(location["Interactives"][index], location));
                pushHit(MiniLocationContentCollection::Interactives, index, MiniLocationSubTarget::ResizeHandle,
                    platformHandleBounds(bounds.position + bounds.size), 10030 + static_cast<int>(index));
                pushHit(MiniLocationContentCollection::Interactives, index, MiniLocationSubTarget::Body,
                    bounds, 100000 + drawOrderPriority(MiniLocationContentCollection::Interactives, index));
            }
        }

        if (location.contains("Portals") && location["Portals"].is_array())
        {
            for (std::size_t index = 0; index < location["Portals"].size(); ++index)
            {
                const sf::FloatRect bounds = portalBounds(absoluteNestedObject(location["Portals"][index], location));
                pushHit(MiniLocationContentCollection::Portals, index, MiniLocationSubTarget::ResizeHandle,
                    platformHandleBounds(bounds.position + bounds.size), 10040 + static_cast<int>(index));
                pushHit(MiniLocationContentCollection::Portals, index, MiniLocationSubTarget::Body,
                    bounds, 100000 + drawOrderPriority(MiniLocationContentCollection::Portals, index));
            }
        }

        std::stable_sort(hits.begin(), hits.end(), [](const MiniLocationHit& lhs, const MiniLocationHit& rhs) {
            return lhs.priority > rhs.priority;
        });
        (void)bounds;
        return hits;
    }

    void selectMiniLocationContentAt(const sf::Vector2f worldPosition)
    {
        if (selection_.kind != SelectionKind::MiniLocation || selection_.index >= document_["MiniLocations"].size())
        {
            miniEditor_.clearSelection();
            return;
        }

        auto& location = document_["MiniLocations"][selection_.index];
        ensureMiniLocationNestedArrays(location);
        std::vector<MiniLocationHit> hits = hitTestMiniLocationContent(selection_.index, worldPosition);
        if (hits.empty())
        {
            miniEditor_.clearSelection();
            return;
        }

        const bool cycleRequested = ImGui::GetIO().KeyCtrl;
        const float dx = worldPosition.x - miniEditor_.lastHitWorld.x;
        const float dy = worldPosition.y - miniEditor_.lastHitWorld.y;
        const bool sameSpot = (dx * dx + dy * dy) <= 64.f;
        const bool sameHitStack =
            hits.size() == miniEditor_.lastHits.size() &&
            std::equal(hits.begin(), hits.end(), miniEditor_.lastHits.begin(), [](const MiniLocationHit& lhs, const MiniLocationHit& rhs) {
                return lhs.ref.sameObject(rhs.ref) && lhs.ref.subTarget == rhs.ref.subTarget;
            });
        if (!cycleRequested && miniEditor_.selected.isValid() && miniEditor_.selected.locationIndex == selection_.index)
        {
            const auto selectedHit = std::find_if(hits.begin(), hits.end(), [&](const MiniLocationHit& hit) {
                return hit.ref.sameObject(miniEditor_.selected);
            });
            if (selectedHit != hits.end())
            {
                miniEditor_.selected = selectedHit->ref;
                miniEditor_.lastHitWorld = worldPosition;
                miniEditor_.lastHits = hits;
                miniEditor_.cycleIndex = static_cast<std::size_t>(std::distance(hits.begin(), selectedHit));
                return;
            }
        }

        if (cycleRequested && sameSpot && sameHitStack)
        {
            miniEditor_.cycleIndex = (miniEditor_.cycleIndex + 1u) % hits.size();
        }
        else
        {
            miniEditor_.cycleIndex = 0u;
        }
        miniEditor_.lastHitWorld = worldPosition;
        miniEditor_.lastHits = hits;
        miniEditor_.selected = hits[miniEditor_.cycleIndex].ref;
    }

    bool beginMiniLocationInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None)
        {
            return false;
        }

        if (selection_.kind == SelectionKind::MiniLocation && selection_.index < document_["MiniLocations"].size())
        {
            nlohmann::json& currentLocation = document_["MiniLocations"][selection_.index];
            ensureMiniLocationNestedArrays(currentLocation);

            if (miniEditor_.contentMode)
            {
                selectMiniLocationContentAt(worldPosition);
                const MiniLocationObjectRef selectedRef = miniEditor_.selected;
                if (!selectedRef.isValid())
                {
                    return true;
                }

                if (selectedRef.collection == MiniLocationContentCollection::SpawnPosition)
                {
                    miniLocationInteraction_.mode = MiniLocationInteractionMode::MoveSpawn;
                    miniLocationInteraction_.index = selection_.index;
                }
                else if (selectedRef.collection == MiniLocationContentCollection::Entry)
                {
                    miniLocationInteraction_.mode = MiniLocationInteractionMode::MoveEntry;
                    miniLocationInteraction_.index = selection_.index;
                }
                else if (selectedRef.collection == MiniLocationContentCollection::Exit)
                {
                    miniLocationInteraction_.mode = MiniLocationInteractionMode::MoveExit;
                    miniLocationInteraction_.index = selection_.index;
                }
                else if (selectedRef.collection == MiniLocationContentCollection::Platforms)
                {
                    miniLocationInteraction_.mode = MiniLocationInteractionMode::MoveNestedPlatform;
                    miniLocationInteraction_.index = selection_.index;
                    miniLocationInteraction_.nestedIndex = selectedRef.index;
                }
                else if (selectedRef.collection == MiniLocationContentCollection::Decorations)
                {
                    miniLocationInteraction_.mode = selectedRef.subTarget == MiniLocationSubTarget::ResizeHandle
                        ? MiniLocationInteractionMode::ScaleNestedDecoration
                        : MiniLocationInteractionMode::MoveNestedDecoration;
                    miniLocationInteraction_.index = selection_.index;
                    miniLocationInteraction_.nestedIndex = selectedRef.index;
                }
                else if (selectedRef.collection == MiniLocationContentCollection::Interactives)
                {
                    miniLocationInteraction_.mode = selectedRef.subTarget == MiniLocationSubTarget::ResizeHandle
                        ? MiniLocationInteractionMode::ScaleNestedInteractive
                        : MiniLocationInteractionMode::MoveNestedInteractive;
                    miniLocationInteraction_.index = selection_.index;
                    miniLocationInteraction_.nestedIndex = selectedRef.index;
                }
                else if (selectedRef.collection == MiniLocationContentCollection::Portals)
                {
                    miniLocationInteraction_.mode = selectedRef.subTarget == MiniLocationSubTarget::ResizeHandle
                        ? MiniLocationInteractionMode::ScaleNestedPortal
                        : MiniLocationInteractionMode::MoveNestedPortal;
                    miniLocationInteraction_.index = selection_.index;
                    miniLocationInteraction_.nestedIndex = selectedRef.index;
                }
                else if (selectedRef.collection == MiniLocationContentCollection::DeadAreas)
                {
                    miniLocationInteraction_.mode = selectedRef.subTarget == MiniLocationSubTarget::ResizeHandle
                        ? MiniLocationInteractionMode::ResizeDeadArea
                        : MiniLocationInteractionMode::MoveDeadArea;
                    miniLocationInteraction_.index = selection_.index;
                    miniLocationInteraction_.nestedIndex = selectedRef.index;
                }
                else if (selectedRef.collection == MiniLocationContentCollection::Barriers)
                {
                    miniLocationInteraction_.mode = selectedRef.subTarget == MiniLocationSubTarget::ResizeHandle
                        ? MiniLocationInteractionMode::ResizeBarrier
                        : MiniLocationInteractionMode::MoveBarrier;
                    miniLocationInteraction_.index = selection_.index;
                    miniLocationInteraction_.nestedIndex = selectedRef.index;
                }
                if (miniLocationInteraction_.active())
                {
                    // Start fields are filled below.
                }
                else
                {
                    return true;
                }
            }
            else if (miniLocationResizeHandleBounds(currentLocation).contains(worldPosition) ||
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
        miniLocationInteraction_.startLocation = document_["MiniLocations"][miniLocationInteraction_.index];
        miniLocationInteraction_.startBounds = miniLocationBounds(miniLocationInteraction_.startLocation);
        miniLocationInteraction_.startSpawnPosition = readVector2f(
            miniLocationInteraction_.startLocation.value("SpawnPosition", nlohmann::json::array()),
            {miniLocationInteraction_.startBounds.size.x * 0.5f, miniLocationInteraction_.startBounds.size.y - 42.f}
        );
        auto& startLocation = miniLocationInteraction_.startLocation;
        if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveEntry && startLocation.contains("Entry") && startLocation["Entry"].is_object())
        {
            miniLocationInteraction_.startNestedPosition = readVector2f(startLocation["Entry"].value("Position", nlohmann::json::array()));
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveExit && startLocation.contains("Exit") && startLocation["Exit"].is_object())
        {
            miniLocationInteraction_.startNestedPosition = readVector2f(startLocation["Exit"].value("Position", nlohmann::json::array()));
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveNestedPlatform && miniLocationInteraction_.nestedIndex < startLocation["Platforms"].size())
        {
            miniLocationInteraction_.startNestedPosition = readVector2f(startLocation["Platforms"][miniLocationInteraction_.nestedIndex].value("Position", nlohmann::json::array()));
        }
        else if ((miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveNestedDecoration ||
            miniLocationInteraction_.mode == MiniLocationInteractionMode::ScaleNestedDecoration) &&
            miniLocationInteraction_.nestedIndex < startLocation["Decorations"].size())
        {
            miniLocationInteraction_.startNestedPosition = readVector2f(startLocation["Decorations"][miniLocationInteraction_.nestedIndex].value("Position", nlohmann::json::array()));
            miniLocationInteraction_.startNestedScale = readVector2f(startLocation["Decorations"][miniLocationInteraction_.nestedIndex].value("Scale", nlohmann::json::array()), {1.f, 1.f});
        }
        else if ((miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveNestedInteractive ||
            miniLocationInteraction_.mode == MiniLocationInteractionMode::ScaleNestedInteractive) &&
            miniLocationInteraction_.nestedIndex < startLocation["Interactives"].size())
        {
            miniLocationInteraction_.startNestedPosition = readVector2f(startLocation["Interactives"][miniLocationInteraction_.nestedIndex].value("Position", nlohmann::json::array()));
            miniLocationInteraction_.startNestedScale = readVector2f(startLocation["Interactives"][miniLocationInteraction_.nestedIndex].value("Scale", nlohmann::json::array()), {1.f, 1.f});
        }
        else if ((miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveNestedPortal ||
            miniLocationInteraction_.mode == MiniLocationInteractionMode::ScaleNestedPortal) &&
            miniLocationInteraction_.nestedIndex < startLocation["Portals"].size())
        {
            miniLocationInteraction_.startNestedPosition = readVector2f(startLocation["Portals"][miniLocationInteraction_.nestedIndex].value("Position", nlohmann::json::array()));
            miniLocationInteraction_.startNestedScale = readVector2f(startLocation["Portals"][miniLocationInteraction_.nestedIndex].value("Scale", nlohmann::json::array()), {0.36f, 0.36f});
        }
        else if ((miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveDeadArea ||
            miniLocationInteraction_.mode == MiniLocationInteractionMode::ResizeDeadArea) &&
            miniLocationInteraction_.nestedIndex < startLocation["DeadAreas"].size())
        {
            miniLocationInteraction_.startDeadAreaRect = readRect(startLocation["DeadAreas"][miniLocationInteraction_.nestedIndex].value("Rect", nlohmann::json::array()));
        }
        else if ((miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveBarrier ||
            miniLocationInteraction_.mode == MiniLocationInteractionMode::ResizeBarrier) &&
            miniLocationInteraction_.nestedIndex < startLocation["Barriers"].size())
        {
            miniLocationInteraction_.startDeadAreaRect = readRect(startLocation["Barriers"][miniLocationInteraction_.nestedIndex].value("Rect", nlohmann::json::array()));
        }
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
        const nlohmann::json& startLocation = miniLocationInteraction_.startLocation.is_object()
            ? miniLocationInteraction_.startLocation
            : location;
        const sf::Vector2f delta = worldPosition - miniLocationInteraction_.startWorld;

        if (miniLocationInteraction_.mode == MiniLocationInteractionMode::Move)
        {
            const sf::FloatRect movedBounds{
                miniLocationInteraction_.startBounds.position + delta,
                miniLocationInteraction_.startBounds.size
            };
            if (miniLocationOverlapsAnother(movedBounds, miniLocationInteraction_.index))
            {
                return;
            }

            location["Bounds"] = toJson(movedBounds);

            auto offsetVectorField = [&](const nlohmann::json& source, nlohmann::json& destination, const char* key) {
                destination[key] = toJson(readVector2f(source.value(key, nlohmann::json::array())) + delta);
            };

            if (startLocation.contains("Entry") && startLocation["Entry"].is_object() &&
                location.contains("Entry") && location["Entry"].is_object())
            {
                offsetVectorField(startLocation["Entry"], location["Entry"], "Position");
                offsetVectorField(startLocation["Entry"], location["Entry"], "DestinationSupport");
            }

            if (startLocation.contains("Exit") && startLocation["Exit"].is_object() &&
                location.contains("Exit") && location["Exit"].is_object())
            {
                offsetVectorField(startLocation["Exit"], location["Exit"], "Position");
                offsetVectorField(startLocation["Exit"], location["Exit"], "DestinationSupport");
            }

            if (startLocation.contains("Hazard") && startLocation["Hazard"].is_object() &&
                location.contains("Hazard") && location["Hazard"].is_object())
            {
                sf::FloatRect hazardRect = readRect(startLocation["Hazard"].value("Rect", nlohmann::json::array()));
                hazardRect.position += delta;
                location["Hazard"]["Rect"] = toJson(hazardRect);
            }
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveSpawn)
        {
            const sf::Vector2f relative = miniLocationInteraction_.startSpawnPosition + delta;
            location["SpawnPosition"] = toJson(sf::Vector2f{
                std::clamp(relative.x, 0.f, miniLocationInteraction_.startBounds.size.x),
                std::clamp(relative.y, 0.f, miniLocationInteraction_.startBounds.size.y)
            });
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveEntry &&
            location.contains("Entry") && location["Entry"].is_object())
        {
            location["Entry"]["Position"] = toJson(miniLocationInteraction_.startNestedPosition + delta);
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveExit &&
            location.contains("Exit") && location["Exit"].is_object())
        {
            location["Exit"]["Position"] = toJson(miniLocationInteraction_.startNestedPosition + delta);
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveNestedPlatform &&
            miniLocationInteraction_.nestedIndex < location["Platforms"].size())
        {
            location["Platforms"][miniLocationInteraction_.nestedIndex]["Position"] = toJson(miniLocationInteraction_.startNestedPosition + delta);
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveNestedDecoration &&
            miniLocationInteraction_.nestedIndex < location["Decorations"].size())
        {
            location["Decorations"][miniLocationInteraction_.nestedIndex]["Position"] = toJson(miniLocationInteraction_.startNestedPosition + delta);
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveNestedInteractive &&
            miniLocationInteraction_.nestedIndex < location["Interactives"].size())
        {
            location["Interactives"][miniLocationInteraction_.nestedIndex]["Position"] = toJson(miniLocationInteraction_.startNestedPosition + delta);
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveNestedPortal &&
            miniLocationInteraction_.nestedIndex < location["Portals"].size())
        {
            location["Portals"][miniLocationInteraction_.nestedIndex]["Position"] = toJson(miniLocationInteraction_.startNestedPosition + delta);
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::ScaleNestedDecoration &&
            miniLocationInteraction_.nestedIndex < location["Decorations"].size())
        {
            location["Decorations"][miniLocationInteraction_.nestedIndex]["Scale"] = toJson(sf::Vector2f{
                std::max(0.02f, miniLocationInteraction_.startNestedScale.x + delta.x * 0.005f),
                std::max(0.02f, miniLocationInteraction_.startNestedScale.y + delta.y * 0.005f)
            });
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::ScaleNestedInteractive &&
            miniLocationInteraction_.nestedIndex < location["Interactives"].size())
        {
            location["Interactives"][miniLocationInteraction_.nestedIndex]["Scale"] = toJson(sf::Vector2f{
                std::max(0.02f, miniLocationInteraction_.startNestedScale.x + delta.x * 0.005f),
                std::max(0.02f, miniLocationInteraction_.startNestedScale.y + delta.y * 0.005f)
            });
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::ScaleNestedPortal &&
            miniLocationInteraction_.nestedIndex < location["Portals"].size())
        {
            location["Portals"][miniLocationInteraction_.nestedIndex]["Scale"] = toJson(sf::Vector2f{
                std::max(0.02f, miniLocationInteraction_.startNestedScale.x + delta.x * 0.003f),
                std::max(0.02f, miniLocationInteraction_.startNestedScale.y + delta.y * 0.003f)
            });
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveDeadArea &&
            miniLocationInteraction_.nestedIndex < location["DeadAreas"].size())
        {
            sf::FloatRect rect = miniLocationInteraction_.startDeadAreaRect;
            rect.position += delta;
            location["DeadAreas"][miniLocationInteraction_.nestedIndex]["Rect"] = toJson(rect);
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::ResizeDeadArea &&
            miniLocationInteraction_.nestedIndex < location["DeadAreas"].size())
        {
            sf::FloatRect rect = miniLocationInteraction_.startDeadAreaRect;
            rect.size = {
                std::max(rect.size.x + delta.x, 8.f),
                std::max(rect.size.y + delta.y, 8.f)
            };
            location["DeadAreas"][miniLocationInteraction_.nestedIndex]["Rect"] = toJson(rect);
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::MoveBarrier &&
            miniLocationInteraction_.nestedIndex < location["Barriers"].size())
        {
            sf::FloatRect rect = miniLocationInteraction_.startDeadAreaRect;
            rect.position += delta;
            location["Barriers"][miniLocationInteraction_.nestedIndex]["Rect"] = toJson(rect);
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::ResizeBarrier &&
            miniLocationInteraction_.nestedIndex < location["Barriers"].size())
        {
            sf::FloatRect rect = miniLocationInteraction_.startDeadAreaRect;
            rect.size = {
                std::max(rect.size.x + delta.x, 8.f),
                std::max(rect.size.y + delta.y, 8.f)
            };
            location["Barriers"][miniLocationInteraction_.nestedIndex]["Rect"] = toJson(rect);
        }
        else if (miniLocationInteraction_.mode == MiniLocationInteractionMode::Resize)
        {
            sf::FloatRect bounds = miniLocationInteraction_.startBounds;
            bounds.size = {
                std::max(bounds.size.x + delta.x, 64.f),
                std::max(bounds.size.y + delta.y, 64.f)
            };
            if (miniLocationOverlapsAnother(bounds, miniLocationInteraction_.index))
            {
                return;
            }

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

    std::optional<std::size_t> findWorldHazardAt(const SelectionKind kind, const sf::Vector2f worldPosition) const
    {
        const char* arrayName = kind == SelectionKind::Barrier ? "Barriers" : "DeadAreas";
        if (!document_.contains(arrayName) || !document_[arrayName].is_array())
        {
            return std::nullopt;
        }

        const auto& array = document_[arrayName];
        for (std::size_t index = array.size(); index > 0u; --index)
        {
            if (worldHazardBounds(array[index - 1u]).contains(worldPosition))
            {
                return index - 1u;
            }
        }
        return std::nullopt;
    }

    bool beginHazardInteraction(const sf::Vector2f worldPosition)
    {
        if (placementMode_ != SelectionKind::None)
        {
            return false;
        }

        const auto startForSelection = [&](const SelectionKind kind, const std::size_t index) -> bool {
            const char* arrayName = kind == SelectionKind::Barrier ? "Barriers" : "DeadAreas";
            if (!document_.contains(arrayName) || !document_[arrayName].is_array() || index >= document_[arrayName].size())
            {
                return false;
            }

            const nlohmann::json& hazard = document_[arrayName][index];
            const sf::FloatRect bounds = worldHazardBounds(hazard);
            if (worldHazardResizeHandleBounds(hazard).contains(worldPosition) || resizeZoneContains(bounds, worldPosition))
            {
                hazardInteraction_.mode = HazardInteractionMode::Resize;
            }
            else if (bounds.contains(worldPosition))
            {
                hazardInteraction_.mode = HazardInteractionMode::Move;
            }
            else
            {
                return false;
            }

            hazardInteraction_.kind = kind;
            hazardInteraction_.index = index;
            hazardInteraction_.startWorld = worldPosition;
            hazardInteraction_.startBounds = bounds;
            selection_ = {kind, index};
            return true;
        };

        if ((selection_.kind == SelectionKind::DeadArea || selection_.kind == SelectionKind::Barrier) &&
            startForSelection(selection_.kind, selection_.index))
        {
            return true;
        }

        for (const SelectionKind kind : {SelectionKind::Barrier, SelectionKind::DeadArea})
        {
            const std::optional<std::size_t> index = findWorldHazardAt(kind, worldPosition);
            if (index.has_value() && startForSelection(kind, *index))
            {
                return true;
            }
        }

        return false;
    }

    void updateHazardInteraction(const sf::Vector2f worldPosition)
    {
        if (!hazardInteraction_.active())
        {
            return;
        }

        const char* arrayName = hazardInteraction_.kind == SelectionKind::Barrier ? "Barriers" : "DeadAreas";
        if (!document_.contains(arrayName) || !document_[arrayName].is_array() || hazardInteraction_.index >= document_[arrayName].size())
        {
            hazardInteraction_.clear();
            return;
        }

        sf::FloatRect rect = hazardInteraction_.startBounds;
        const sf::Vector2f delta = worldPosition - hazardInteraction_.startWorld;
        if (hazardInteraction_.mode == HazardInteractionMode::Move)
        {
            rect.position += delta;
        }
        else if (hazardInteraction_.mode == HazardInteractionMode::Resize)
        {
            rect.size.x = std::max(16.f, hazardInteraction_.startBounds.size.x + delta.x);
            rect.size.y = std::max(16.f, hazardInteraction_.startBounds.size.y + delta.y);
        }

        document_[arrayName][hazardInteraction_.index]["Rect"] = toJson(rect);
        markDirty();
    }

    void finishHazardInteraction()
    {
        hazardInteraction_.clear();
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
        case SelectionKind::Trader:
            return isTraderEditable() && traderMarkerBounds().contains(worldPosition);
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
                (spawnerBounds(document_["Spawners"][selection.index]).contains(worldPosition) ||
                 spawnerActivationBounds(document_["Spawners"][selection.index]).contains(worldPosition));
        case SelectionKind::Portal:
            return selection.index < document_["Portals"].size() &&
                (portalBounds(document_["Portals"][selection.index]).contains(worldPosition) ||
                 portalActivationBounds(document_["Portals"][selection.index]).contains(worldPosition));
        case SelectionKind::Interactive:
            return selection.index < document_["Interactives"].size() &&
                interactiveBounds(document_["Interactives"][selection.index]).contains(worldPosition);
        case SelectionKind::MiniLocation:
            return selection.index < document_["MiniLocations"].size() &&
                miniLocationBounds(document_["MiniLocations"][selection.index]).contains(worldPosition);
        case SelectionKind::DeadArea:
            return selection.index < document_["DeadAreas"].size() &&
                worldHazardBounds(document_["DeadAreas"][selection.index]).contains(worldPosition);
        case SelectionKind::Barrier:
            return selection.index < document_["Barriers"].size() &&
                worldHazardBounds(document_["Barriers"][selection.index]).contains(worldPosition);
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
                 spawnerActivationResizeHandleBounds(document_["Spawners"][selection.index]).contains(worldPosition) ||
                 resizeZoneContains(spawnerBounds(document_["Spawners"][selection.index]), worldPosition) ||
                 resizeZoneContains(spawnerActivationBounds(document_["Spawners"][selection.index]), worldPosition));
        case SelectionKind::Portal:
            return selection.index < document_["Portals"].size() &&
                (portalActivationHandleBounds(document_["Portals"][selection.index]).contains(worldPosition) ||
                 resizeZoneContains(portalActivationBounds(document_["Portals"][selection.index]), worldPosition));
        case SelectionKind::Ground:
            return selection.index < document_["Ground"].size() &&
                (groundResizeHandleBounds(document_["Ground"][selection.index]).contains(worldPosition) ||
                 groundResizeZoneContains(groundBounds(document_["Ground"][selection.index]), worldPosition));
        case SelectionKind::MiniLocation:
            return selection.index < document_["MiniLocations"].size() &&
                (miniLocationResizeHandleBounds(document_["MiniLocations"][selection.index]).contains(worldPosition) ||
                 resizeZoneContains(miniLocationBounds(document_["MiniLocations"][selection.index]), worldPosition));
        case SelectionKind::DeadArea:
            return selection.index < document_["DeadAreas"].size() &&
                (worldHazardResizeHandleBounds(document_["DeadAreas"][selection.index]).contains(worldPosition) ||
                 resizeZoneContains(worldHazardBounds(document_["DeadAreas"][selection.index]), worldPosition));
        case SelectionKind::Barrier:
            return selection.index < document_["Barriers"].size() &&
                (worldHazardResizeHandleBounds(document_["Barriers"][selection.index]).contains(worldPosition) ||
                 resizeZoneContains(worldHazardBounds(document_["Barriers"][selection.index]), worldPosition));
        default:
            return false;
        }
    }

    bool beginInteractionForSelection(const EditorSelection& selection, const sf::Vector2f worldPosition)
    {
        const EditorSelection previousSelection = selection_;
        selection_ = selection;

        switch (selection.kind)
        {
        case SelectionKind::Spawn:
            return beginSpawnInteraction(worldPosition);
        case SelectionKind::Trader:
            return beginTraderInteraction(worldPosition);
        case SelectionKind::Interactive:
            return beginInteractiveInteraction(worldPosition);
        case SelectionKind::Spawner:
            return beginSpawnerInteraction(worldPosition);
        case SelectionKind::Portal:
            return beginPortalInteraction(worldPosition);
        case SelectionKind::Platform:
            return beginPlatformInteraction(worldPosition);
        case SelectionKind::Decoration:
            return beginDecorationInteraction(worldPosition);
        case SelectionKind::MiniLocation:
            return beginMiniLocationInteraction(worldPosition);
        case SelectionKind::Ground:
            return beginGroundInteraction(worldPosition);
        case SelectionKind::DeadArea:
        case SelectionKind::Barrier:
            return beginHazardInteraction(worldPosition);
        default:
            selection_ = previousSelection;
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
            document_["Portals"].size() +
            document_["Interactives"].size() +
            document_["DeadAreas"].size() +
            document_["Barriers"].size() +
            2u
        );

        const sf::Vector2f spawn = readVector2f(document_["Presets"].value("PlayerSpawn", nlohmann::json::array()), {0.f, 0.f});
        if (sf::FloatRect({spawn.x - 24.f, spawn.y - 48.f}, {48.f, 48.f}).contains(worldPosition))
        {
            candidates.push_back({SelectionKind::Spawn, 0u});
        }

        if (isTraderEditable() && traderMarkerBounds().contains(worldPosition))
        {
            candidates.push_back({SelectionKind::Trader, 0u});
        }

        for (std::size_t index = document_["Barriers"].size(); index > 0u; --index)
        {
            if (worldHazardBounds(document_["Barriers"][index - 1u]).contains(worldPosition))
            {
                candidates.push_back({SelectionKind::Barrier, index - 1u});
            }
        }

        for (std::size_t index = document_["DeadAreas"].size(); index > 0u; --index)
        {
            if (worldHazardBounds(document_["DeadAreas"][index - 1u]).contains(worldPosition))
            {
                candidates.push_back({SelectionKind::DeadArea, index - 1u});
            }
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
            if (spawnerBounds(document_["Spawners"][index - 1u]).contains(worldPosition) ||
                spawnerActivationBounds(document_["Spawners"][index - 1u]).contains(worldPosition))
            {
                candidates.push_back({SelectionKind::Spawner, index - 1u});
            }
        }

        for (std::size_t index = document_["Portals"].size(); index > 0u; --index)
        {
            if (portalBounds(document_["Portals"][index - 1u]).contains(worldPosition) ||
                portalActivationBounds(document_["Portals"][index - 1u]).contains(worldPosition))
            {
                candidates.push_back({SelectionKind::Portal, index - 1u});
            }
        }

        const std::vector<EditorSelection> sharedSelections = sharedWorldDrawOrderSelections();
        for (auto it = sharedSelections.rbegin(); it != sharedSelections.rend(); ++it)
        {
            if (it->kind == SelectionKind::Platform)
            {
                if (platformContainsPoint(document_["Platforms"][it->index], worldPosition))
                {
                    candidates.push_back(*it);
                }
            }
            else if (it->kind == SelectionKind::Decoration)
            {
                if (decorationBounds(document_["Decorations"][it->index]).contains(worldPosition))
                {
                    candidates.push_back(*it);
                }
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
        if (usesSharedWorldDrawOrder(selection_.kind))
        {
            return moveSharedWorldDrawOrder(delta);
        }

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
        if (usesSharedWorldDrawOrder(selection_.kind))
        {
            return moveSharedWorldToDrawOrderEdge(toFront);
        }

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
        if (usesSharedWorldDrawOrder(selection_.kind))
        {
            const std::vector<EditorSelection> orderedSelections = sharedWorldDrawOrderSelections();
            const auto currentIt = std::find(orderedSelections.begin(), orderedSelections.end(), selection_);
            if (currentIt == orderedSelections.end())
            {
                return;
            }

            const std::size_t currentIndex = static_cast<std::size_t>(std::distance(orderedSelections.begin(), currentIt));
            int layer = sharedWorldDrawOrder(selection_.kind, selection_.index);
            ImGui::SeparatorText("Draw Order");
            ImGui::TextDisabled(
                "Shared world layer. Lower number is drawn earlier. Current sorted position %zu of %zu.",
                currentIndex + 1u,
                orderedSelections.size()
            );
            if (ImGui::InputInt((std::string("Layer Number##") + idSuffix).c_str(), &layer))
            {
                setSharedWorldDrawOrder(selection_.kind, selection_.index, layer);
                markDirty();
            }

            const bool canMoveBackward = currentIndex > 0u;
            const bool canMoveForward = currentIndex + 1u < orderedSelections.size();

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
            return;
        }

        const nlohmann::json* array = arrayForSelectionKind(selection_.kind);
        if (array == nullptr ||
            selection_.kind == SelectionKind::Spawn ||
            selection_.kind == SelectionKind::Trader ||
            selection_.index >= array->size())
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

    bool trySelectTraderAt(const sf::Vector2f worldPosition)
    {
        if (!isTraderEditable() || !traderMarkerBounds().contains(worldPosition))
        {
            return false;
        }

        selection_ = {SelectionKind::Trader, 0u};
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

        const auto trySelectSharedPlatformDecorationAt = [&](const bool platformsOnly, const bool decorationsOnly) {
            const std::vector<EditorSelection> sharedSelections = sharedWorldDrawOrderSelections();
            for (auto it = sharedSelections.rbegin(); it != sharedSelections.rend(); ++it)
            {
                if (platformsOnly && it->kind != SelectionKind::Platform)
                {
                    continue;
                }
                if (decorationsOnly && it->kind != SelectionKind::Decoration)
                {
                    continue;
                }

                if (it->kind == SelectionKind::Platform)
                {
                    if (platformContainsPoint(document_["Platforms"][it->index], worldPosition))
                    {
                        selection_ = *it;
                        return true;
                    }
                }
                else if (it->kind == SelectionKind::Decoration)
                {
                    if (decorationBounds(document_["Decorations"][it->index]).contains(worldPosition))
                    {
                        selection_ = *it;
                        return true;
                    }
                }
            }
            return false;
        };

        if (searchAllKinds)
        {
            if (trySelectSpawnAt(worldPosition))
            {
                return;
            }
            if (trySelectTraderAt(worldPosition))
            {
                return;
            }
            if (trySelectFromArray(document_["Interactives"], SelectionKind::Interactive, worldPosition, [&](const auto& value) { return interactiveBounds(value); }))
            {
                return;
            }
            if (trySelectFromArray(document_["Spawners"], SelectionKind::Spawner, worldPosition, [&](const auto& value) { return combinedSpawnerEditBounds(value); }))
            {
                return;
            }
            if (trySelectFromArray(document_["Portals"], SelectionKind::Portal, worldPosition, [&](const auto& value) { return combinedPortalEditBounds(value); }))
            {
                return;
            }
            if (trySelectFromArray(document_["Barriers"], SelectionKind::Barrier, worldPosition, [&](const auto& value) { return worldHazardBounds(value); }))
            {
                return;
            }
            if (trySelectFromArray(document_["DeadAreas"], SelectionKind::DeadArea, worldPosition, [&](const auto& value) { return worldHazardBounds(value); }))
            {
                return;
            }
            if (trySelectSharedPlatformDecorationAt(false, false))
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
            if (trySelectSpawnAt(worldPosition))
            {
                return;
            }
            trySelectTraderAt(worldPosition);
            return;
        }

        switch (activeTab_)
        {
        case EditorTab::Platforms:
            trySelectSharedPlatformDecorationAt(true, false);
            break;
        case EditorTab::Decorations:
            trySelectSharedPlatformDecorationAt(false, true);
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
            trySelectFromArray(document_["Spawners"], SelectionKind::Spawner, worldPosition, [&](const auto& value) { return combinedSpawnerEditBounds(value); });
            break;
        case EditorTab::Portals:
            trySelectFromArray(document_["Portals"], SelectionKind::Portal, worldPosition, [&](const auto& value) { return combinedPortalEditBounds(value); });
            break;
        case EditorTab::Interactives:
            trySelectFromArray(document_["Interactives"], SelectionKind::Interactive, worldPosition, [&](const auto& value) { return interactiveBounds(value); });
            break;
        case EditorTab::Hazards:
            if (trySelectFromArray(document_["Platforms"], SelectionKind::Platform, worldPosition, [&](const auto& value) {
                    return isHazardPlatformHitbox(value) ? platformBounds(value) : sf::FloatRect{};
                }))
            {
                break;
            }
            if (!trySelectFromArray(document_["Barriers"], SelectionKind::Barrier, worldPosition, [&](const auto& value) { return worldHazardBounds(value); }))
            {
                trySelectFromArray(document_["DeadAreas"], SelectionKind::DeadArea, worldPosition, [&](const auto& value) { return worldHazardBounds(value); });
            }
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
            {"EditorDrawOrder", nextWorldDrawOrder()},
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
            {"EditorDrawOrder", nextWorldDrawOrder()},
            {"Name", decorationName},
            {"Position", toJson(removeParallaxPreview(worldPosition, parallax))},
            {"Scale", {1.f, 1.f}},
            {"Rotation", 0.f},
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
        (void)worldPosition;
        document_["Background"].push_back({
            {"EditorDrawOrder", nextWorldDrawOrder()},
            {"BgName", backgroundName},
            {"Position", toJson(backgroundTileCenter({0, 0}))},
            {"ParallaxFactor", toJson(parallax)},
            {"Type", "repeatedXY"}
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
            {"ActivationMode", "Immediate"},
            {"ActivationPadding", 120.f},
            {"ActivationArea", toJson(sf::FloatRect({worldPosition.x - 210.f, worldPosition.y - 180.f}, {420.f, 300.f}))},
            {"FirstSpawnDelayMs", 0},
            {"EnemyHP", 0},
            {"EnemyDamage", 0},
            {"GoldReward", 0},
            {"SpawnArea", {
                {worldPosition.x - 90.f, worldPosition.x + 90.f},
                {worldPosition.y - 80.f, worldPosition.y + 20.f}
            }}
        });
        selection_ = {SelectionKind::Spawner, document_["Spawners"].size() - 1u};
        markDirty();
    }

    void placePortal(const sf::Vector2f worldPosition)
    {
        const int portalIndex = static_cast<int>(document_["Portals"].size()) + 1;
        document_["Portals"].push_back({
            {"EditorDrawOrder", nextWorldDrawOrder()},
            {"Id", "portal_" + std::to_string(portalIndex)},
            {"Title", "Portal " + std::to_string(portalIndex)},
            {"Position", toJson(worldPosition)},
            {"Scale", {0.36f, 0.36f}},
            {"PortalTexture", "portalGreen"},
            {"Color", {212, 236, 255, 245}},
            {"AccentColor", {112, 208, 255, 255}},
            {"InteractRadius", 130.f},
            {"ActivationArea", toJson(sf::FloatRect({worldPosition.x - 130.f, worldPosition.y - 130.f}, {260.f, 260.f}))},
            {"Prompt", "Enter portal"},
            {"Target", {
                {"Type", "Position"},
                {"Position", {worldPosition.x + 160.f, worldPosition.y}}
            }}
        });
        selection_ = {SelectionKind::Portal, document_["Portals"].size() - 1u};
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
            {"EditorDrawOrder", nextWorldDrawOrder()},
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

    void placeDeadArea(const sf::Vector2f worldPosition)
    {
        const int deadAreaIndex = static_cast<int>(document_["DeadAreas"].size()) + 1;
        document_["DeadAreas"].push_back({
            {"Id", "dead_area_" + std::to_string(deadAreaIndex)},
            {"Enabled", true},
            {"Rect", {worldPosition.x - 120.f, worldPosition.y - 32.f, 240.f, 64.f}},
            {"CoreColor", {242, 104, 56, 255}},
            {"GlowColor", {255, 182, 96, 255}},
            {"EmberColor", {255, 236, 188, 255}}
        });
        selection_ = {SelectionKind::DeadArea, document_["DeadAreas"].size() - 1u};
        markDirty();
    }

    void placeBarrier(const sf::Vector2f worldPosition)
    {
        const int barrierIndex = static_cast<int>(document_["Barriers"].size()) + 1;
        document_["Barriers"].push_back({
            {"Id", "barrier_" + std::to_string(barrierIndex)},
            {"Enabled", true},
            {"BlocksPlayer", true},
            {"Rect", {worldPosition.x - 12.f, worldPosition.y - 110.f, 24.f, 220.f}},
            {"CoreColor", {130, 214, 184, 255}},
            {"GlowColor", {156, 238, 208, 255}}
        });
        selection_ = {SelectionKind::Barrier, document_["Barriers"].size() - 1u};
        markDirty();
    }

    void placePlatformHitbox(const sf::Vector2f worldPosition)
    {
        const sf::Vector2f size{160.f, 160.f};
        document_["Platforms"].push_back({
            {"EditorDrawOrder", nextWorldDrawOrder()},
            {"Type", "Invisible-wall"},
            {"HazardTool", "PlatformHitbox"},
            {"Position", {worldPosition.x - size.x * 0.5f, worldPosition.y - size.y * 0.5f}},
            {"EditHitbox", true},
            {"HitboxOffset", {0.f, 0.f}},
            {"HitboxSize", {size.x, size.y}},
            {"BounceEnabled", false}
        });
        selection_ = {SelectionKind::Platform, document_["Platforms"].size() - 1u};
        markDirty();
    }

    nlohmann::json makeMiniLocationPreset(const sf::FloatRect& bounds) const
    {
        const sf::Vector2f spawn{bounds.size.x * 0.5f, bounds.size.y - 42.f};
        const float floorY = bounds.size.y - 22.f;
        const float shelfY = bounds.size.y - 164.f;
        const float centerX = bounds.size.x * 0.5f;

        return {
            {"SpawnPosition", toJson(spawn)},
            {"Platforms", nlohmann::json::array({
                {{"Type", "Quadruple"}, {"Position", {92.f, floorY}}},
                {{"Type", "Quadruple"}, {"Position", {392.f, floorY}}},
                {{"Type", "Quadruple"}, {"Position", {bounds.size.x - 252.f, floorY}}},
                {{"Type", "Crypt-pillar"}, {"Position", {18.f, floorY - 142.f}}},
                {{"Type", "Crypt-pillar"}, {"Position", {bounds.size.x - 78.f, floorY - 142.f}}},
                {{"Type", "Fallen-arcade"}, {"Position", {centerX - 166.f, 34.f}}},
                {{"Type", "Triple"}, {"Position", {centerX - 135.f, shelfY}}},
                {{"Type", "Double-horizontal-1"}, {"Position", {136.f, floorY - 96.f}}}
            })},
            {"Decorations", nlohmann::json::array({
                {{"Name", "MossyBackgroundDecoration_07.png"}, {"Position", {centerX, floorY - 118.f}}, {"Scale", {1.04f, 1.04f}}, {"ParallaxFactor", {1.f, 1.f}}, {"Z", -4}, {"Color", {255, 255, 255, 96}}},
                {{"Name", "MossyHills_06.png"}, {"Position", {centerX, floorY + 34.f}}, {"Scale", {0.84f, 0.84f}}, {"ParallaxFactor", {1.f, 1.f}}, {"Z", -3}, {"Color", {176, 198, 204, 162}}},
                {{"Name", "MossyDecorationHazard_03.png"}, {"Position", {86.f, floorY - 10.f}}, {"Scale", {0.22f, 0.22f}}, {"Rotation", 0.f}, {"ParallaxFactor", {1.f, 1.f}}, {"Z", 1}, {"Color", {255, 255, 255, 220}}},
                {{"Name", "MossyDecorationHazard_19.png"}, {"Position", {bounds.size.x - 80.f, floorY - 6.f}}, {"Scale", {0.20f, 0.20f}}, {"Rotation", 0.f}, {"ParallaxFactor", {1.f, 1.f}}, {"Z", 1}, {"Color", {255, 255, 255, 216}}}
            })},
            {"Interactives", nlohmann::json::array()},
            {"Portals", nlohmann::json::array()},
            {"DeadAreas", nlohmann::json::array({
                {
                    {"Id", "dead_area_1"},
                    {"Enabled", true},
                    {"Rect", {-8.f, bounds.size.y + 72.f, bounds.size.x + 56.f, 96.f}},
                    {"CoreColor", {242, 104, 56, 255}},
                    {"GlowColor", {255, 182, 96, 255}},
                    {"EmberColor", {255, 236, 188, 255}}
                }
            })}
        };
    }

    void placeMiniLocation(const sf::Vector2f worldPosition)
    {
        auto& presets = document_["Presets"];
        const float currentLevelWidth = presets["Size"][0].get<float>();
        const float mainWorldWidth = static_cast<float>(presets.value("MainWorldWidth", static_cast<int>(currentLevelWidth)));
        float roomX = std::max(currentLevelWidth + 260.f, mainWorldWidth + 640.f);
        float roomY = 420.f;
        const float roomWidth = 920.f;
        const float roomHeight = 340.f;
        const int locationIndex = static_cast<int>(document_["MiniLocations"].size()) + 1;

        sf::FloatRect proposedBounds{{roomX, roomY}, {roomWidth, roomHeight}};
        for (int attempt = 0; attempt < 32 && miniLocationOverlapsAnother(proposedBounds, document_["MiniLocations"].size()); ++attempt)
        {
            roomX += roomWidth + 180.f;
            proposedBounds.position.x = roomX;
        }

        nlohmann::json location = {
            {"Id", "mini_location_" + std::to_string(locationIndex)},
            {"Title", "Mini Location " + std::to_string(locationIndex)},
            {"Bounds", {proposedBounds.position.x, proposedBounds.position.y, proposedBounds.size.x, proposedBounds.size.y}},
            {"BarrierEnabled", true},
            {"BarrierWidth", 22.f},
            {"AccentColor", {130, 214, 184, 255}},
            {"Entry", {
                {"Texture", "MossyDecorationHazard_25.png"},
                {"Position", {worldPosition.x, worldPosition.y}},
                {"Scale", {0.22f, 0.33f}},
                {"DestinationSupport", {proposedBounds.position.x + 180.f, proposedBounds.position.y + proposedBounds.size.y - 18.f}},
                {"Color", {214, 246, 232, 255}},
                {"AccentColor", {130, 214, 184, 255}},
                {"InteractRadius", 126.f},
                {"Prompt", "Enter the hidden route"}
            }},
            {"Exit", {
                {"Texture", "MossyDecorationHazard_24.png"},
                {"Position", {proposedBounds.position.x + 110.f, proposedBounds.position.y + proposedBounds.size.y - 18.f}},
                {"Scale", {0.22f, 0.33f}},
                {"DestinationSupport", {worldPosition.x, worldPosition.y - 18.f}},
                {"Color", {212, 232, 255, 255}},
                {"AccentColor", {130, 214, 184, 255}},
                {"InteractRadius", 126.f},
                {"Prompt", "Enter to return"}
            }},
        };

        const nlohmann::json preset = makeMiniLocationPreset(proposedBounds);
        for (const auto& item : preset.items())
        {
            location[item.key()] = item.value();
        }
        document_["MiniLocations"].push_back(location);

        presets["MainWorldWidth"] = static_cast<int>(std::max(mainWorldWidth, currentLevelWidth));
        presets["Size"][0] = static_cast<int>(std::ceil(proposedBounds.position.x + proposedBounds.size.x + 220.f));
        presets["GenerateMiniLocations"] = false;

        selection_ = {SelectionKind::MiniLocation, document_["MiniLocations"].size() - 1u};
        markDirty();
        focusMiniLocation(selection_.index);
    }

    void deleteSelection()
    {
        if (deleteMiniLocationContentSelection())
        {
            return;
        }

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
                portalInteraction_.clear();
                hazardInteraction_.clear();
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
            if (document_["Ground"].size() <= 1u)
            {
                return;
            }
            eraseFromArray("Ground");
            break;
        case SelectionKind::Spawner:
            eraseFromArray("Spawners");
            break;
        case SelectionKind::Portal:
            eraseFromArray("Portals");
            break;
        case SelectionKind::Interactive:
            eraseFromArray("Interactives");
            break;
        case SelectionKind::DeadArea:
            eraseFromArray("DeadAreas");
            break;
        case SelectionKind::Barrier:
            eraseFromArray("Barriers");
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
        sprite.setColor(selected ? sf::Color(255, 255, 255, 220) : sf::Color(255, 255, 255, 180));

        if (!backgroundTypeIsSingle(background))
        {
            const sf::Vector2i minTile = backgroundTileMinIndex();
            const sf::Vector2i maxTile = backgroundTileMaxIndex();
            const sf::Vector2i tile = backgroundTileIndex(background);
            const bool repeatX = backgroundTypeRepeatsX(background);
            const bool repeatY = backgroundTypeRepeatsY(background);
            const int firstColumn = repeatX ? 0 : tile.x;
            const int lastColumn = repeatX ? maxTile.x : tile.x;
            const int firstRow = repeatY ? minTile.y : tile.y;
            const int lastRow = repeatY ? maxTile.y : tile.y;
            const sf::Vector2f parallax = backgroundPreviewParallax(background);
            for (int y = firstRow; y <= lastRow; ++y)
            {
                for (int x = firstColumn; x <= lastColumn; ++x)
                {
                    sf::Sprite repeatedSprite(sprite);
                    repeatedSprite.setPosition(applyParallaxPreview(backgroundTileDisplayCenter({x, y}), parallax));
                    window_.draw(repeatedSprite);
                }
            }
        }
        else
        {
            sprite.setPosition(backgroundDisplayPosition(background));
            window_.draw(sprite);
        }
    }

    void drawWorld()
    {
        window_.setView(worldView_);
        drawGrid();

        const std::vector<EditorSelection> sharedSelections = sharedWorldDrawOrderSelections();
        for (const EditorSelection& entry : sharedSelections)
        {
            if (entry.kind == SelectionKind::Background && entry.index < document_["Background"].size())
            {
                drawBackgroundPreview(document_["Background"][entry.index], selection_.kind == SelectionKind::Background && selection_.index == entry.index);
            }
            else if (entry.kind == SelectionKind::Decoration)
            {
                drawDecorationPreview(entry.index);
            }
            else if (entry.kind == SelectionKind::Ground)
            {
                if (entry.index == 0u)
                {
                    drawGroundPreview();
                }
            }
            else if (entry.kind == SelectionKind::Actors)
            {
                drawSpawnPreview();
            }
            else if (entry.kind == SelectionKind::Interactive)
            {
                if (entry.index == 0u)
                {
                    drawInteractivePreview();
                }
            }
            else if (entry.kind == SelectionKind::Portal)
            {
                if (entry.index == 0u)
                {
                    drawPortalPreview();
                }
            }
            else if (entry.kind == SelectionKind::Platform)
            {
                drawPlatformPreview(entry.index);
            }
        }
        drawSpawnerPreview();
        drawHazardPreview();
        drawMiniLocationPreview();
    }

    void drawWorldHazardArray(const char* arrayName, const SelectionKind kind, const sf::Color fallbackCore, const sf::Color fallbackOutline)
    {
        if (!document_.contains(arrayName) || !document_[arrayName].is_array())
        {
            return;
        }

        const auto& hazards = document_[arrayName];
        for (std::size_t index = 0; index < hazards.size(); ++index)
        {
            const auto& hazard = hazards[index];
            const bool enabled = hazard.value("Enabled", true);
            const bool selected = selection_.kind == kind && selection_.index == index;
            const sf::FloatRect bounds = worldHazardBounds(hazard);
            const sf::Color core = readColor(hazard.value("CoreColor", nlohmann::json::array()), fallbackCore);
            const sf::Color outline = readColor(hazard.value("GlowColor", nlohmann::json::array()), fallbackOutline);

            sf::RectangleShape rect(bounds.size);
            rect.setPosition(bounds.position);
            rect.setFillColor(sf::Color(core.r, core.g, core.b, enabled ? (selected ? 70 : 42) : 18));
            rect.setOutlineThickness(selected ? kSelectionOutlineThickness : 1.5f);
            rect.setOutlineColor(sf::Color(outline.r, outline.g, outline.b, enabled ? (selected ? 230 : 154) : 86));
            window_.draw(rect);

            sf::Text label(font_);
            label.setCharacterSize(14u);
            label.setFillColor(sf::Color(outline.r, outline.g, outline.b, enabled ? 230 : 130));
            label.setString(kind == SelectionKind::Barrier ? "Barrier" : "DeadArea");
            label.setPosition(bounds.position + sf::Vector2f{6.f, 5.f});
            window_.draw(label);

            if (selected)
            {
                const sf::FloatRect handleBounds = worldHazardResizeHandleBounds(hazard);
                sf::RectangleShape handle(handleBounds.size);
                handle.setPosition(handleBounds.position);
                handle.setFillColor(outline);
                handle.setOutlineThickness(1.f);
                handle.setOutlineColor(sf::Color(18, 24, 24, 220));
                window_.draw(handle);
            }
        }
    }

    void drawHazardPreview()
    {
        drawWorldHazardArray("DeadAreas", SelectionKind::DeadArea, sf::Color(242, 104, 56, 255), sf::Color(255, 182, 96, 255));
        drawWorldHazardArray("Barriers", SelectionKind::Barrier, sf::Color(130, 214, 184, 255), sf::Color(156, 238, 208, 255));
    }

    void drawPortalPreview()
    {
        if (!document_.contains("Portals") || !document_["Portals"].is_array())
        {
            return;
        }

        for (std::size_t index = 0; index < document_["Portals"].size(); ++index)
        {
            const auto& portal = document_["Portals"][index];
            const sf::Vector2f position = readVector2f(portal.value("Position", nlohmann::json::array()));
            const sf::Vector2f scale = readVector2f(portal.value("Scale", nlohmann::json::array()), {0.36f, 0.36f});
            const sf::Color color = readColor(portal.value("Color", nlohmann::json::array()), sf::Color(212, 236, 255, 245));
            const bool selected = selection_.kind == SelectionKind::Portal && selection_.index == index;

            const std::string portalTexture = portal.value("PortalTexture", portal.value("Texture", std::string{"portalGreen"}));
            drawPortalMarker(position, color, portalTexture);

            const sf::FloatRect activationBounds = portalActivationBounds(portal);
            sf::RectangleShape activationRect(activationBounds.size);
            activationRect.setPosition(activationBounds.position);
            activationRect.setFillColor(sf::Color(112, 208, 255, selected ? 26 : 12));
            activationRect.setOutlineThickness(selected ? 1.5f : 1.f);
            activationRect.setOutlineColor(sf::Color(112, 208, 255, selected ? 190 : 76));
            window_.draw(activationRect);

            sf::RectangleShape outline(portalBounds(portal).size);
            outline.setPosition(portalBounds(portal).position);
            outline.setFillColor(selected ? sf::Color(kPortalOutlineColor.r, kPortalOutlineColor.g, kPortalOutlineColor.b, 22) : sf::Color::Transparent);
            outline.setOutlineThickness(selected ? kSelectionOutlineThickness : 1.f);
            outline.setOutlineColor(selected ? kPortalOutlineColor : sf::Color(112, 208, 255, 80));
            window_.draw(outline);

            if (selected)
            {
                const sf::FloatRect handleBounds = portalActivationHandleBounds(portal);
                sf::RectangleShape handle(handleBounds.size);
                handle.setPosition(handleBounds.position);
                handle.setFillColor(kPortalOutlineColor);
                handle.setOutlineThickness(1.f);
                handle.setOutlineColor(sf::Color(18, 24, 32, 220));
                window_.draw(handle);
            }

            sf::Text label(font_);
            label.setCharacterSize(16u);
            label.setFillColor(sf::Color(214, 238, 255, 232));
            label.setString(portal.value("Title", std::string{"Portal"}));
            label.setPosition({position.x + 14.f, position.y - 32.f});
            window_.draw(label);

            if (portal.contains("Target") && portal["Target"].is_object())
            {
                const auto& target = portal["Target"];
                const std::string targetType = target.value("Type", std::string{"Position"});
                std::optional<sf::Vector2f> targetPosition;
                if (targetType == "Position")
                {
                    targetPosition = readVector2f(target.value("Position", nlohmann::json::array()), position);
                }
                else if (targetType == "MiniLocation")
                {
                    const std::string miniLocationId = target.value("MiniLocationId", std::string{});
                    for (const auto& location : document_["MiniLocations"])
                    {
                        if (location.value("Id", location.value("Title", std::string{})) == miniLocationId)
                        {
                            const sf::FloatRect bounds = miniLocationBounds(location);
                            targetPosition = bounds.position + readVector2f(
                                location.value("SpawnPosition", nlohmann::json::array()),
                                {bounds.size.x * 0.5f, bounds.size.y - 42.f}
                            );
                            break;
                        }
                    }
                }

                if (targetPosition.has_value())
                {
                    drawConnectionLine(position, *targetPosition, kPortalOutlineColor);
                }
            }

            (void)scale;
        }
    }

    bool deleteMiniLocationContentSelection()
    {
        nlohmann::json* location = selectedMiniLocationForContent();
        if (location == nullptr || !miniEditor_.selected.isValid() ||
            miniEditor_.selected.collection == MiniLocationContentCollection::SpawnPosition ||
            miniEditor_.selected.collection == MiniLocationContentCollection::Entry ||
            miniEditor_.selected.collection == MiniLocationContentCollection::Exit)
        {
            return false;
        }

        ensureMiniLocationNestedArrays(*location);
        auto eraseNested = [&](const char* key) {
            auto& array = (*location)[key];
            if (miniEditor_.selected.index >= array.size())
            {
                return false;
            }
            array.erase(array.begin() + static_cast<nlohmann::json::difference_type>(miniEditor_.selected.index));
            miniEditor_.clearSelection();
            miniLocationInteraction_.clear();
            markDirty();
            return true;
        };

        switch (miniEditor_.selected.collection)
        {
        case MiniLocationContentCollection::Platforms:
            return eraseNested("Platforms");
        case MiniLocationContentCollection::Decorations:
            return eraseNested("Decorations");
        case MiniLocationContentCollection::Interactives:
            return eraseNested("Interactives");
        case MiniLocationContentCollection::Portals:
            return eraseNested("Portals");
        case MiniLocationContentCollection::DeadAreas:
            return eraseNested("DeadAreas");
        case MiniLocationContentCollection::Barriers:
            return eraseNested("Barriers");
        default:
            return false;
        }
    }

    bool copyMiniLocationContentSelection()
    {
        const nlohmann::json* location = selectedMiniLocationForContent();
        if (location == nullptr || !miniEditor_.selected.isValid())
        {
            return false;
        }

        const char* key = miniLocationCollectionKey(miniEditor_.selected.collection);
        if (key[0] == '\0' || !location->contains(key) || !(*location)[key].is_array() ||
            miniEditor_.selected.index >= (*location)[key].size())
        {
            return false;
        }

        miniLocationClipboard_ = (*location)[key][miniEditor_.selected.index];
        miniLocationClipboardCollection_ = miniEditor_.selected.collection;
        return true;
    }

    bool pasteMiniLocationContentAt(const sf::Vector2f worldPosition)
    {
        nlohmann::json* location = selectedMiniLocationForContent();
        if (location == nullptr || miniLocationClipboardCollection_ == MiniLocationContentCollection::None ||
            miniLocationClipboard_.is_null())
        {
            return false;
        }

        const char* key = miniLocationCollectionKey(miniLocationClipboardCollection_);
        if (key[0] == '\0')
        {
            return false;
        }

        ensureMiniLocationNestedArrays(*location);
        nlohmann::json pasted = miniLocationClipboard_;
        const sf::Vector2f relative = contentRelativePosition(worldPosition, *location);
        if (miniLocationClipboardCollection_ == MiniLocationContentCollection::DeadAreas ||
            miniLocationClipboardCollection_ == MiniLocationContentCollection::Barriers)
        {
            sf::FloatRect rect = readRect(pasted.value("Rect", nlohmann::json::array()), sf::FloatRect({0.f, 0.f}, {120.f, 48.f}));
            rect.position = relative - rect.size * 0.5f;
            pasted["Rect"] = toJson(rect);
        }
        else
        {
            pasted["Position"] = toJson(relative);
        }

        pasted["EditorId"] = std::string(key) + "_copy_" + std::to_string((*location)[key].size() + 1u);
        (*location)[key].push_back(pasted);
        selectMiniLocationContent(miniLocationClipboardCollection_, (*location)[key].size() - 1u);
        markDirty();
        return true;
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

    void drawPlatformPreview(const std::size_t index)
    {
        if (index >= document_["Platforms"].size())
        {
            return;
        }

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

    void drawDecorationPreview(const std::size_t index)
    {
        if (index >= document_["Decorations"].size())
        {
            return;
        }

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
            sprite.setRotation(sf::degrees(decorationRotation(decoration)));
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

    void drawSpawnerPreview()
    {
        for (std::size_t index = 0; index < document_["Spawners"].size(); ++index)
        {
            const auto& spawner = document_["Spawners"][index];
            const sf::FloatRect bounds = spawnerBounds(spawner);
            const sf::FloatRect activationBounds = spawnerActivationBounds(spawner);
            const bool selected = selection_.kind == SelectionKind::Spawner && selection_.index == index;

            sf::RectangleShape activationRect(activationBounds.size);
            activationRect.setPosition(activationBounds.position);
            activationRect.setFillColor(sf::Color(244, 188, 92, selected ? 28 : 14));
            activationRect.setOutlineThickness(selected ? 1.5f : 1.f);
            activationRect.setOutlineColor(sf::Color(244, 188, 92, selected ? 190 : 88));
            window_.draw(activationRect);

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

                const sf::FloatRect activationHandleBounds = spawnerActivationResizeHandleBounds(spawner);
                sf::RectangleShape activationHandle(activationHandleBounds.size);
                activationHandle.setPosition(activationHandleBounds.position);
                activationHandle.setFillColor(sf::Color(244, 188, 92, 230));
                activationHandle.setOutlineThickness(1.f);
                activationHandle.setOutlineColor(sf::Color(24, 18, 18, 220));
                window_.draw(activationHandle);
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
            const sf::FloatRect visibleCameraRect = miniLocationVisibleCameraRect(location);
            const sf::Color accentColor = location.contains("AccentColor")
                ? readColor(location["AccentColor"], kMiniLocationOutlineColor)
                : kMiniLocationOutlineColor;
            const bool selected = selection_.kind == SelectionKind::MiniLocation && selection_.index == index;

            sf::RectangleShape visibleCamera(visibleCameraRect.size);
            visibleCamera.setPosition(visibleCameraRect.position);
            visibleCamera.setFillColor(sf::Color(accentColor.r, accentColor.g, accentColor.b, 14));
            visibleCamera.setOutlineThickness(selected ? kSelectionOutlineThickness : 1.5f);
            visibleCamera.setOutlineColor(selected
                ? accentColor
                : sf::Color(accentColor.r, accentColor.g, accentColor.b, 148));
            window_.draw(visibleCamera);

            sf::RectangleShape room(bounds.size);
            room.setPosition(bounds.position);
            room.setFillColor(sf::Color(accentColor.r, accentColor.g, accentColor.b, 18));
            room.setOutlineThickness(1.f);
            room.setOutlineColor(sf::Color(accentColor.r, accentColor.g, accentColor.b, 84));
            window_.draw(room);

            sf::Text boundsLabel(font_);
            boundsLabel.setCharacterSize(14u);
            boundsLabel.setFillColor(sf::Color(224, 250, 242, 210));
            boundsLabel.setString("Camera visible viewport");
            boundsLabel.setPosition({visibleCameraRect.position.x + 8.f, visibleCameraRect.position.y + 30.f});
            window_.draw(boundsLabel);

            sf::Text clampLabel(font_);
            clampLabel.setCharacterSize(12u);
            clampLabel.setFillColor(sf::Color(224, 250, 242, 150));
            clampLabel.setString("Camera clamp bounds");
            clampLabel.setPosition({bounds.position.x + 8.f, bounds.position.y + 50.f});
            window_.draw(clampLabel);

            if (location.contains("Barriers") && location["Barriers"].is_array())
            {
                for (const auto& barrierData : location["Barriers"])
                {
                    if (!barrierData.value("Enabled", true))
                    {
                        continue;
                    }
                    const sf::FloatRect barrierRect = deadAreaBounds(barrierData, location);
                    const sf::Color core = barrierData.contains("CoreColor")
                        ? readColor(barrierData["CoreColor"], accentColor)
                        : accentColor;
                    sf::RectangleShape barrier(barrierRect.size);
                    barrier.setPosition(barrierRect.position);
                    barrier.setFillColor(sf::Color(core.r, core.g, core.b, 74));
                    barrier.setOutlineThickness(1.f);
                    barrier.setOutlineColor(sf::Color(core.r, core.g, core.b, 188));
                    window_.draw(barrier);
                }
            }
            else if (location.value("BarrierEnabled", true))
            {
                const float barrierWidth = location.value("BarrierWidth", 22.f);
                const float barrierHeight = std::max(bounds.size.y + 180.f, 220.f);
                const float barrierTop = bounds.position.y - 90.f;
                const float leftBarrierX = bounds.position.x - 18.f + 36.f;
                const float rightBarrierX = bounds.position.x + bounds.size.x + 18.f + 36.f;
                const sf::Color barrierColor(accentColor.r, accentColor.g, accentColor.b, 74);

                for (const float x : {leftBarrierX, rightBarrierX})
                {
                    sf::RectangleShape barrier({barrierWidth, barrierHeight});
                    barrier.setPosition({x - barrierWidth * 0.5f, barrierTop});
                    barrier.setFillColor(barrierColor);
                    barrier.setOutlineThickness(1.f);
                    barrier.setOutlineColor(sf::Color(accentColor.r, accentColor.g, accentColor.b, 188));
                    window_.draw(barrier);
                }
            }

            if (location.contains("Hazard") && location["Hazard"].is_object())
            {
                const auto& hazard = location["Hazard"];
                if (hazard.value("Enabled", false))
                {
                    const sf::FloatRect hazardRect = readRect(hazard.value("Rect", nlohmann::json::array()));
                    sf::RectangleShape hazardPreview(hazardRect.size);
                    hazardPreview.setPosition(hazardRect.position);
                    hazardPreview.setFillColor(sf::Color(242, 104, 56, 54));
                    hazardPreview.setOutlineThickness(1.5f);
                    hazardPreview.setOutlineColor(sf::Color(255, 132, 78, 190));
                    window_.draw(hazardPreview);
                }
            }
            if (location.contains("DeadAreas") && location["DeadAreas"].is_array())
            {
                for (const auto& deadArea : location["DeadAreas"])
                {
                    if (!deadArea.value("Enabled", true))
                    {
                        continue;
                    }
                    const sf::FloatRect relativeRect = readRect(deadArea.value("Rect", nlohmann::json::array()));
                    sf::RectangleShape deadAreaPreview(relativeRect.size);
                    deadAreaPreview.setPosition(bounds.position + relativeRect.position);
                    deadAreaPreview.setFillColor(sf::Color(184, 42, 52, 56));
                    deadAreaPreview.setOutlineThickness(1.5f);
                    deadAreaPreview.setOutlineColor(sf::Color(255, 96, 96, 190));
                    window_.draw(deadAreaPreview);
                }
            }

            const float deathY = location.value("DeathY", std::numeric_limits<float>::max());
            if (std::isfinite(deathY) && deathY < std::numeric_limits<float>::max() * 0.5f)
            {
                sf::RectangleShape deathZone({bounds.size.x, 72.f});
                deathZone.setPosition({bounds.position.x, deathY});
                deathZone.setFillColor(sf::Color(184, 42, 52, 44));
                deathZone.setOutlineThickness(1.f);
                deathZone.setOutlineColor(sf::Color(255, 96, 96, 150));
                window_.draw(deathZone);
            }

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

            drawPortalMarker(entryPos, accentColor, location["Entry"].value("Texture", std::string{}));
            drawPortalMarker(exitPos, sf::Color(212, 232, 255, 255), location["Exit"].value("Texture", std::string{}));

            const sf::Vector2f origin = bounds.position;
            const auto drawNestedPlatform = [&](const std::size_t nestedIndex) {
                if (!location.contains("Platforms") || !location["Platforms"].is_array() || nestedIndex >= location["Platforms"].size())
                {
                    return;
                }
                const auto& nestedPlatform = location["Platforms"][nestedIndex];
                nlohmann::json absolutePlatform = nestedPlatform;
                absolutePlatform["Position"] = toJson(origin + readVector2f(nestedPlatform.value("Position", nlohmann::json::array())));
                const sf::FloatRect platformRect = platformBounds(absolutePlatform);
                const sf::FloatRect spriteRect = platformSpriteBounds(absolutePlatform);
                const std::string typeName = absolutePlatform.value("Type", std::string{});
                const auto definition = platformDefinition(absolutePlatform);
                if (definition.has_value())
                {
                    if (const sf::Texture* texture = findPreviewTexture(typeName); texture != nullptr)
                    {
                        sf::Sprite sprite(*texture);
                        sprite.setOrigin(sprite.getGlobalBounds().getCenter());
                        sprite.setScale(platformScale(absolutePlatform));
                        sprite.setColor(definition->tint);
                        sprite.setPosition(platformSpriteCenter(absolutePlatform) + definition->spriteOffset);
                        window_.draw(sprite);
                    }
                }

                sf::RectangleShape marker(platformRect.size);
                marker.setPosition(platformRect.position);
                marker.setFillColor(sf::Color(kPlatformOutlineColor.r, kPlatformOutlineColor.g, kPlatformOutlineColor.b, 28));
                marker.setOutlineThickness(1.f);
                marker.setOutlineColor(sf::Color(kPlatformOutlineColor.r, kPlatformOutlineColor.g, kPlatformOutlineColor.b, 120));
                window_.draw(marker);
                (void)spriteRect;
            };

            const auto drawNestedDecoration = [&](const std::size_t nestedIndex) {
                if (!location.contains("Decorations") || !location["Decorations"].is_array() || nestedIndex >= location["Decorations"].size())
                {
                    return;
                }
                const auto& nestedDecoration = location["Decorations"][nestedIndex];
                nlohmann::json absoluteDecoration = nestedDecoration;
                absoluteDecoration["Position"] = toJson(origin + readVector2f(nestedDecoration.value("Position", nlohmann::json::array())));
                const std::string textureName = absoluteDecoration.value("Name", std::string{});
                const sf::Vector2f position = readVector2f(absoluteDecoration.value("Position", nlohmann::json::array()));
                const sf::Color color = readColor(absoluteDecoration.value("Color", nlohmann::json::array()), sf::Color::White);
                if (const sf::Texture* texture = findPreviewTexture(textureName); texture != nullptr)
                {
                    sf::Sprite sprite(*texture);
                    sprite.setOrigin(sprite.getGlobalBounds().getCenter());
                    sprite.setPosition(position);
                    sprite.setScale(decorationScale(absoluteDecoration));
                    sprite.setRotation(sf::degrees(decorationRotation(absoluteDecoration)));
                    sprite.setColor(color);
                    window_.draw(sprite);
                }

                const sf::FloatRect decorationRect = decorationSpriteBounds(
                    textureName,
                    position,
                    decorationScale(absoluteDecoration),
                    decorationRotation(absoluteDecoration)
                );
                sf::RectangleShape marker(decorationRect.size);
                marker.setPosition(decorationRect.position);
                marker.setFillColor(sf::Color(kDecorationOutlineColor.r, kDecorationOutlineColor.g, kDecorationOutlineColor.b, 18));
                marker.setOutlineThickness(1.f);
                marker.setOutlineColor(sf::Color(kDecorationOutlineColor.r, kDecorationOutlineColor.g, kDecorationOutlineColor.b, 96));
                window_.draw(marker);
            };

            const auto drawNestedInteractive = [&](const std::size_t nestedIndex) {
                if (!location.contains("Interactives") || !location["Interactives"].is_array() || nestedIndex >= location["Interactives"].size())
                {
                    return;
                }
                const auto& nestedInteractive = location["Interactives"][nestedIndex];
                sf::CircleShape marker(10.f);
                marker.setOrigin({10.f, 10.f});
                marker.setPosition(origin + readVector2f(nestedInteractive.value("Position", nlohmann::json::array())));
                marker.setFillColor(sf::Color(kInteractiveOutlineColor.r, kInteractiveOutlineColor.g, kInteractiveOutlineColor.b, 80));
                marker.setOutlineThickness(1.f);
                marker.setOutlineColor(kInteractiveOutlineColor);
                window_.draw(marker);
            };

            const auto drawNestedPortal = [&](const std::size_t nestedIndex) {
                if (!location.contains("Portals") || !location["Portals"].is_array() || nestedIndex >= location["Portals"].size())
                {
                    return;
                }
                const auto& nestedPortal = location["Portals"][nestedIndex];
                drawPortalMarker(
                    origin + readVector2f(nestedPortal.value("Position", nlohmann::json::array())),
                    kPortalOutlineColor,
                    nestedPortal.value("PortalTexture", nestedPortal.value("Texture", std::string{"portalGreen"})));
            };

            for (const MiniLocationObjectRef& drawEntry : miniLocationDrawOrderEntries(index))
            {
                switch (drawEntry.collection)
                {
                case MiniLocationContentCollection::Decorations:
                    drawNestedDecoration(drawEntry.index);
                    break;
                case MiniLocationContentCollection::Interactives:
                    drawNestedInteractive(drawEntry.index);
                    break;
                case MiniLocationContentCollection::Portals:
                    drawNestedPortal(drawEntry.index);
                    break;
                case MiniLocationContentCollection::Platforms:
                    drawNestedPlatform(drawEntry.index);
                    break;
                default:
                    break;
                }
            }

            const sf::Vector2f spawnPosition = origin + readVector2f(
                location.value("SpawnPosition", nlohmann::json::array()),
                {bounds.size.x * 0.5f, bounds.size.y - 42.f}
            );
            sf::CircleShape spawnMarker(13.f);
            spawnMarker.setOrigin({13.f, 13.f});
            spawnMarker.setPosition(spawnPosition);
            spawnMarker.setFillColor(sf::Color(255, 255, 255, 42));
            spawnMarker.setOutlineThickness(2.f);
            spawnMarker.setOutlineColor(sf::Color(255, 255, 255, 220));
            window_.draw(spawnMarker);

            sf::Text spawnLabel(font_);
            spawnLabel.setCharacterSize(13u);
            spawnLabel.setFillColor(sf::Color(255, 255, 255, 210));
            spawnLabel.setString("Portal spawn");
            spawnLabel.setPosition({spawnPosition.x + 14.f, spawnPosition.y - 22.f});
            window_.draw(spawnLabel);

            if (selected && miniEditor_.contentMode && miniEditor_.selected.isValid() &&
                miniEditor_.selected.locationIndex == index)
            {
                const sf::FloatRect selectedBounds = miniLocationContentBounds(miniEditor_.selected);
                if (selectedBounds.size.x > 0.f && selectedBounds.size.y > 0.f)
                {
                    sf::RectangleShape selectedOutline(selectedBounds.size);
                    selectedOutline.setPosition(selectedBounds.position);
                    selectedOutline.setFillColor(sf::Color(255, 255, 255, 22));
                    selectedOutline.setOutlineThickness(3.f);
                    selectedOutline.setOutlineColor(sf::Color(255, 236, 150, 255));
                    window_.draw(selectedOutline);

                    const sf::FloatRect handleBounds = platformHandleBounds(selectedBounds.position + selectedBounds.size);
                    sf::RectangleShape handle(handleBounds.size);
                    handle.setPosition(handleBounds.position);
                    handle.setFillColor(sf::Color(255, 236, 150, 255));
                    handle.setOutlineThickness(1.f);
                    handle.setOutlineColor(sf::Color(28, 24, 16, 240));
                    window_.draw(handle);
                }
            }

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
        const int actorLayer = sharedWorldDrawOrder(SelectionKind::Actors, 0u);
        const bool selectedActorLayer = selection_.kind == SelectionKind::Actors;
        const bool selectedSpawn = selection_.kind == SelectionKind::Spawn;

        sf::RectangleShape playerBody({26.f, 44.f});
        playerBody.setOrigin({13.f, 38.f});
        playerBody.setPosition(spawn);
        playerBody.setFillColor(selectedActorLayer
            ? sf::Color(255, 236, 150, 92)
            : sf::Color(255, 255, 255, 44));
        playerBody.setOutlineThickness(selectedActorLayer ? kSelectionOutlineThickness : 1.5f);
        playerBody.setOutlineColor(selectedActorLayer ? sf::Color(255, 236, 150, 255) : kSpawnOutlineColor);
        window_.draw(playerBody);

        sf::CircleShape marker(16.f);
        marker.setOrigin({16.f, 16.f});
        marker.setPosition(spawn);
        marker.setFillColor(selectedSpawn ? sf::Color(255, 255, 255, 52) : sf::Color(255, 255, 255, 22));
        marker.setOutlineThickness(selectedSpawn ? kSelectionOutlineThickness : 1.5f);
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

        sf::Text layerLabel(font_);
        layerLabel.setCharacterSize(15u);
        layerLabel.setFillColor(selectedActorLayer ? sf::Color(255, 236, 150, 255) : sf::Color(255, 255, 255, 230));
        layerLabel.setOutlineThickness(2.f);
        layerLabel.setOutlineColor(sf::Color(18, 16, 18, 220));
        std::string label = "Actors layer";
        label += ": " + std::to_string(actorLayer);
        layerLabel.setString(label + "\nplayer + enemies");
        layerLabel.setPosition({spawn.x + 22.f, spawn.y - 62.f});
        window_.draw(layerLabel);

        if (document_["Presets"].value("LevelId", std::string{}) == "level1.json")
        {
            const sf::Vector2f traderPosition = this->traderPosition();
            const bool selectedTrader = selection_.kind == SelectionKind::Trader;

            sf::CircleShape traderMarker(18.f);
            traderMarker.setOrigin({18.f, 18.f});
            traderMarker.setPosition(traderPosition);
            traderMarker.setFillColor(selectedTrader ? sf::Color(220, 144, 76, 82) : sf::Color(220, 144, 76, 48));
            traderMarker.setOutlineThickness(selectedTrader ? kSelectionOutlineThickness : 2.f);
            traderMarker.setOutlineColor(selectedTrader ? sf::Color(255, 236, 150, 255) : sf::Color(220, 144, 76, 230));
            window_.draw(traderMarker);

            sf::VertexArray traderCross(sf::PrimitiveType::Lines, 4u);
            traderCross[0].position = {traderPosition.x - 16.f, traderPosition.y};
            traderCross[1].position = {traderPosition.x + 16.f, traderPosition.y};
            traderCross[2].position = {traderPosition.x, traderPosition.y - 20.f};
            traderCross[3].position = {traderPosition.x, traderPosition.y + 20.f};
            for (std::size_t index = 0; index < traderCross.getVertexCount(); ++index)
            {
                traderCross[index].color = selectedTrader ? sf::Color(255, 236, 150, 255) : sf::Color(220, 144, 76, 220);
            }
            window_.draw(traderCross);

            sf::Text traderLabel(font_);
            traderLabel.setCharacterSize(14u);
            traderLabel.setFillColor(sf::Color(255, 211, 158, 235));
            traderLabel.setOutlineThickness(2.f);
            traderLabel.setOutlineColor(sf::Color(18, 16, 18, 220));
            traderLabel.setString("Trader");
            traderLabel.setPosition({traderPosition.x + 24.f, traderPosition.y - 26.f});
            window_.draw(traderLabel);
        }
    }

    void drawUi()
    {
        drawToolbarWindow();
        drawLevelsWindow();
        drawInspectorWindow();
        drawMiniLocationContextMenu();
        drawWorldContextMenu();
    }

    nlohmann::json* selectedMiniLocationContentObject(nlohmann::json& location)
    {
        if (!miniEditor_.selected.isValid() || miniEditor_.selected.locationIndex != selection_.index)
        {
            return nullptr;
        }

        const char* key = miniLocationCollectionKey(miniEditor_.selected.collection);
        if (key[0] == '\0' || !location.contains(key) || !location[key].is_array() ||
            miniEditor_.selected.index >= location[key].size())
        {
            return nullptr;
        }

        return &location[key][miniEditor_.selected.index];
    }

    void drawMiniLocationContentDrawOrderControls(nlohmann::json& location, const char* idSuffix, bool& changed)
    {
        const MiniLocationContentCollection collection = miniEditor_.selected.collection;
        if (!miniLocationCollectionUsesDrawOrder(collection))
        {
            return;
        }

        const std::vector<MiniLocationObjectRef> drawEntries = miniLocationDrawOrderEntries(selection_.index);
        const auto currentEntryIt = std::find_if(drawEntries.begin(), drawEntries.end(), [&](const MiniLocationObjectRef& entry) {
            return entry.sameObject(miniEditor_.selected);
        });
        const std::size_t currentLayerIndex = currentEntryIt == drawEntries.end()
            ? 0u
            : static_cast<std::size_t>(std::distance(drawEntries.begin(), currentEntryIt));
        int layer = miniLocationDrawOrder(location, collection, miniEditor_.selected.index);

        ImGui::SeparatorText("Draw Order");
        ImGui::TextDisabled(
            "Layer %zu of %zu. Lower number is drawn earlier.",
            currentEntryIt == drawEntries.end() ? 0u : currentLayerIndex + 1u,
            drawEntries.size()
        );
        if (ImGui::InputInt((std::string("Layer Number##") + idSuffix).c_str(), &layer))
        {
            setMiniLocationDrawOrder(location, collection, miniEditor_.selected.index, layer);
            changed = true;
        }

        const bool canMoveBackward = currentEntryIt != drawEntries.end() && currentLayerIndex > 0u;
        const bool canMoveForward = currentEntryIt != drawEntries.end() && currentLayerIndex + 1u < drawEntries.size();
        ImGui::BeginDisabled(!canMoveBackward);
        if (ImGui::Button((std::string("To Back##") + idSuffix).c_str()))
        {
            changed |= moveMiniLocationToDrawOrderEdge(location, miniEditor_.selected, false);
        }
        ImGui::SameLine();
        if (ImGui::Button((std::string("Backward##") + idSuffix).c_str()))
        {
            changed |= moveMiniLocationDrawOrder(location, miniEditor_.selected, -1);
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!canMoveForward);
        if (ImGui::Button((std::string("Forward##") + idSuffix).c_str()))
        {
            changed |= moveMiniLocationDrawOrder(location, miniEditor_.selected, 1);
        }
        ImGui::SameLine();
        if (ImGui::Button((std::string("To Front##") + idSuffix).c_str()))
        {
            changed |= moveMiniLocationToDrawOrderEdge(location, miniEditor_.selected, true);
        }
        ImGui::EndDisabled();
    }

    void drawMiniLocationContentContextControls()
    {
        nlohmann::json* location = selectedMiniLocationForContent();
        if (location == nullptr)
        {
            return;
        }

        nlohmann::json* object = selectedMiniLocationContentObject(*location);
        if (object == nullptr)
        {
            return;
        }

        bool changed = false;
        const MiniLocationContentCollection collection = miniEditor_.selected.collection;
        drawMiniLocationContentDrawOrderControls(*location, "mini_context", changed);

        if (collection == MiniLocationContentCollection::Platforms)
        {
            ImGui::TextUnformatted("Platform");
            std::string type = object->value("Type", platformTypes_.empty() ? std::string{"Single-flat"} : platformTypes_.front());
            if (comboFromStrings("Type##mini_context", platformTypes_, type))
            {
                (*object)["Type"] = type;
                changed = true;
            }
            changed |= editVector2Field("Position##mini_context", *object, "Position");
            bool bounceEnabled = platformBounceEnabled(*object);
            if (ImGui::Checkbox("Bounce##mini_context", &bounceEnabled))
            {
                (*object)["BounceEnabled"] = bounceEnabled;
                changed = true;
            }
        }
        else if (collection == MiniLocationContentCollection::Decorations)
        {
            ImGui::TextUnformatted("Decoration");
            std::string name = object->value("Name", decorationOptions_.empty() ? std::string{"plant1"} : decorationOptions_.front());
            if (comboFromStrings("Name##mini_context", decorationOptions_, name))
            {
                (*object)["Name"] = name;
                changed = true;
            }
            changed |= editVector2Field("Position##mini_context", *object, "Position");
            changed |= editVector2Field("Scale##mini_context", *object, "Scale", {1.f, 1.f});
            float rotation = decorationRotation(*object);
            if (ImGui::InputFloat("Rotation##mini_context", &rotation, 1.f, 15.f, "%.2f"))
            {
                (*object)["Rotation"] = rotation;
                changed = true;
            }
            changed |= editParallaxFactorField("Parallax##mini_context", *object, {1.f, 1.f});
            changed |= editColorField("Color##mini_context", *object, "Color", sf::Color::White);
        }
        else if (collection == MiniLocationContentCollection::Interactives)
        {
            ImGui::TextUnformatted("Interactive / Sign");
            std::string type = object->value("Type", std::string{"EchoTablet"});
            if (comboFromStrings("Type##mini_context", interactiveTypeOptions_, type))
            {
                (*object)["Type"] = type;
                changed = true;
            }
            changed |= editVector2Field("Position##mini_context", *object, "Position");
            changed |= editVector2Field("Scale##mini_context", *object, "Scale", {1.f, 1.f});
            changed |= editStringField("Prompt##mini_context", *object, "Prompt", 256u);
            changed |= editStringField("Title##mini_context", *object, "Title", 256u);
            changed |= editMultilineStringField("Body##mini_context", *object, "Body", ImVec2(320.f, 96.f), 2048u);
        }
        else if (collection == MiniLocationContentCollection::Portals)
        {
            ImGui::TextUnformatted("Portal");
            changed |= editStringField("Id##mini_context", *object, "Id", 256u);
            changed |= editStringField("Title##mini_context", *object, "Title", 256u);
            changed |= editVector2Field("Position##mini_context", *object, "Position");
            changed |= editVector2Field("Scale##mini_context", *object, "Scale", {0.36f, 0.36f});
            std::string portalTexture = object->value("PortalTexture", object->value("Texture", std::string{"portalGreen"}));
            const std::vector<std::string> portalTextures{"portalGreen", "portalViolet"};
            if (comboFromStrings("Portal Texture##mini_context", portalTextures, portalTexture))
            {
                (*object)["PortalTexture"] = portalTexture;
                changed = true;
            }
            drawPortalTargetControls(*object, changed);
        }
        else if (collection == MiniLocationContentCollection::DeadAreas || collection == MiniLocationContentCollection::Barriers)
        {
            ImGui::TextUnformatted(collection == MiniLocationContentCollection::Barriers ? "Barrier" : "DeadArea");
            changed |= editStringField("Id##mini_context", *object, "Id", 256u);
            bool enabled = object->value("Enabled", true);
            if (ImGui::Checkbox("Enabled##mini_context", &enabled))
            {
                (*object)["Enabled"] = enabled;
                changed = true;
            }
            if (collection == MiniLocationContentCollection::Barriers)
            {
                bool blocksPlayer = object->value("BlocksPlayer", true);
                if (ImGui::Checkbox("Blocks Player##mini_context", &blocksPlayer))
                {
                    (*object)["BlocksPlayer"] = blocksPlayer;
                    changed = true;
                }
            }
            sf::FloatRect rect = readRect(object->value("Rect", nlohmann::json::array()));
            float raw[4]{rect.position.x, rect.position.y, rect.size.x, rect.size.y};
            if (ImGui::InputFloat4("Rect##mini_context", raw))
            {
                (*object)["Rect"] = nlohmann::json::array({raw[0], raw[1], std::max(raw[2], 1.f), std::max(raw[3], 1.f)});
                changed = true;
            }
        }

        if (changed)
        {
            markDirty();
        }
    }

    void drawMiniLocationContextMenu()
    {
        if (openMiniLocationContextMenu_)
        {
            ImGui::OpenPopup("mini_location_content_context_menu");
            openMiniLocationContextMenu_ = false;
        }

        ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);
        if (!ImGui::BeginPopup("mini_location_content_context_menu"))
        {
            return;
        }

        const bool hasSelection = miniEditor_.contentMode && miniEditor_.selected.isValid();
        const bool canDelete = hasSelection &&
            miniEditor_.selected.collection != MiniLocationContentCollection::SpawnPosition &&
            miniEditor_.selected.collection != MiniLocationContentCollection::Entry &&
            miniEditor_.selected.collection != MiniLocationContentCollection::Exit;
        const bool canCopy = hasSelection && miniLocationCollectionKey(miniEditor_.selected.collection)[0] != '\0';
        const bool canPaste = miniLocationClipboardCollection_ != MiniLocationContentCollection::None && !miniLocationClipboard_.is_null();

        if (hasSelection)
        {
            ImGui::TextUnformatted("Mini-location content");
        }
        else
        {
            ImGui::TextDisabled("No content under cursor");
        }
        ImGui::Separator();

        if (hasSelection)
        {
            drawMiniLocationContentContextControls();
            ImGui::Separator();
        }

        if (ImGui::MenuItem("Copy", "Ctrl+C", false, canCopy))
        {
            copyMiniLocationContentSelection();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Paste Here", "Ctrl+V", false, canPaste))
        {
            pasteMiniLocationContentAt(lastMouseWorldPosition_);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Duplicate Here", nullptr, false, canCopy))
        {
            if (copyMiniLocationContentSelection())
            {
                pasteMiniLocationContentAt(lastMouseWorldPosition_);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete", "Del", false, canDelete))
        {
            deleteMiniLocationContentSelection();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
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
        else if (selection_.kind == SelectionKind::Trader)
        {
            ImGui::TextUnformatted("Trader");
            ImGui::Separator();
            changed |= editVector2Field("Position##trader_context", document_["Presets"], "TraderPosition", {800.f, 940.f});
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

                bool bounceEnabled = platformBounceEnabled(*platform);
                if (ImGui::Checkbox("Bounce##platform_context", &bounceEnabled))
                {
                    (*platform)["BounceEnabled"] = bounceEnabled;
                    changed = true;
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
                float rotation = decorationRotation(*decoration);
                if (ImGui::InputFloat("Rotation##decoration_context", &rotation, 1.f, 15.f, "%.2f"))
                {
                    (*decoration)["Rotation"] = rotation;
                    changed = true;
                }
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
                    const nlohmann::json previousBackground = *background;
                    (*background)["BgName"] = bgName;
                    if (resolveBackgroundTilePlacement(*background, selection_.index))
                    {
                        changed = true;
                    }
                    else
                    {
                        *background = previousBackground;
                    }
                }

                changed |= editBackgroundTileField("Tile X/Y##background_context", *background, selection_.index);
                if (drawParallaxControls(
                    "Parallax##background_context",
                    "background_context",
                    *background,
                    backgroundDisplayPosition(*background),
                    {0.08f, 0.06f}
                ))
                {
                    resolveBackgroundTilePlacement(*background, selection_.index);
                    changed = true;
                }
                changed |= editStringField("Theme Override##background_context", *background, "Theme", 128u);

                int backgroundType = backgroundTypeComboIndex(*background);
                if (ImGui::Combo("Type##background_context", &backgroundType, "Single\0Repeated X\0Repeated Y\0Repeated XY\0"))
                {
                    const nlohmann::json previousBackground = *background;
                    setBackgroundTypeFromCombo(*background, backgroundType);
                    if (resolveBackgroundTilePlacement(*background, selection_.index))
                    {
                        changed = true;
                    }
                    else
                    {
                        *background = previousBackground;
                    }
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

                changed |= drawSpawnerAdvancedControls(*spawner, "spawner_context", false);

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
        const ImVec2 displaySize = editorDisplaySize();
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(displaySize.x, topVisibleHeight()), ImGuiCond_Always);
        if (!ImGui::Begin(
                "Dark Gate Editor",
                nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            ))
        {
            ImGui::End();
            return;
        }

        drawPanelBackdrop(ImVec2(420.f, 94.f));
        if (ImGui::ArrowButton("##top_drawer_toggle", topDrawerExpanded_ ? ImGuiDir_Up : ImGuiDir_Down))
        {
            topDrawerExpanded_ = !topDrawerExpanded_;
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.96f, 0.82f, 0.58f, 1.f), "Dark Gate Editor");
        ImGui::SameLine();
        ImGui::TextDisabled("workspace shell");

        if (topDrawerReveal_ < 0.12f)
        {
            ImGui::End();
            return;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, saturate((topDrawerReveal_ - 0.08f) / 0.92f));
        if (ImGui::BeginTable("toolbar_layout", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableNextColumn();
            if (ImGui::Button("Save", ImVec2(100.f, 0.f)))
            {
                saveLevel();
            }
            ImGui::SameLine();
            if (ImGui::Button("Play", ImVec2(100.f, 0.f)))
            {
                if (saveLevel())
                {
                    playCurrentLevel();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Reload", ImVec2(100.f, 0.f)))
            {
                refreshRegistry();
            }
            ImGui::SameLine();
            if (ImGui::Button("Spawn", ImVec2(100.f, 0.f)))
            {
                const sf::Vector2f spawn = readVector2f(document_["Presets"].value("PlayerSpawn", nlohmann::json::array()));
                worldView_.setCenter(spawn);
            }
            drawInfoLine("Document", currentFilePath_.empty() ? "Unsaved document" : currentFilePath_.filename().string());
            drawInfoLine("Status", dirty_ ? "Unsaved changes" : "All changes saved");

            ImGui::TableNextColumn();
            drawInfoLine("Mode", placementModeLabel());
            drawInfoLine(
                "Camera",
                std::to_string(static_cast<int>(std::lround(worldView_.getCenter().x))) + ", " +
                    std::to_string(static_cast<int>(std::lround(worldView_.getCenter().y)))
            );
            drawInfoLine("Selection", selectedObjectLabel());
            drawInfoLine("Workspace", editorTabLabel(activeTab_));
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::End();
    }

    void drawLevelsWindow()
    {
        const float width = currentSidebarWidth();
        const float y = panelTop();
        const float height = panelHeight();
        ImGui::SetNextWindowPos(ImVec2(0.f, y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        if (!ImGui::Begin(
                "Levels",
                nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            ))
        {
            ImGui::End();
            return;
        }

        drawPanelBackdrop(ImVec2(180.f, 138.f));
        drawDrawerCaption("##left_drawer_toggle", &leftDrawerExpanded_, "Levels", ImGuiDir_Left, ImGuiDir_Right);
        if (leftDrawerReveal_ < 0.16f)
        {
            ImGui::End();
            return;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, saturate((leftDrawerReveal_ - 0.10f) / 0.90f));
        if (ImGui::Button("New Level", ImVec2(140.f, 0.f)))
        {
            createDefaultLevel();
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh", ImVec2(120.f, 0.f)))
        {
            refreshRegistry();
        }

        ImGui::SeparatorText("Create Level");
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
        if (ImGui::Button("Start Blank Level", ImVec2(-1.f, 0.f)))
        {
            createDefaultLevel();
            document_["Presets"]["LevelId"] = std::string(levelIdBuffer_);
            document_["Presets"]["Title"] = std::string(levelTitleBuffer_);
            syncMetadataBuffers();
        }

        ImGui::SeparatorText("Level Library");
        ImGui::TextDisabled("%zu files in registry", registry_.getLevels().size());
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
                ImGui::Spacing();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::End();
    }

    void drawInspectorWindow()
    {
        const float width = currentInspectorWidth();
        const float x = editorDisplaySize().x - width;
        const float y = panelTop();
        const float height = panelHeight();
        ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        if (!ImGui::Begin(
                "Inspector",
                nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            ))
        {
            ImGui::End();
            return;
        }

        drawPanelBackdrop(ImVec2(260.f, 132.f));
        if (ImGui::ArrowButton("##right_drawer_toggle", rightDrawerExpanded_ ? ImGuiDir_Right : ImGuiDir_Left))
        {
            rightDrawerExpanded_ = !rightDrawerExpanded_;
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.96f, 0.82f, 0.58f, 1.f), "Inspector");
        ImGui::SameLine();
        ImGui::TextDisabled("details");

        if (rightDrawerReveal_ < 0.16f)
        {
            ImGui::End();
            return;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, saturate((rightDrawerReveal_ - 0.10f) / 0.90f));
        drawInfoLine("Selected", selectedObjectLabel());
        drawInfoLine("Mode", placementModeLabel());
        ImGui::Separator();

        if (ImGui::BeginChild("inspector_scroll"))
        {
            drawSectionSelector();

            if (selectedMiniLocationForContent() != nullptr)
            {
                drawMiniLocationsInspector();
            }
            else switch (activeTab_)
            {
            case EditorTab::Level:
                drawLevelInspector();
                break;
            case EditorTab::Platforms:
                drawPlatformsInspector();
                break;
            case EditorTab::Decorations:
                drawDecorationsInspector();
                break;
            case EditorTab::Backgrounds:
                drawBackgroundInspector();
                break;
            case EditorTab::Ground:
                drawGroundInspector();
                break;
            case EditorTab::Spawners:
                drawSpawnersInspector();
                break;
            case EditorTab::Portals:
                drawPortalsInspector();
                break;
            case EditorTab::Interactives:
                drawInteractivesInspector();
                break;
            case EditorTab::Hazards:
                drawHazardsInspector();
                break;
            case EditorTab::MiniLocations:
                drawMiniLocationsInspector();
                break;
            }
            ImGui::EndChild();
        }
        ImGui::PopStyleVar();

        ImGui::End();
    }

    void drawLevelInspector()
    {
        bool changed = false;
        ImGui::SeparatorText("Level Settings");
        changed |= ImGui::InputText("File Name", fileNameBuffer_, sizeof(fileNameBuffer_));
        changed |= ImGui::InputText("Level ID", levelIdBuffer_, sizeof(levelIdBuffer_));
        changed |= ImGui::InputText("Title", levelTitleBuffer_, sizeof(levelTitleBuffer_));

        auto& presets = document_["Presets"];
        int size[2]{presets["Size"][0].get<int>(), presets["Size"][1].get<int>()};
        if (ImGui::InputInt2("Level Size", size))
        {
            const int nextWidth = std::max(size[0], 640);
            const int nextHeight = std::max(size[1], 360);
            presets["Size"] = nlohmann::json::array({nextWidth, nextHeight});
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

        const bool isFirstLevel = presets.value("LevelId", std::string{}) == "level1.json";
        if (isFirstLevel)
        {
            sf::Vector2f traderPosition = readVector2f(presets.value("TraderPosition", nlohmann::json::array()), {800.f, 940.f});
            float traderRaw[2]{traderPosition.x, traderPosition.y};
            if (ImGui::InputFloat2("Trader Position", traderRaw))
            {
                presets["TraderPosition"] = nlohmann::json::array({traderRaw[0], traderRaw[1]});
                changed = true;
            }
        }
        else
        {
            ImGui::TextDisabled("Trader is only spawned on level1.json.");
        }

        std::string weatherTheme = presets.value("BackgroundTheme", std::string{"VerdantDawn"});
        if (comboFromStrings("Weather Theme", weatherThemeOptions_, weatherTheme))
        {
            presets["BackgroundTheme"] = weatherTheme;
            changed = true;
        }
        changed |= editStringField("Weather Id", presets, "BackgroundTheme", 128u);

        float backgroundOffsetY = presets.value("BackgroundTileOffsetY", 0.f);
        if (ImGui::InputFloat("Background Tile Y Offset", &backgroundOffsetY, 8.f, 32.f, "%.1f"))
        {
            presets["BackgroundTileOffsetY"] = backgroundOffsetY;
            changed = true;
        }
        ImGui::TextDisabled("Moves every 1920 x 1080 background tile vertically. Negative values lift backgrounds up.");

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
        ImGui::SameLine();
        if (ImGui::Button("Actors Layer"))
        {
            selection_ = {SelectionKind::Actors, 0u};
        }

        ImGui::SeparatorText("Actors Layer");
        ImGui::TextWrapped("Player, enemies, and player bullets are drawn together in this world layer.");
        int actorsLayer = sharedWorldDrawOrder(SelectionKind::Actors, 0u);
        if (ImGui::InputInt("Actors Layer Number", &actorsLayer))
        {
            setSharedWorldDrawOrder(SelectionKind::Actors, 0u, actorsLayer);
            changed = true;
        }

        if (changed)
        {
            markDirty();
        }

        if (selection_.kind == SelectionKind::Actors)
        {
            drawSelectedDrawOrderControls("actors_layer");
        }

        ImGui::SeparatorText("Tips");
        ImGui::TextWrapped("You can also drag the spawn marker directly with the mouse in the scene.");
    }

    void drawPlatformsInspector()
    {
        const std::vector<std::string> atmosphereOptions(kPlatformAtmosphereModeOptions.begin(), kPlatformAtmosphereModeOptions.end());

        ImGui::SeparatorText("Placement");
        if (!platformTypes_.empty())
        {
            drawPlacementCombo("Platform Type", platformTypes_, selectedPlatformTypeIndex_);
        }
        drawPlacementButtons(SelectionKind::Platform);
        ImGui::TextWrapped("In Select Mode: drag the platform sprite to move it, drag the blue handle to resize the sprite, and enable Edit hitbox in RMB if you want to move or resize the hitbox separately.");
        ImGui::SeparatorText("Scene Objects");
        drawPlatformList();

        if (selection_.kind == SelectionKind::Platform && selection_.index < document_["Platforms"].size())
        {
            ImGui::SeparatorText("Selected Platform");
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

            bool bounceEnabled = platformBounceEnabled(platform);
            if (ImGui::Checkbox("Bounce", &bounceEnabled))
            {
                platform["BounceEnabled"] = bounceEnabled;
                changed = true;
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
        ImGui::SeparatorText("Placement");
        if (!decorationOptions_.empty())
        {
            drawPlacementCombo("Decoration", decorationOptions_, selectedDecorationIndex_);
        }
        drawPlacementButtons(SelectionKind::Decoration);
        ImGui::TextWrapped("In Select Mode: drag a decoration to move it, or drag the green corner handle to resize it directly in the scene.");
        ImGui::SeparatorText("Scene Objects");
        drawDecorationList();

        if (selection_.kind == SelectionKind::Decoration && selection_.index < document_["Decorations"].size())
        {
            ImGui::SeparatorText("Selected Decoration");
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
            float rotation = decorationRotation(decoration);
            if (ImGui::InputFloat("Rotation", &rotation, 1.f, 15.f, "%.2f"))
            {
                decoration["Rotation"] = rotation;
                changed = true;
            }
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
        ImGui::SeparatorText("Placement");
        if (!backgroundOptions_.empty())
        {
            drawPlacementCombo("Background", backgroundOptions_, selectedBackgroundIndex_);
        }
        drawPlacementButtons(SelectionKind::Background);
        ImGui::TextWrapped("Background layers support parallax and draw-order tuning, so it is easier to stage depth right from the editor.");
        if (document_.contains("Presets") && document_["Presets"].is_object())
        {
            float backgroundOffsetY = document_["Presets"].value("BackgroundTileOffsetY", 0.f);
            if (ImGui::InputFloat("Tile Y Offset", &backgroundOffsetY, 8.f, 32.f, "%.1f"))
            {
                document_["Presets"]["BackgroundTileOffsetY"] = backgroundOffsetY;
                markDirty();
            }
            ImGui::TextDisabled("Negative values lift the whole background tile grid.");
        }
        ImGui::SeparatorText("Scene Objects");
        drawBackgroundList();

        if (selection_.kind == SelectionKind::Background && selection_.index < document_["Background"].size())
        {
            ImGui::SeparatorText("Selected Background");
            auto& background = document_["Background"][selection_.index];
            bool changed = false;
            std::string bgName = background.value("BgName", backgroundOptions_.empty() ? std::string{} : backgroundOptions_.front());
            if (comboFromStrings("Texture", backgroundOptions_, bgName))
            {
                const nlohmann::json previousBackground = background;
                background["BgName"] = bgName;
                if (resolveBackgroundTilePlacement(background, selection_.index))
                {
                    changed = true;
                }
                else
                {
                    background = previousBackground;
                }
            }
            changed |= editBackgroundTileField("Tile X/Y", background, selection_.index);
            if (drawParallaxControls(
                "Parallax",
                "background_inspector",
                background,
                backgroundDisplayPosition(background),
                {0.08f, 0.06f}
            ))
            {
                resolveBackgroundTilePlacement(background, selection_.index);
                changed = true;
            }
            changed |= editStringField("Theme Override", background, "Theme", 128u);

            int backgroundType = backgroundTypeComboIndex(background);
            if (ImGui::Combo("Type", &backgroundType, "Single\0Repeated X\0Repeated Y\0Repeated XY\0"))
            {
                const nlohmann::json previousBackground = background;
                setBackgroundTypeFromCombo(background, backgroundType);
                if (resolveBackgroundTilePlacement(background, selection_.index))
                {
                    changed = true;
                }
                else
                {
                    background = previousBackground;
                }
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
        ImGui::SeparatorText("Placement");
        if (ImGui::Button("Add Ground Strip"))
        {
            document_["Ground"].push_back({
                {"EditorDrawOrder", nextWorldDrawOrder()},
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
        ImGui::SeparatorText("Scene Objects");
        drawGroundList();

        if (selection_.kind == SelectionKind::Ground && selection_.index < document_["Ground"].size())
        {
            ImGui::SeparatorText("Selected Ground");
            auto& ground = document_["Ground"][selection_.index];
            bool changed = false;

            int groundLayer = sharedWorldDrawOrder(SelectionKind::Ground, selection_.index);
            if (ImGui::InputInt("Ground Layer", &groundLayer))
            {
                setSharedWorldDrawOrder(SelectionKind::Ground, selection_.index, groundLayer);
                changed = true;
            }

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
        ImGui::SeparatorText("Placement");
        if (!enemyTypeOptions_.empty())
        {
            drawPlacementCombo("Enemy", enemyTypeOptions_, selectedEnemyTypeIndex_);
        }
        drawPlacementButtons(SelectionKind::Spawner);
        ImGui::TextWrapped("In Select Mode: drag a spawner area to move it, or drag its corner handle to resize the spawn zone.");
        ImGui::SeparatorText("Scene Objects");
        drawSpawnerList();

        if (selection_.kind == SelectionKind::Spawner && selection_.index < document_["Spawners"].size())
        {
            ImGui::SeparatorText("Selected Spawner");
            auto& spawner = document_["Spawners"][selection_.index];
            bool changed = drawSpawnerAdvancedControls(spawner, "spawner_inspector", true);

            if (changed)
            {
                markDirty();
            }

            drawSelectedDrawOrderControls("spawner_inspector");
        }
    }

    void drawPortalTargetControls(nlohmann::json& portal, bool& changed)
    {
        if (!portal.contains("Target") || !portal["Target"].is_object())
        {
            portal["Target"] = {
                {"Type", "Position"},
                {"Position", portal.value("Position", nlohmann::json::array({0.f, 0.f}))}
            };
        }

        auto& target = portal["Target"];
        std::string targetType = target.value("Type", std::string{"Position"});
        const std::vector<std::string> targetTypes{"Position", "Level", "MiniLocation"};
        if (comboFromStrings("Target Type", targetTypes, targetType))
        {
            target["Type"] = targetType;
            if (targetType == "MiniLocation")
            {
                target.erase("Position");
                target.erase("SpawnPosition");
                target["MiniLocationId"] = firstMiniLocationId();
            }
            changed = true;
        }

        if (targetType == "Level")
        {
            std::vector<std::string> levelIds;
            for (const LevelDescriptor& descriptor : registry_.getLevels())
            {
                levelIds.push_back(descriptor.id);
            }
            if (levelIds.empty())
            {
                levelIds.push_back("level1.json");
            }
            std::string levelId = target.value("LevelId", levelIds.front());
            if (comboFromStrings("Level", levelIds, levelId))
            {
                target["LevelId"] = levelId;
                changed = true;
            }
            changed |= editStringField("Level Id", target, "LevelId", 256u);
            changed |= editVector2Field("Spawn Position", target, "SpawnPosition", readVector2f(document_["Presets"].value("PlayerSpawn", nlohmann::json::array()), {200.f, 900.f}));
        }
        else if (targetType == "MiniLocation")
        {
            std::vector<std::string> ids;
            for (const auto& location : document_["MiniLocations"])
            {
                ids.push_back(location.value("Id", location.value("Title", std::string{"Mini Location"})));
            }
            if (ids.empty())
            {
                ids.push_back("mini_location_1");
            }
            std::string miniLocationId = target.value("MiniLocationId", ids.front());
            if (std::find(ids.begin(), ids.end(), miniLocationId) == ids.end())
            {
                miniLocationId = ids.front();
                target["MiniLocationId"] = miniLocationId;
                changed = true;
            }
            target.erase("Position");
            if (comboFromStrings("Mini Location", ids, miniLocationId))
            {
                target["MiniLocationId"] = miniLocationId;
                target.erase("Position");
                changed = true;
            }
            changed |= editStringField("Mini Location Id", target, "MiniLocationId", 256u);

            bool overrideSpawn = target.contains("SpawnPosition") && target["SpawnPosition"].is_array();
            if (ImGui::Checkbox("Override Spawn Position", &overrideSpawn))
            {
                if (overrideSpawn)
                {
                    sf::Vector2f fallbackSpawn{0.f, 0.f};
                    for (const auto& location : document_["MiniLocations"])
                    {
                        if (location.value("Id", location.value("Title", std::string{})) == miniLocationId)
                        {
                            const sf::FloatRect bounds = miniLocationBounds(location);
                            fallbackSpawn = bounds.position + readVector2f(
                                location.value("SpawnPosition", nlohmann::json::array()),
                                {bounds.size.x * 0.5f, bounds.size.y - 42.f}
                            );
                            break;
                        }
                    }
                    target["SpawnPosition"] = toJson(fallbackSpawn);
                }
                else
                {
                    target.erase("SpawnPosition");
                }
                changed = true;
            }
            if (overrideSpawn)
            {
                changed |= editVector2Field("Spawn Position", target, "SpawnPosition", {0.f, 0.f});
            }
            else
            {
                ImGui::TextDisabled("Uses the selected mini-location Portal Spawn marker.");
            }
        }
        else
        {
            changed |= editVector2Field("Position", target, "Position", readVector2f(portal.value("Position", nlohmann::json::array())));
        }
    }

    void drawPortalsInspector()
    {
        ImGui::SeparatorText("Placement");
        drawPlacementButtons(SelectionKind::Portal);
        ImGui::TextWrapped("Static portals are world objects. Press Enter near one at runtime to teleport.");
        ImGui::SeparatorText("Scene Objects");
        drawPortalList();

        if (selection_.kind == SelectionKind::Portal && selection_.index < document_["Portals"].size())
        {
            ImGui::SeparatorText("Selected Portal");
            auto& portal = document_["Portals"][selection_.index];
            bool changed = false;
            changed |= editStringField("Id", portal, "Id", 256u);
            changed |= editStringField("Title", portal, "Title", 256u);
            changed |= editVector2Field("Position", portal, "Position");
            changed |= editVector2Field("Scale", portal, "Scale", {0.36f, 0.36f});
            std::string portalTexture = portal.value("PortalTexture", portal.value("Texture", std::string{"portalGreen"}));
            const std::vector<std::string> portalTextures{"portalGreen", "portalViolet"};
            if (comboFromStrings("Portal Texture", portalTextures, portalTexture))
            {
                portal["PortalTexture"] = portalTexture;
                changed = true;
            }
            changed |= editColorField("Color", portal, "Color", sf::Color(212, 236, 255, 245));
            changed |= editColorField("Accent Color", portal, "AccentColor", sf::Color(112, 208, 255, 255));

            float radius = portal.value("InteractRadius", 130.f);
            if (ImGui::InputFloat("Interact Radius", &radius))
            {
                portal["InteractRadius"] = std::max(radius, 0.f);
                changed = true;
            }

            sf::FloatRect activationBounds = portalActivationBounds(portal);
            float activationRaw[4]{
                activationBounds.position.x,
                activationBounds.position.y,
                activationBounds.size.x,
                activationBounds.size.y
            };
            if (ImGui::InputFloat4("Activation Area", activationRaw))
            {
                portal["ActivationArea"] = toJson(sf::FloatRect(
                    {activationRaw[0], activationRaw[1]},
                    {std::max(activationRaw[2], 1.f), std::max(activationRaw[3], 1.f)}
                ));
                changed = true;
            }
            changed |= editStringField("Prompt", portal, "Prompt", 256u);

            ImGui::SeparatorText("Target");
            drawPortalTargetControls(portal, changed);

            if (changed)
            {
                markDirty();
            }

            drawSelectedDrawOrderControls("portal_inspector");
        }
    }

    void ensureMiniLocationNestedArrays(nlohmann::json& location)
    {
        for (const char* key : {"Platforms", "Decorations", "Interactives", "Portals", "DeadAreas", "Barriers"})
        {
            if (!location.contains(key) || !location[key].is_array())
            {
                location[key] = nlohmann::json::array();
            }
        }

        if (location["DeadAreas"].empty() && location.contains("Hazard") && location["Hazard"].is_object())
        {
            nlohmann::json deadArea = location["Hazard"];
            const sf::FloatRect bounds = miniLocationBounds(location);
            sf::FloatRect rect = readRect(deadArea.value("Rect", nlohmann::json::array()));
            rect.position -= bounds.position;
            deadArea["Id"] = deadArea.value("Id", std::string{"legacy_hazard"});
            deadArea["Rect"] = toJson(rect);
            location["DeadAreas"].push_back(deadArea);
        }

        if (location["Barriers"].empty() && location.value("BarrierEnabled", true))
        {
            const sf::FloatRect bounds = miniLocationBounds(location);
            const float width = location.value("BarrierWidth", 22.f);
            const float top = -90.f;
            const float height = std::max(bounds.size.y + 180.f, 220.f);
            location["Barriers"].push_back({
                {"Id", "left_barrier"},
                {"Rect", {-18.f, top, width, height}},
                {"Enabled", true},
                {"BlocksPlayer", true},
                {"CoreColor", {130, 214, 184, 255}},
                {"GlowColor", {156, 238, 208, 255}}
            });
            location["Barriers"].push_back({
                {"Id", "right_barrier"},
                {"Rect", {bounds.size.x + 18.f, top, width, height}},
                {"Enabled", true},
                {"BlocksPlayer", true},
                {"CoreColor", {130, 214, 184, 255}},
                {"GlowColor", {156, 238, 208, 255}}
            });
        }

        const auto ensureIds = [&](const char* key, const char* prefix) {
            auto& array = location[key];
            for (std::size_t index = 0; index < array.size(); ++index)
            {
                if (!array[index].contains("EditorId") || !array[index]["EditorId"].is_string() ||
                    array[index].value("EditorId", std::string{}).empty())
                {
                    array[index]["EditorId"] = std::string(prefix) + "_" + std::to_string(index + 1u);
                }
            }
        };

        ensureIds("Platforms", "platform");
        ensureIds("Decorations", "decoration");
        ensureIds("Interactives", "interactive");
        ensureIds("Portals", "portal");
        ensureIds("DeadAreas", "dead_area");
        ensureIds("Barriers", "barrier");
    }

    void normalizeMiniLocations()
    {
        if (!document_.contains("MiniLocations") || !document_["MiniLocations"].is_array())
        {
            document_["MiniLocations"] = nlohmann::json::array();
        }

        for (std::size_t index = 0; index < document_["MiniLocations"].size(); ++index)
        {
            auto& location = document_["MiniLocations"][index];
            if (!location.contains("Id") || !location["Id"].is_string() || location.value("Id", std::string{}).empty())
            {
                location["Id"] = "mini_location_" + std::to_string(index + 1u);
            }
            if (!location.contains("Bounds") || !location["Bounds"].is_array())
            {
                location["Bounds"] = {0.f, 0.f, 920.f, 340.f};
            }
            const sf::FloatRect bounds = miniLocationBounds(location);
            if (!location.contains("SpawnPosition") || !location["SpawnPosition"].is_array())
            {
                location["SpawnPosition"] = {bounds.size.x * 0.5f, bounds.size.y - 42.f};
            }
            if (!location.contains("Entry") || !location["Entry"].is_object())
            {
                location["Entry"] = nlohmann::json::object();
            }
            if (!location.contains("Exit") || !location["Exit"].is_object())
            {
                location["Exit"] = nlohmann::json::object();
            }
            ensureMiniLocationNestedArrays(location);
            location.erase("Hazard");
        }
    }

    void drawMiniLocationContentEditor(nlohmann::json& location)
    {
        ensureMiniLocationNestedArrays(location);
        const sf::FloatRect bounds = miniLocationBounds(location);
        const sf::Vector2f center{bounds.size.x * 0.5f, bounds.size.y * 0.5f};

        ImGui::SeparatorText("Nested Content");
        ImGui::TextDisabled("Nested object positions are relative to the room top-left.");

        const auto toolButton = [&](const char* label, const MiniLocationTool tool) {
            const bool active = miniEditor_.tool == tool;
            if (active)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.38f, 0.46f, 0.36f, 1.f));
            }
            if (ImGui::SmallButton(label))
            {
                miniEditor_.tool = tool;
            }
            if (active)
            {
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();
        };
        toolButton("Select/Move", MiniLocationTool::SelectMove);
        toolButton("Platform", MiniLocationTool::Platform);
        toolButton("Decoration", MiniLocationTool::Decoration);
        toolButton("Interactive", MiniLocationTool::Interactive);
        toolButton("Portal", MiniLocationTool::Portal);
        ImGui::TextDisabled("Hazards:");
        ImGui::SameLine();
        toolButton("DeadArea", MiniLocationTool::DeadArea);
        toolButton("Barrier", MiniLocationTool::Barrier);
        toolButton("Spawn", MiniLocationTool::Spawn);
        ImGui::NewLine();

        if (miniEditor_.selected.isValid() && miniEditor_.selected.locationIndex == selection_.index)
        {
            ImGui::SeparatorText("Selected Content");
            const MiniLocationContentCollection collection = miniEditor_.selected.collection;
            const char* key = miniLocationCollectionKey(collection);
            bool changed = false;

            if (collection == MiniLocationContentCollection::SpawnPosition)
            {
                changed |= editVector2Field("Relative Position", location, "SpawnPosition", center);
            }
            else if (collection == MiniLocationContentCollection::Entry || collection == MiniLocationContentCollection::Exit)
            {
                auto& endpoint = collection == MiniLocationContentCollection::Entry ? location["Entry"] : location["Exit"];
                changed |= editVector2Field("Position", endpoint, "Position");
                changed |= editVector2Field("Scale", endpoint, "Scale", {0.22f, 0.33f});
                changed |= editVector2Field("Destination Support", endpoint, "DestinationSupport");
                changed |= editStringField("Prompt", endpoint, "Prompt", 256u);
            }
            else if (key[0] != '\0' && location.contains(key) && location[key].is_array() && miniEditor_.selected.index < location[key].size())
            {
                auto& object = location[key][miniEditor_.selected.index];
                changed |= editStringField("EditorId", object, "EditorId", 256u);
                if (miniLocationCollectionUsesDrawOrder(collection))
                {
                    const std::vector<MiniLocationObjectRef> drawEntries = miniLocationDrawOrderEntries(selection_.index);
                    const auto currentEntryIt = std::find_if(drawEntries.begin(), drawEntries.end(), [&](const MiniLocationObjectRef& entry) {
                        return entry.sameObject(miniEditor_.selected);
                    });
                    const std::size_t currentLayerIndex = currentEntryIt == drawEntries.end()
                        ? 0u
                        : static_cast<std::size_t>(std::distance(drawEntries.begin(), currentEntryIt));
                    int layer = miniLocationDrawOrder(location, collection, miniEditor_.selected.index);

                    ImGui::SeparatorText("Draw Order");
                    ImGui::TextDisabled(
                        "Shared world layer after room expansion. Lower number is drawn earlier. Current sorted position %zu of %zu.",
                        currentEntryIt == drawEntries.end() ? 0u : currentLayerIndex + 1u,
                        drawEntries.size()
                    );
                    if (ImGui::InputInt("Layer Number##mini_content_layer", &layer))
                    {
                        object["EditorDrawOrder"] = layer;
                        changed = true;
                    }

                    const bool canMoveBackward = currentEntryIt != drawEntries.end() && currentLayerIndex > 0u;
                    const bool canMoveForward = currentEntryIt != drawEntries.end() && currentLayerIndex + 1u < drawEntries.size();
                    ImGui::BeginDisabled(!canMoveBackward);
                    if (ImGui::Button("To Back##mini_content_layer"))
                    {
                        changed |= moveMiniLocationToDrawOrderEdge(location, miniEditor_.selected, false);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Backward##mini_content_layer"))
                    {
                        changed |= moveMiniLocationDrawOrder(location, miniEditor_.selected, -1);
                    }
                    ImGui::EndDisabled();

                    ImGui::BeginDisabled(!canMoveForward);
                    if (ImGui::Button("Forward##mini_content_layer"))
                    {
                        changed |= moveMiniLocationDrawOrder(location, miniEditor_.selected, 1);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("To Front##mini_content_layer"))
                    {
                        changed |= moveMiniLocationToDrawOrderEdge(location, miniEditor_.selected, true);
                    }
                    ImGui::EndDisabled();
                }
                if (collection == MiniLocationContentCollection::Platforms)
                {
                    std::string type = object.value("Type", platformTypes_.empty() ? std::string{"Single-flat"} : platformTypes_.front());
                    if (comboFromStrings("Type", platformTypes_, type))
                    {
                        object["Type"] = type;
                        changed = true;
                    }
                    changed |= editVector2Field("Relative Position", object, "Position", center);
                    bool bounceEnabled = platformBounceEnabled(object);
                    if (ImGui::Checkbox("Bounce", &bounceEnabled))
                    {
                        object["BounceEnabled"] = bounceEnabled;
                        changed = true;
                    }
                }
                else if (collection == MiniLocationContentCollection::Decorations)
                {
                    std::string name = object.value("Name", decorationOptions_.empty() ? std::string{"plant1"} : decorationOptions_.front());
                    if (comboFromStrings("Texture", decorationOptions_, name))
                    {
                        object["Name"] = name;
                        changed = true;
                    }
                    changed |= editVector2Field("Relative Position", object, "Position", center);
                    changed |= editVector2Field("Scale", object, "Scale", {1.f, 1.f});
                    float rotation = decorationRotation(object);
                    if (ImGui::InputFloat("Rotation", &rotation, 1.f, 15.f, "%.2f"))
                    {
                        object["Rotation"] = rotation;
                        changed = true;
                    }
                    changed |= editParallaxFactorField("Parallax", object, {1.f, 1.f});
                    changed |= editColorField("Color", object, "Color", sf::Color::White);
                }
                else if (collection == MiniLocationContentCollection::Interactives)
                {
                    std::string type = object.value("Type", std::string{"EchoTablet"});
                    if (comboFromStrings("Type", interactiveTypeOptions_, type))
                    {
                        object["Type"] = type;
                        changed = true;
                    }
                    changed |= editVector2Field("Relative Position", object, "Position", center);
                    changed |= editVector2Field("Scale", object, "Scale", {1.f, 1.f});
                    std::string texture = object.value("Texture", previewTextureOptions_.empty() ? std::string{} : previewTextureOptions_.front());
                    if (comboFromStrings("Texture", previewTextureOptions_, texture))
                    {
                        object["Texture"] = texture;
                        changed = true;
                    }
                    changed |= editColorField("Color", object, "Color", sf::Color(255, 255, 255, 255));
                    changed |= editColorField("Accent", object, "AccentColor", sf::Color(220, 184, 122, 255));
                    float interactRadius = object.value("InteractRadius", 120.f);
                    if (ImGui::InputFloat("Interact Radius", &interactRadius, 1.f, 20.f, "%.1f"))
                    {
                        object["InteractRadius"] = std::max(0.f, interactRadius);
                        changed = true;
                    }
                    int rewardGold = object.value("RewardGold", 0);
                    if (ImGui::InputInt("Reward Gold", &rewardGold))
                    {
                        object["RewardGold"] = std::max(0, rewardGold);
                        changed = true;
                    }
                    bool singleUse = object.value("SingleUse", true);
                    if (ImGui::Checkbox("Single Use", &singleUse))
                    {
                        object["SingleUse"] = singleUse;
                        changed = true;
                    }
                    changed |= editStringField("Title", object, "Title", 256u);
                    changed |= editStringField("Prompt", object, "Prompt", 256u);
                    changed |= editMultilineStringField("Body", object, "Body", ImVec2(-1.f, 64.f), 2048u);
                }
                else if (collection == MiniLocationContentCollection::Portals)
                {
                    changed |= editStringField("Id", object, "Id", 256u);
                    changed |= editStringField("Title", object, "Title", 256u);
                    changed |= editVector2Field("Relative Position", object, "Position", center);
                    changed |= editVector2Field("Scale", object, "Scale", {0.36f, 0.36f});
                    std::string portalTexture = object.value("PortalTexture", object.value("Texture", std::string{"portalGreen"}));
                    const std::vector<std::string> portalTextures{"portalGreen", "portalViolet"};
                    if (comboFromStrings("Portal Texture", portalTextures, portalTexture))
                    {
                        object["PortalTexture"] = portalTexture;
                        changed = true;
                    }
                    changed |= editColorField("Color", object, "Color", sf::Color(212, 236, 255, 245));
                    changed |= editColorField("Accent", object, "AccentColor", sf::Color(112, 208, 255, 255));
                    float interactRadius = object.value("InteractRadius", 130.f);
                    if (ImGui::InputFloat("Interact Radius", &interactRadius, 1.f, 20.f, "%.1f"))
                    {
                        object["InteractRadius"] = std::max(0.f, interactRadius);
                        changed = true;
                    }
                    changed |= editStringField("Prompt", object, "Prompt", 256u);
                    drawPortalTargetControls(object, changed);
                }
                else if (collection == MiniLocationContentCollection::DeadAreas || collection == MiniLocationContentCollection::Barriers)
                {
                    changed |= editStringField("Id", object, "Id", 256u);
                    bool enabled = object.value("Enabled", true);
                    if (ImGui::Checkbox("Enabled", &enabled))
                    {
                        object["Enabled"] = enabled;
                        changed = true;
                    }
                    sf::FloatRect rect = readRect(object.value("Rect", nlohmann::json::array()), sf::FloatRect({center.x - 120.f, center.y - 24.f}, {240.f, 48.f}));
                    float raw[4]{rect.position.x, rect.position.y, rect.size.x, rect.size.y};
                    if (ImGui::InputFloat4("Relative Rect", raw))
                    {
                        object["Rect"] = nlohmann::json::array({raw[0], raw[1], std::max(raw[2], 1.f), std::max(raw[3], 1.f)});
                        changed = true;
                    }
                    if (collection == MiniLocationContentCollection::Barriers)
                    {
                        bool blocksPlayer = object.value("BlocksPlayer", true);
                        if (ImGui::Checkbox("Blocks Player", &blocksPlayer))
                        {
                            object["BlocksPlayer"] = blocksPlayer;
                            changed = true;
                        }
                        changed |= editColorField("Core", object, "CoreColor", sf::Color(130, 214, 184, 255));
                        changed |= editColorField("Glow", object, "GlowColor", sf::Color(156, 238, 208, 255));
                    }
                    else
                    {
                        changed |= editColorField("Core", object, "CoreColor", sf::Color(242, 104, 56, 255));
                        changed |= editColorField("Glow", object, "GlowColor", sf::Color(255, 182, 96, 255));
                        changed |= editColorField("Ember", object, "EmberColor", sf::Color(255, 236, 188, 255));
                    }
                }

                ImGui::TextDisabled("Absolute preview: %s", formatPositionLabel(miniLocationContentBounds(miniEditor_.selected).position).c_str());
                if (ImGui::SmallButton("Duplicate"))
                {
                    location[key].insert(location[key].begin() + static_cast<nlohmann::json::difference_type>(miniEditor_.selected.index + 1u), object);
                    location[key][miniEditor_.selected.index + 1u]["EditorId"] =
                        std::string(key) + "_" + std::to_string(location[key].size());
                    miniEditor_.selected.index += 1u;
                    changed = true;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Delete"))
                {
                    deleteMiniLocationContentSelection();
                    changed = false;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Back") && miniEditor_.selected.index > 0u)
                {
                    std::swap(location[key][miniEditor_.selected.index], location[key][miniEditor_.selected.index - 1u]);
                    miniEditor_.selected.index -= 1u;
                    changed = true;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Forward") && miniEditor_.selected.index + 1u < location[key].size())
                {
                    std::swap(location[key][miniEditor_.selected.index], location[key][miniEditor_.selected.index + 1u]);
                    miniEditor_.selected.index += 1u;
                    changed = true;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("To Center"))
                {
                    if (collection == MiniLocationContentCollection::DeadAreas || collection == MiniLocationContentCollection::Barriers)
                    {
                        sf::FloatRect rect = readRect(object.value("Rect", nlohmann::json::array()), sf::FloatRect({0.f, 0.f}, {240.f, 48.f}));
                        rect.position = center - rect.size * 0.5f;
                        object["Rect"] = toJson(rect);
                    }
                    else
                    {
                        object["Position"] = toJson(center);
                    }
                    changed = true;
                }
            }

            if (changed)
            {
                markDirty();
            }
        }

        if (ImGui::Button("Add Platform"))
        {
            const std::string typeName = platformTypes_.empty() ? "Single-flat" : platformTypes_.front();
            location["Platforms"].push_back({
                {"Type", typeName},
                {"Position", toJson(center)},
                {"EditorDrawOrder", nextMiniLocationDrawOrder(location)}
            });
            markDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Decoration"))
        {
            const std::string name = decorationOptions_.empty() ? "plant1" : decorationOptions_.front();
            location["Decorations"].push_back({
                {"Name", name},
                {"Position", toJson(center)},
                {"Scale", {1.f, 1.f}},
                {"Rotation", 0.f},
                {"Color", {255, 255, 255, 255}},
                {"ParallaxFactor", {1.f, 1.f}},
                {"Z", 0},
                {"EditorDrawOrder", nextMiniLocationDrawOrder(location)}
            });
            markDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Interactive"))
        {
            location["Interactives"].push_back({
                {"Type", "EchoTablet"},
                {"Texture", previewTextureOptions_.empty() ? std::string{} : previewTextureOptions_.front()},
                {"Position", toJson(center)},
                {"Scale", {1.f, 1.f}},
                {"Color", {255, 255, 255, 255}},
                {"AccentColor", {220, 184, 122, 255}},
                {"InteractRadius", 120.f},
                {"RewardGold", 0},
                {"SingleUse", true},
                {"Prompt", "Enter to interact"},
                {"Title", "Pocket Relic"},
                {"Body", "The pocket space keeps this memory close."},
                {"EditorDrawOrder", nextMiniLocationDrawOrder(location)}
            });
            markDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Portal"))
        {
            const int portalIndex = static_cast<int>(location["Portals"].size()) + 1;
            location["Portals"].push_back({
                {"Id", "pocket_portal_" + std::to_string(portalIndex)},
                {"Title", "Pocket Portal " + std::to_string(portalIndex)},
                {"Position", toJson(center)},
                {"Scale", {0.36f, 0.36f}},
                {"PortalTexture", "portalGreen"},
                {"Color", {212, 236, 255, 245}},
                {"AccentColor", {112, 208, 255, 255}},
                {"InteractRadius", 130.f},
                {"Prompt", "Enter portal"},
                {"Target", {{"Type", "Position"}, {"Position", toJson(center)}}},
                {"EditorDrawOrder", nextMiniLocationDrawOrder(location)}
            });
            markDirty();
        }

        ImGui::SeparatorText("Hazards");
        if (ImGui::Button("Add DeadArea"))
        {
            const int deadAreaIndex = static_cast<int>(location["DeadAreas"].size()) + 1;
            location["DeadAreas"].push_back({
                {"Id", "dead_area_" + std::to_string(deadAreaIndex)},
                {"Enabled", true},
                {"Rect", {center.x - 120.f, center.y - 24.f, 240.f, 48.f}},
                {"CoreColor", {242, 104, 56, 255}},
                {"GlowColor", {255, 182, 96, 255}},
                {"EmberColor", {255, 236, 188, 255}}
            });
            selectMiniLocationContent(MiniLocationContentCollection::DeadAreas, location["DeadAreas"].size() - 1u);
            markDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Barrier"))
        {
            const int barrierIndex = static_cast<int>(location["Barriers"].size()) + 1;
            location["Barriers"].push_back({
                {"Id", "barrier_" + std::to_string(barrierIndex)},
                {"Enabled", true},
                {"BlocksPlayer", true},
                {"Rect", {center.x - 12.f, center.y - 110.f, 24.f, 220.f}},
                {"CoreColor", {130, 214, 184, 255}},
                {"GlowColor", {156, 238, 208, 255}}
            });
            selectMiniLocationContent(MiniLocationContentCollection::Barriers, location["Barriers"].size() - 1u);
            markDirty();
        }

        const auto drawCountAndClear = [&](const char* label, const char* key) {
            ImGui::Text("%s: %zu", label, location[key].size());
            ImGui::SameLine();
            const std::string button = std::string("Clear##") + key;
            if (ImGui::SmallButton(button.c_str()))
            {
                location[key] = nlohmann::json::array();
                markDirty();
            }
        };

        drawCountAndClear("Platforms", "Platforms");
        drawCountAndClear("Decorations", "Decorations");
        drawCountAndClear("Interactives", "Interactives");
        drawCountAndClear("Portals", "Portals");
        ImGui::SeparatorText("Hazards");
        drawCountAndClear("DeadAreas", "DeadAreas");
        drawCountAndClear("Barriers", "Barriers");

        ImGui::SeparatorText("Content Outliner");
        if (ImGui::Selectable("Portal Spawn", miniEditor_.selected.collection == MiniLocationContentCollection::SpawnPosition))
        {
            selectMiniLocationContent(MiniLocationContentCollection::SpawnPosition);
        }
        if (ImGui::Selectable("Entry", miniEditor_.selected.collection == MiniLocationContentCollection::Entry))
        {
            selectMiniLocationContent(MiniLocationContentCollection::Entry);
        }
        if (ImGui::Selectable("Exit", miniEditor_.selected.collection == MiniLocationContentCollection::Exit))
        {
            selectMiniLocationContent(MiniLocationContentCollection::Exit);
        }

        const auto drawOutlinerCollection = [&](const char* label, const char* key, const MiniLocationContentCollection collection) {
            if (!ImGui::TreeNode(label))
            {
                return;
            }
            for (std::size_t index = 0; index < location[key].size(); ++index)
            {
                const auto& item = location[key][index];
                const std::string text = std::to_string(index + 1u) + ". " +
                    item.value("EditorId", item.value("Id", item.value("Title", std::string{"item"})));
                const bool isSelected = miniEditor_.selected.collection == collection &&
                    miniEditor_.selected.locationIndex == selection_.index &&
                    miniEditor_.selected.index == index;
                if (ImGui::Selectable(text.c_str(), isSelected))
                {
                    selectMiniLocationContent(collection, index);
                    const sf::FloatRect itemBounds = miniLocationContentBounds(miniEditor_.selected);
                    if (itemBounds.size.x > 0.f && itemBounds.size.y > 0.f)
                    {
                        worldView_.setCenter(itemBounds.getCenter());
                    }
                }
            }
            ImGui::TreePop();
        };
        drawOutlinerCollection("Platforms", "Platforms", MiniLocationContentCollection::Platforms);
        drawOutlinerCollection("Decorations", "Decorations", MiniLocationContentCollection::Decorations);
        drawOutlinerCollection("Interactives", "Interactives", MiniLocationContentCollection::Interactives);
        drawOutlinerCollection("Portals", "Portals", MiniLocationContentCollection::Portals);
        if (ImGui::TreeNode("Hazards"))
        {
            drawOutlinerCollection("DeadAreas", "DeadAreas", MiniLocationContentCollection::DeadAreas);
            drawOutlinerCollection("Barriers", "Barriers", MiniLocationContentCollection::Barriers);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Nested Platforms"))
        {
            for (std::size_t index = 0; index < location["Platforms"].size(); ++index)
            {
                ImGui::PushID(static_cast<int>(index));
                auto& platform = location["Platforms"][index];
                std::string type = platform.value("Type", platformTypes_.empty() ? std::string{"Single-flat"} : platformTypes_.front());
                bool changed = comboFromStrings("Type", platformTypes_, type);
                if (changed)
                {
                    platform["Type"] = type;
                }
                changed |= editVector2Field("Position", platform, "Position", center);
                if (ImGui::SmallButton("Delete"))
                {
                    location["Platforms"].erase(location["Platforms"].begin() + static_cast<nlohmann::json::difference_type>(index));
                    markDirty();
                    ImGui::PopID();
                    break;
                }
                if (changed)
                {
                    markDirty();
                }
                ImGui::Separator();
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Nested Decorations"))
        {
            for (std::size_t index = 0; index < location["Decorations"].size(); ++index)
            {
                ImGui::PushID(static_cast<int>(index));
                auto& decoration = location["Decorations"][index];
                std::string name = decoration.value("Name", decorationOptions_.empty() ? std::string{"plant1"} : decorationOptions_.front());
                bool changed = comboFromStrings("Name", decorationOptions_, name);
                if (changed)
                {
                    decoration["Name"] = name;
                }
                changed |= editVector2Field("Position", decoration, "Position", center);
                changed |= editVector2Field("Scale", decoration, "Scale", {1.f, 1.f});
                float rotation = decorationRotation(decoration);
                if (ImGui::InputFloat("Rotation", &rotation, 1.f, 15.f, "%.2f"))
                {
                    decoration["Rotation"] = rotation;
                    changed = true;
                }
                changed |= editColorField("Color", decoration, "Color", sf::Color::White);
                if (ImGui::SmallButton("Delete"))
                {
                    location["Decorations"].erase(location["Decorations"].begin() + static_cast<nlohmann::json::difference_type>(index));
                    markDirty();
                    ImGui::PopID();
                    break;
                }
                if (changed)
                {
                    markDirty();
                }
                ImGui::Separator();
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Nested Interactives"))
        {
            for (std::size_t index = 0; index < location["Interactives"].size(); ++index)
            {
                ImGui::PushID(static_cast<int>(index));
                auto& interactive = location["Interactives"][index];
                bool changed = false;
                std::string type = interactive.value("Type", std::string{"EchoTablet"});
                if (comboFromStrings("Type", interactiveTypeOptions_, type))
                {
                    interactive["Type"] = type;
                    changed = true;
                }
                changed |= editStringField("Title", interactive, "Title", 256u);
                changed |= editVector2Field("Position", interactive, "Position", center);
                changed |= editVector2Field("Scale", interactive, "Scale", {1.f, 1.f});
                changed |= editStringField("Prompt", interactive, "Prompt", 256u);
                if (ImGui::SmallButton("Delete"))
                {
                    location["Interactives"].erase(location["Interactives"].begin() + static_cast<nlohmann::json::difference_type>(index));
                    markDirty();
                    ImGui::PopID();
                    break;
                }
                if (changed)
                {
                    markDirty();
                }
                ImGui::Separator();
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Nested Portals"))
        {
            for (std::size_t index = 0; index < location["Portals"].size(); ++index)
            {
                ImGui::PushID(static_cast<int>(index));
                auto& portal = location["Portals"][index];
                bool changed = false;
                changed |= editStringField("Id", portal, "Id", 256u);
                changed |= editStringField("Title", portal, "Title", 256u);
                changed |= editVector2Field("Position", portal, "Position", center);
                changed |= editVector2Field("Scale", portal, "Scale", {0.36f, 0.36f});
                std::string portalTexture = portal.value("PortalTexture", portal.value("Texture", std::string{"portalGreen"}));
                const std::vector<std::string> portalTextures{"portalGreen", "portalViolet"};
                if (comboFromStrings("Portal Texture", portalTextures, portalTexture))
                {
                    portal["PortalTexture"] = portalTexture;
                    changed = true;
                }
                drawPortalTargetControls(portal, changed);
                if (ImGui::SmallButton("Delete"))
                {
                    location["Portals"].erase(location["Portals"].begin() + static_cast<nlohmann::json::difference_type>(index));
                    markDirty();
                    ImGui::PopID();
                    break;
                }
                if (changed)
                {
                    markDirty();
                }
                ImGui::Separator();
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Hazards Details"))
        {
            ImGui::SeparatorText("DeadAreas");
            for (std::size_t index = 0; index < location["DeadAreas"].size(); ++index)
            {
                ImGui::PushID(static_cast<int>(index));
                auto& deadArea = location["DeadAreas"][index];
                bool changed = false;
                changed |= editStringField("Id", deadArea, "Id", 256u);
                bool enabled = deadArea.value("Enabled", true);
                if (ImGui::Checkbox("Enabled", &enabled))
                {
                    deadArea["Enabled"] = enabled;
                    changed = true;
                }
                sf::FloatRect rect = readRect(deadArea.value("Rect", nlohmann::json::array()), sf::FloatRect({center.x - 120.f, center.y - 24.f}, {240.f, 48.f}));
                float raw[4]{rect.position.x, rect.position.y, rect.size.x, rect.size.y};
                if (ImGui::InputFloat4("Rect", raw))
                {
                    deadArea["Rect"] = nlohmann::json::array({raw[0], raw[1], std::max(raw[2], 1.f), std::max(raw[3], 1.f)});
                    changed = true;
                }
                changed |= editColorField("Core", deadArea, "CoreColor", sf::Color(242, 104, 56, 255));
                changed |= editColorField("Glow", deadArea, "GlowColor", sf::Color(255, 182, 96, 255));
                changed |= editColorField("Ember", deadArea, "EmberColor", sf::Color(255, 236, 188, 255));
                if (ImGui::SmallButton("Select"))
                {
                    selectMiniLocationContent(MiniLocationContentCollection::DeadAreas, index);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Delete"))
                {
                    location["DeadAreas"].erase(location["DeadAreas"].begin() + static_cast<nlohmann::json::difference_type>(index));
                    markDirty();
                    ImGui::PopID();
                    break;
                }
                if (changed)
                {
                    markDirty();
                }
                ImGui::Separator();
                ImGui::PopID();
            }

            ImGui::SeparatorText("Barriers");
            for (std::size_t index = 0; index < location["Barriers"].size(); ++index)
            {
                ImGui::PushID(static_cast<int>(index + 10000u));
                auto& barrier = location["Barriers"][index];
                bool changed = false;
                changed |= editStringField("Id", barrier, "Id", 256u);
                bool enabled = barrier.value("Enabled", true);
                if (ImGui::Checkbox("Enabled", &enabled))
                {
                    barrier["Enabled"] = enabled;
                    changed = true;
                }
                bool blocksPlayer = barrier.value("BlocksPlayer", true);
                if (ImGui::Checkbox("Blocks Player", &blocksPlayer))
                {
                    barrier["BlocksPlayer"] = blocksPlayer;
                    changed = true;
                }
                sf::FloatRect rect = readRect(barrier.value("Rect", nlohmann::json::array()), sf::FloatRect({center.x - 12.f, center.y - 110.f}, {24.f, 220.f}));
                float raw[4]{rect.position.x, rect.position.y, rect.size.x, rect.size.y};
                if (ImGui::InputFloat4("Rect", raw))
                {
                    barrier["Rect"] = nlohmann::json::array({raw[0], raw[1], std::max(raw[2], 1.f), std::max(raw[3], 1.f)});
                    changed = true;
                }
                changed |= editColorField("Core", barrier, "CoreColor", sf::Color(130, 214, 184, 255));
                changed |= editColorField("Glow", barrier, "GlowColor", sf::Color(156, 238, 208, 255));
                if (ImGui::SmallButton("Select"))
                {
                    selectMiniLocationContent(MiniLocationContentCollection::Barriers, index);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Delete"))
                {
                    location["Barriers"].erase(location["Barriers"].begin() + static_cast<nlohmann::json::difference_type>(index));
                    markDirty();
                    ImGui::PopID();
                    break;
                }
                if (changed)
                {
                    markDirty();
                }
                ImGui::Separator();
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }

    void drawInteractivesInspector()
    {
        ImGui::SeparatorText("Placement");
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
        ImGui::SeparatorText("Scene Objects");
        drawInteractiveList();

        if (selection_.kind == SelectionKind::Interactive && selection_.index < document_["Interactives"].size())
        {
            ImGui::SeparatorText("Selected Interactive");
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

    void drawHazardRectEditor(nlohmann::json& hazard, const char* label, const sf::Color coreFallback, const sf::Color glowFallback)
    {
        bool changed = false;
        changed |= editStringField("Id", hazard, "Id", 256u);

        bool enabled = hazard.value("Enabled", true);
        if (ImGui::Checkbox("Enabled", &enabled))
        {
            hazard["Enabled"] = enabled;
            changed = true;
        }

        sf::FloatRect rect = worldHazardBounds(hazard);
        float raw[4]{rect.position.x, rect.position.y, rect.size.x, rect.size.y};
        if (ImGui::InputFloat4("Rect X/Y/W/H", raw))
        {
            hazard["Rect"] = toJson(sf::FloatRect({raw[0], raw[1]}, {std::max(raw[2], 16.f), std::max(raw[3], 16.f)}));
            changed = true;
        }

        changed |= editColorField("Core Color", hazard, "CoreColor", coreFallback);
        changed |= editColorField("Glow Color", hazard, "GlowColor", glowFallback);

        if (std::strcmp(label, "Barrier") == 0)
        {
            bool blocksPlayer = hazard.value("BlocksPlayer", true);
            if (ImGui::Checkbox("Blocks Player", &blocksPlayer))
            {
                hazard["BlocksPlayer"] = blocksPlayer;
                changed = true;
            }
        }
        else
        {
            changed |= editColorField("Ember Color", hazard, "EmberColor", sf::Color(255, 236, 188, 255));
        }

        if (changed)
        {
            markDirty();
        }
    }

    void drawHazardsInspector()
    {
        ImGui::SeparatorText("Placement");
        const float buttonWidth = (ImGui::GetContentRegionAvail().x - 8.f) / 2.f;
        if (ImGui::Button("Place DeadArea", ImVec2(buttonWidth, 0.f)))
        {
            placementMode_ = SelectionKind::DeadArea;
            placingPlatformHitbox_ = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Place Barrier", ImVec2(buttonWidth, 0.f)))
        {
            placementMode_ = SelectionKind::Barrier;
            placingPlatformHitbox_ = false;
        }
        if (ImGui::Button("Place Platform Hitbox", ImVec2(buttonWidth, 0.f)))
        {
            placementMode_ = SelectionKind::None;
            placingPlatformHitbox_ = true;
        }
        if (ImGui::Button("Select Mode"))
        {
            placementMode_ = SelectionKind::None;
            placingPlatformHitbox_ = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete Selected"))
        {
            deleteSelection();
        }
        ImGui::TextWrapped("DeadAreas kill the player and enemies. Barriers are visual blockers. Platform Hitbox creates a pure invisible wall/collider.");

        ImGui::SeparatorText("Scene Hazards");
        drawHazardList();

        if (selection_.kind == SelectionKind::DeadArea && selection_.index < document_["DeadAreas"].size())
        {
            ImGui::SeparatorText("Selected DeadArea");
            drawHazardRectEditor(
                document_["DeadAreas"][selection_.index],
                "DeadArea",
                sf::Color(242, 104, 56, 255),
                sf::Color(255, 182, 96, 255)
            );
        }
        else if (selection_.kind == SelectionKind::Barrier && selection_.index < document_["Barriers"].size())
        {
            ImGui::SeparatorText("Selected Barrier");
            drawHazardRectEditor(
                document_["Barriers"][selection_.index],
                "Barrier",
                sf::Color(130, 214, 184, 255),
                sf::Color(156, 238, 208, 255)
            );
        }
        else if (selection_.kind == SelectionKind::Platform && selection_.index < document_["Platforms"].size() &&
            isHazardPlatformHitbox(document_["Platforms"][selection_.index]))
        {
            ImGui::SeparatorText("Selected Platform Hitbox");
            auto& platform = document_["Platforms"][selection_.index];
            bool changed = false;
            sf::FloatRect rect = platformBounds(platform);
            float raw[4]{rect.position.x, rect.position.y, rect.size.x, rect.size.y};
            if (ImGui::InputFloat4("Rect X/Y/W/H", raw))
            {
                platform["Position"] = {raw[0], raw[1]};
                platform["HitboxOffset"] = {0.f, 0.f};
                platform["HitboxSize"] = {std::max(raw[2], kPlatformMinHitboxSize), std::max(raw[3], kPlatformMinHitboxSize)};
                platform["EditHitbox"] = true;
                platform["Type"] = "Invisible-wall";
                platform["HazardTool"] = "PlatformHitbox";
                changed = true;
            }
            bool bounceEnabled = platformBounceEnabled(platform);
            if (ImGui::Checkbox("Bounce Enabled", &bounceEnabled))
            {
                platform["BounceEnabled"] = bounceEnabled;
                changed = true;
            }
            if (changed)
            {
                markDirty();
            }
        }
    }

    void drawMiniLocationsInspector()
    {
        ImGui::SeparatorText("Placement");
        drawPlacementButtons(SelectionKind::MiniLocation);
        ImGui::TextWrapped("In Select Mode: drag the room to move the whole mini-location, or drag its corner handle to resize the room.");
        ImGui::SeparatorText("Scene Objects");
        drawMiniLocationList();

        if (selection_.kind == SelectionKind::MiniLocation && selection_.index < document_["MiniLocations"].size())
        {
            ImGui::SeparatorText("Selected Mini Location");
            auto& location = document_["MiniLocations"][selection_.index];
            if (!location.contains("Entry") || !location["Entry"].is_object())
            {
                location["Entry"] = nlohmann::json::object();
            }
            if (!location.contains("Exit") || !location["Exit"].is_object())
            {
                location["Exit"] = nlohmann::json::object();
            }
            ensureMiniLocationNestedArrays(location);
            bool changed = false;
            changed |= editStringField("Id", location, "Id", 256u);
            changed |= editStringField("Title", location, "Title", 256u);
            if (ImGui::Checkbox("Edit Contents", &miniEditor_.contentMode))
            {
                miniEditor_.clearSelection();
                miniEditor_.tool = MiniLocationTool::SelectMove;
                miniLocationInteraction_.clear();
            }
            ImGui::TextDisabled(miniEditor_.contentMode
                ? "Content mode: clicks edit this room's nested objects, even outside Bounds. Tab cycles overlaps."
                : "Enable content mode to edit room objects on the scene.");

            if (miniEditor_.contentMode)
            {
                drawMiniLocationContentEditor(location);
                if (ImGui::Button("Focus Room"))
                {
                    focusMiniLocation(selection_.index);
                }
                if (changed)
                {
                    markDirty();
                }
                return;
            }

            sf::FloatRect bounds = readRect(location.value("Bounds", nlohmann::json::array()), sf::FloatRect({0.f, 0.f}, {920.f, 340.f}));
            float rectRaw[4]{bounds.position.x, bounds.position.y, bounds.size.x, bounds.size.y};
            if (ImGui::InputFloat4("Camera Visibility Area", rectRaw))
            {
                location["Bounds"] = nlohmann::json::array({rectRaw[0], rectRaw[1], std::max(rectRaw[2], 64.f), std::max(rectRaw[3], 64.f)});
                changed = true;
            }

            changed |= editVector2Field(
                "Portal Spawn",
                location,
                "SpawnPosition",
                {bounds.size.x * 0.5f, bounds.size.y - 42.f}
            );
            ImGui::TextDisabled("Portal Spawn is relative to Bounds top-left.");

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
        const float width = (ImGui::GetContentRegionAvail().x - 16.f) / 3.f;
        if (ImGui::Button("Place In Scene", ImVec2(width, 0.f)))
        {
            placementMode_ = placementKind;
            placingPlatformHitbox_ = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Select Mode", ImVec2(width, 0.f)))
        {
            placementMode_ = SelectionKind::None;
            placingPlatformHitbox_ = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete Selected", ImVec2(width, 0.f)))
        {
            deleteSelection();
        }
    }

    void drawPlatformList()
    {
        ImGui::TextDisabled("%zu platforms", document_["Platforms"].size());
        if (ImGui::BeginChild("platform_list", ImVec2(0.f, objectListHeight()), true))
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
        ImGui::TextDisabled("%zu decorations", document_["Decorations"].size());
        if (ImGui::BeginChild("decoration_list", ImVec2(0.f, objectListHeight()), true))
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
        ImGui::TextDisabled("%zu background objects", document_["Background"].size());
        if (ImGui::BeginChild("background_list", ImVec2(0.f, objectListHeight()), true))
        {
            for (std::size_t index = 0; index < document_["Background"].size(); ++index)
            {
                const auto& background = document_["Background"][index];
                const sf::Vector2i tile = backgroundTileIndex(background);
                const std::string typeName = backgroundRepeatTypeName(background);
                const std::string placementLabel = typeName == "single"
                    ? " tile " + std::to_string(tile.x) + "," + std::to_string(tile.y)
                    : " " + typeName;
                const std::string label = std::to_string(index + 1u) + ". " + background.value("BgName", std::string{"Background"}) + placementLabel;
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
        ImGui::TextDisabled("%zu ground strips", document_["Ground"].size());
        if (ImGui::BeginChild("ground_list", ImVec2(0.f, objectListHeight()), true))
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
        ImGui::TextDisabled("%zu spawners", document_["Spawners"].size());
        if (ImGui::BeginChild("spawner_list", ImVec2(0.f, objectListHeight()), true))
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

    void drawPortalList()
    {
        ImGui::TextDisabled("%zu portals", document_["Portals"].size());
        if (ImGui::BeginChild("portal_list", ImVec2(0.f, objectListHeight()), true))
        {
            for (std::size_t index = 0; index < document_["Portals"].size(); ++index)
            {
                const auto& portal = document_["Portals"][index];
                const std::string label = std::to_string(index + 1u) + ". " + portal.value("Title", portal.value("Id", std::string{"Portal"}));
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::Portal && selection_.index == index))
                {
                    selection_ = {SelectionKind::Portal, index};
                }
            }
        }
        ImGui::EndChild();
    }

    void drawInteractiveList()
    {
        ImGui::TextDisabled("%zu interactives", document_["Interactives"].size());
        if (ImGui::BeginChild("interactive_list", ImVec2(0.f, objectListHeight()), true))
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

    void drawHazardList()
    {
        std::size_t hitboxCount = 0u;
        for (const auto& platform : document_["Platforms"])
        {
            if (isHazardPlatformHitbox(platform))
            {
                ++hitboxCount;
            }
        }
        ImGui::TextDisabled("%zu dead areas, %zu barriers, %zu platform hitboxes", document_["DeadAreas"].size(), document_["Barriers"].size(), hitboxCount);
        if (ImGui::BeginChild("hazard_list", ImVec2(0.f, objectListHeight()), true))
        {
            ImGui::SeparatorText("Platform Hitboxes");
            for (std::size_t index = 0; index < document_["Platforms"].size(); ++index)
            {
                const auto& platform = document_["Platforms"][index];
                if (!isHazardPlatformHitbox(platform))
                {
                    continue;
                }

                const sf::FloatRect rect = platformBounds(platform);
                const std::string label = std::to_string(index + 1u) + ". Invisible wall @ " + formatPositionLabel(rect.position);
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::Platform && selection_.index == index))
                {
                    selection_ = {SelectionKind::Platform, index};
                }
            }

            ImGui::SeparatorText("DeadAreas");
            for (std::size_t index = 0; index < document_["DeadAreas"].size(); ++index)
            {
                const auto& deadArea = document_["DeadAreas"][index];
                const sf::FloatRect rect = worldHazardBounds(deadArea);
                const std::string label = std::to_string(index + 1u) + ". " +
                    deadArea.value("Id", std::string{"DeadArea"}) + " @ " +
                    formatPositionLabel(rect.position);
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::DeadArea && selection_.index == index))
                {
                    selection_ = {SelectionKind::DeadArea, index};
                }
            }

            ImGui::SeparatorText("Barriers");
            for (std::size_t index = 0; index < document_["Barriers"].size(); ++index)
            {
                const auto& barrier = document_["Barriers"][index];
                const sf::FloatRect rect = worldHazardBounds(barrier);
                const std::string label = std::to_string(index + 1u) + ". " +
                    barrier.value("Id", std::string{"Barrier"}) + " @ " +
                    formatPositionLabel(rect.position);
                if (ImGui::Selectable(label.c_str(), selection_.kind == SelectionKind::Barrier && selection_.index == index))
                {
                    selection_ = {SelectionKind::Barrier, index};
                }
            }
        }
        ImGui::EndChild();
    }

    void drawMiniLocationList()
    {
        ImGui::TextDisabled("%zu mini locations", document_["MiniLocations"].size());
        if (ImGui::BeginChild("mini_location_list", ImVec2(0.f, objectListHeight()), true))
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
        if (placingPlatformHitbox_)
        {
            return "Place platform hitboxes";
        }

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
        case SelectionKind::DeadArea:
            return "Place dead areas";
        case SelectionKind::Barrier:
            return "Place barriers";
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
        std::filesystem::path executablePath = std::filesystem::current_path() / "Dark Gate.exe";
        if (!std::filesystem::exists(executablePath))
        {
            executablePath = std::filesystem::current_path() / "main.exe";
        }
#else
        std::filesystem::path executablePath = std::filesystem::current_path() / "dark-gate";
        if (!std::filesystem::exists(executablePath))
        {
            executablePath = std::filesystem::current_path() / "Dark Gate";
        }
        if (!std::filesystem::exists(executablePath))
        {
            executablePath = std::filesystem::current_path() / "main";
        }
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
