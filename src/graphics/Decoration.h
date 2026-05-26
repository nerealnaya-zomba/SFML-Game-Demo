#pragma once

#include <SFML/Graphics.hpp>

#include <GameCamera.h>
#include <GameData.h>
#include <Mounting.h>
#include <TexturesIterHelper.h>

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class GameCamera;

struct Vector2fHash
{
    std::size_t operator()(const sf::Vector2f& v) const
    {
        const std::size_t h1 = std::hash<float>{}(v.x);
        const std::size_t h2 = std::hash<float>{}(v.y);
        return h1 ^ (h2 << 1);
    }
};

struct Vector2fEqual
{
    bool operator()(const sf::Vector2f& a, const sf::Vector2f& b) const
    {
        return a.x == b.x && a.y == b.y;
    }
};

struct Vector2fPairHash
{
    std::size_t operator()(const std::pair<sf::Vector2f, sf::Vector2f>& pv) const
    {
        const std::size_t h1 = std::hash<float>{}(pv.second.x);
        const std::size_t h2 = std::hash<float>{}(pv.second.y);
        const std::size_t h3 = std::hash<float>{}(pv.first.x);
        const std::size_t h4 = std::hash<float>{}(pv.first.y);
        return ((h1 ^ (h2 << 1)) ^ (h3 << 2)) ^ (h4 << 3);
    }
};

struct Vector2fPairEqual
{
    bool operator()(const std::pair<sf::Vector2f, sf::Vector2f>& a,
                    const std::pair<sf::Vector2f, sf::Vector2f>& b) const
    {
        return (a.second.x == b.second.x && a.second.y == b.second.y)
            && (a.first.x == b.first.x && a.first.y == b.first.y);
    }
};

struct Vector2fPairWithZ
{
    std::pair<sf::Vector2f, sf::Vector2f> posData;
    int z{};

    Vector2fPairWithZ() = default;
    Vector2fPairWithZ(const std::pair<sf::Vector2f, sf::Vector2f>& p, int zi)
        : posData(p)
        , z(zi)
    {
    }
    Vector2fPairWithZ(std::pair<sf::Vector2f, sf::Vector2f>&& p, int zi)
        : posData(std::move(p))
        , z(zi)
    {
    }
};

struct Vector2fPairWithZHash
{
    std::size_t operator()(const Vector2fPairWithZ& data) const
    {
        const std::size_t h1 = Vector2fPairHash{}(data.posData);
        const std::size_t h2 = std::hash<int>{}(data.z);
        return h1 ^ (h2 << 1);
    }
};

struct Vector2fPairWithZEqual
{
    bool operator()(const Vector2fPairWithZ& a, const Vector2fPairWithZ& b) const
    {
        return Vector2fPairEqual{}(a.posData, b.posData) && a.z == b.z;
    }
};

using DecorationSpriteMap = std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual>;

struct DecorationMotionProfile
{
    sf::Vector2f driftAmplitude{0.f, 0.f};
    float driftSpeed{0.f};
    float swayAmplitude{0.f};
    float swaySpeed{0.f};
    float tiltAmplitude{0.f};
    float tiltSpeed{0.f};
    float scalePulseAmplitude{0.f};
    float scalePulseSpeed{0.f};

    bool hasMotion() const
    {
        return driftAmplitude.x != 0.f
            || driftAmplitude.y != 0.f
            || swayAmplitude != 0.f
            || tiltAmplitude != 0.f
            || scalePulseAmplitude != 0.f;
    }
};

struct DecorationMotionState
{
    DecorationMotionProfile profile{};
    sf::Vector2f baseScale{1.f, 1.f};
    float baseRotation = 0.f;
    float phase{0.f};
    float amplitudeMultiplier{1.f};
    float speedMultiplier{1.f};
};

struct DecorationMiniLocationContext
{
    std::string miniLocationId;
    sf::Vector2f displayPosition{0.f, 0.f};
    sf::Vector2f parallaxReference{0.f, 0.f};
};

struct AnimatedDecorationGroup
{
    std::vector<sf::Texture>* textures{};
    texturesIterHelper* helper{};
    DecorationSpriteMap* sprites{};
    DecorationMotionProfile motion{};
};

class Decoration
{
public:
    Decoration(const Decoration&) = delete;
    Decoration& operator=(const Decoration&) = delete;

    Decoration(Decoration&&) = default;
    Decoration& operator=(Decoration&&) = default;

    Decoration(GameData& gameTextures, GameCamera& c);
    ~Decoration();

    void addDecoration(std::string name,
                       sf::Vector2f position,
                       sf::Vector2f scale,
                       sf::Vector2f parallaxFactor,
                       int z = 0,
                       sf::Color color = sf::Color::White,
                       float rotation = 0.f,
                       std::optional<DecorationMiniLocationContext> miniLocationContext = std::nullopt);
    void setActiveMiniLocation(std::optional<std::string> miniLocationId);
    void updateTextures();
    void drawByZOrder(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    void drawInstance(sf::RenderWindow& window, std::size_t index) const;
    std::size_t getInstanceCount() const;
    void clearDecorations();

private:
    GameData* data{};
    GameCamera* camera{};
    sf::Clock ambientClock;

    std::vector<sf::Texture>* plant1Textures{};
    texturesIterHelper plant1;
    std::vector<sf::Texture>* plant2Textures{};
    texturesIterHelper plant2;
    std::vector<sf::Texture>* plant3Textures{};
    texturesIterHelper plant3;
    std::vector<sf::Texture>* plant4Textures{};
    texturesIterHelper plant4;
    std::vector<sf::Texture>* plant5Textures{};
    texturesIterHelper plant5;
    std::vector<sf::Texture>* plant6Textures{};
    texturesIterHelper plant6;
    std::vector<sf::Texture>* plant7Textures{};
    texturesIterHelper plant7;
    std::vector<sf::Texture>* jumpPlantTextures{};
    texturesIterHelper jumpPlant;
    std::vector<sf::Texture>* jumpPlant2Textures{};
    texturesIterHelper jumpPlant2;
    std::vector<sf::Texture>* plantWind1Textures{};
    texturesIterHelper plantWind1;
    std::vector<sf::Texture>* blueFlower1Textures{};
    texturesIterHelper blueFlower1;
    std::vector<sf::Texture>* blueFlower2Textures{};
    texturesIterHelper blueFlower2;
    std::vector<sf::Texture>* plant8PoisonTextures{};
    texturesIterHelper plant8Poison;
    std::vector<sf::Texture>* cat1Textures{};
    texturesIterHelper catHelper;
    std::vector<sf::Texture>* portalGreenTextures{};
    texturesIterHelper portalGreen;
    std::vector<sf::Texture>* portalVioletTextures{};
    texturesIterHelper portalViolet;
    std::vector<sf::Texture>* portalBlue1Textures{};
    texturesIterHelper portalBlue1;
    std::vector<sf::Texture>* portalBlue2Textures{};
    texturesIterHelper portalBlue2;
    std::vector<sf::Texture>* portalBlue3Textures{};
    texturesIterHelper portalBlue3;
    std::vector<sf::Texture>* portalBlue4Textures{};
    texturesIterHelper portalBlue4;
    std::vector<sf::Texture>* portalBlue5Textures{};
    texturesIterHelper portalBlue5;
    std::vector<sf::Texture>* portalBlue6Textures{};
    texturesIterHelper portalBlue6;
    std::vector<sf::Texture>* portalBlue7Textures{};
    texturesIterHelper portalBlue7;
    std::vector<sf::Texture>* portalBlue8Textures{};
    texturesIterHelper portalBlue8;

    std::map<std::string, sf::Texture>* staticTextures{};

    DecorationSpriteMap plant1Sprites;
    DecorationSpriteMap plant2Sprites;
    DecorationSpriteMap plant3Sprites;
    DecorationSpriteMap plant4Sprites;
    DecorationSpriteMap plant5Sprites;
    DecorationSpriteMap plant6Sprites;
    DecorationSpriteMap plant7Sprites;
    DecorationSpriteMap jumpPlantSprites;
    DecorationSpriteMap jumpPlant2Sprites;
    DecorationSpriteMap plantWind1Sprites;
    DecorationSpriteMap blueFlower1Sprites;
    DecorationSpriteMap blueFlower2Sprites;
    DecorationSpriteMap plant8PoisonSprites;
    DecorationSpriteMap cat1Sprites;
    DecorationSpriteMap portalGreenSprites;
    DecorationSpriteMap portalVioletSprites;
    DecorationSpriteMap portal1BlueSprites;
    DecorationSpriteMap portal2BlueSprites;
    DecorationSpriteMap portal3BlueSprites;
    DecorationSpriteMap portal4BlueSprites;
    DecorationSpriteMap portal5BlueSprites;
    DecorationSpriteMap portal6BlueSprites;
    DecorationSpriteMap portal7BlueSprites;
    DecorationSpriteMap portal8BlueSprites;
    DecorationSpriteMap staticSprites;

    std::vector<AnimatedDecorationGroup> animatedGroups;
    std::unordered_map<std::string, std::size_t> animatedGroupLookup;
    std::vector<DecorationSpriteMap*> spriteMaps;
    std::unordered_map<const sf::Sprite*, DecorationMotionState> motionStates;
    std::unordered_map<const sf::Sprite*, DecorationMiniLocationContext> miniLocationContexts;
    std::set<int> all_Z;
    std::vector<const sf::Sprite*> orderedSprites;
    std::optional<std::string> activeMiniLocationId_;

    void registerAnimatedGroup(const std::string& name,
                               std::vector<sf::Texture>* textures,
                               texturesIterHelper& helper,
                               DecorationSpriteMap& sprites,
                               const DecorationMotionProfile& motion,
                               std::initializer_list<const char*> aliases = {});
    void initAnimatedDecoration(sf::Vector2f position,
                                sf::Vector2f scale,
                                sf::Vector2f parallaxFactor,
                                int z,
                                sf::Color color,
                                float rotation,
                                std::optional<DecorationMiniLocationContext> miniLocationContext,
                                AnimatedDecorationGroup& group);
    void initStaticDecoration(const std::string& name,
                              sf::Vector2f position,
                              sf::Vector2f scale,
                              sf::Vector2f parallaxFactor,
                              int z,
                              sf::Color color,
                              float rotation,
                              std::optional<DecorationMiniLocationContext> miniLocationContext);
    void registerMiniLocationContext(const sf::Sprite& sprite,
                                     std::optional<DecorationMiniLocationContext> miniLocationContext);
    bool shouldDrawSprite(const sf::Sprite& sprite, const sf::FloatRect& viewRect) const;
    void switchToNextSprite(DecorationSpriteMap& spritesArray,
                            std::vector<sf::Texture>& texturesArray,
                            texturesIterHelper& iterHelper);
    void updateParallax();
    void applyParalaxes(const std::pair<sf::Vector2f, sf::Vector2f>& vectorPair,
                        const std::unique_ptr<sf::Sprite>& sprite);
    void registerMotionState(const sf::Sprite& sprite,
                             sf::Vector2f position,
                             sf::Vector2f scale,
                             float rotation,
                             int z,
                             const DecorationMotionProfile& motion);
    DecorationMotionProfile resolveStaticMotionProfile(const std::string& name) const;
};
