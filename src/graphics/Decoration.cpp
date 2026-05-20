#include "Decoration.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
constexpr float kPi = 3.14159265359f;

float fract(float value)
{
    return value - std::floor(value);
}

float seededNoise(sf::Vector2f position, int z, float salt)
{
    const float value = std::sin(
        position.x * 0.01373f
        + position.y * 0.00919f
        + static_cast<float>(z) * 0.07131f
        + salt * 17.0f
    ) * 43758.5453f;

    return fract(value);
}

DecorationMotionProfile makePlantMotion(float swayAmplitude,
                                        float swaySpeed,
                                        float tiltAmplitude,
                                        float tiltSpeed,
                                        float driftY,
                                        float driftSpeed,
                                        float pulseAmplitude = 0.0f,
                                        float pulseSpeed = 0.0f)
{
    DecorationMotionProfile profile;
    profile.driftAmplitude = {swayAmplitude * 0.18f, driftY};
    profile.driftSpeed = driftSpeed;
    profile.swayAmplitude = swayAmplitude;
    profile.swaySpeed = swaySpeed;
    profile.tiltAmplitude = tiltAmplitude;
    profile.tiltSpeed = tiltSpeed;
    profile.scalePulseAmplitude = pulseAmplitude;
    profile.scalePulseSpeed = pulseSpeed;
    return profile;
}

DecorationMotionProfile makePortalMotion(float driftX,
                                         float driftY,
                                         float driftSpeed,
                                         float pulseAmplitude,
                                         float pulseSpeed)
{
    DecorationMotionProfile profile;
    profile.driftAmplitude = {driftX, driftY};
    profile.driftSpeed = driftSpeed;
    profile.scalePulseAmplitude = pulseAmplitude;
    profile.scalePulseSpeed = pulseSpeed;
    return profile;
}
}

Decoration::Decoration(GameData& gameTextures, GameCamera& c)
    : data(&gameTextures)
    , camera(&c)
{
    attachTexture(gameTextures.plant1Textures, plant1Textures, gameTextures.plant1, plant1);
    attachTexture(gameTextures.plant2Textures, plant2Textures, gameTextures.plant2, plant2);
    attachTexture(gameTextures.plant3Textures, plant3Textures, gameTextures.plant3, plant3);
    attachTexture(gameTextures.plant4Textures, plant4Textures, gameTextures.plant4, plant4);
    attachTexture(gameTextures.plant5Textures, plant5Textures, gameTextures.plant5, plant5);
    attachTexture(gameTextures.plant6Textures, plant6Textures, gameTextures.plant6, plant6);
    attachTexture(gameTextures.plant7Textures, plant7Textures, gameTextures.plant7, plant7);
    attachTexture(gameTextures.jumpPlantTextures, jumpPlantTextures, gameTextures.jumpPlant, jumpPlant);
    attachTexture(gameTextures.jumpPlant2Textures, jumpPlant2Textures, gameTextures.jumpPlant2, jumpPlant2);
    attachTexture(gameTextures.plantWind1Textures, plantWind1Textures, gameTextures.plantWind1, plantWind1);
    attachTexture(gameTextures.blueFlower1Textures, blueFlower1Textures, gameTextures.blueFlower1, blueFlower1);
    attachTexture(gameTextures.blueFlower2Textures, blueFlower2Textures, gameTextures.blueFlower2, blueFlower2);
    attachTexture(gameTextures.plant8PoisonTextures, plant8PoisonTextures, gameTextures.plant8Poison, plant8Poison);
    attachTexture(gameTextures.cat1Textures, cat1Textures, gameTextures.catHelper, catHelper);
    attachTexture(gameTextures.portalGreenTextures, portalGreenTextures, gameTextures.portalGreen, portalGreen);
    attachTexture(gameTextures.portalVioletTextures, portalVioletTextures, gameTextures.portalViolet, portalViolet);
    attachTexture(gameTextures.portalBlue1Textures, portalBlue1Textures, gameTextures.portalBlue1Helper, portalBlue1);
    attachTexture(gameTextures.portalBlue2Textures, portalBlue2Textures, gameTextures.portalBlue2Helper, portalBlue2);
    attachTexture(gameTextures.portalBlue3Textures, portalBlue3Textures, gameTextures.portalBlue3Helper, portalBlue3);
    attachTexture(gameTextures.portalBlue4Textures, portalBlue4Textures, gameTextures.portalBlue4Helper, portalBlue4);
    attachTexture(gameTextures.portalBlue5Textures, portalBlue5Textures, gameTextures.portalBlue5Helper, portalBlue5);
    attachTexture(gameTextures.portalBlue6Textures, portalBlue6Textures, gameTextures.portalBlue6Helper, portalBlue6);
    attachTexture(gameTextures.portalBlue7Textures, portalBlue7Textures, gameTextures.portalBlue7Helper, portalBlue7);
    attachTexture(gameTextures.portalBlue8Textures, portalBlue8Textures, gameTextures.portalBlue8Helper, portalBlue8);
    attachTexture(gameTextures.allStaticTextures, staticTextures);

    registerAnimatedGroup("plant1", plant1Textures, plant1, plant1Sprites, makePlantMotion(7.0f, 0.72f, 2.2f, 0.92f, 3.0f, 0.54f));
    registerAnimatedGroup("plant2", plant2Textures, plant2, plant2Sprites, makePlantMotion(8.5f, 0.74f, 2.4f, 0.96f, 3.2f, 0.56f));
    registerAnimatedGroup("plant3", plant3Textures, plant3, plant3Sprites, makePlantMotion(6.5f, 0.70f, 1.9f, 0.88f, 2.6f, 0.50f));
    registerAnimatedGroup("plant4", plant4Textures, plant4, plant4Sprites, makePlantMotion(5.0f, 0.66f, 1.6f, 0.82f, 2.2f, 0.46f));
    registerAnimatedGroup("plant5", plant5Textures, plant5, plant5Sprites, makePlantMotion(7.5f, 0.76f, 2.1f, 0.94f, 3.4f, 0.58f));
    registerAnimatedGroup("plant6", plant6Textures, plant6, plant6Sprites, makePlantMotion(8.0f, 0.78f, 2.5f, 0.98f, 3.6f, 0.60f));
    registerAnimatedGroup("plant7", plant7Textures, plant7, plant7Sprites, makePlantMotion(9.0f, 0.82f, 2.7f, 1.04f, 4.2f, 0.64f));
    registerAnimatedGroup("jumpPlant", jumpPlantTextures, jumpPlant, jumpPlantSprites, makePlantMotion(4.0f, 1.10f, 1.4f, 1.18f, 5.0f, 1.20f, 0.016f, 1.25f));
    registerAnimatedGroup("jumpPlant2", jumpPlant2Textures, jumpPlant2, jumpPlant2Sprites, makePlantMotion(4.6f, 1.16f, 1.6f, 1.24f, 5.6f, 1.28f, 0.018f, 1.34f), {"jumpBloom2"});
    registerAnimatedGroup("windPlant1", plantWind1Textures, plantWind1, plantWind1Sprites, makePlantMotion(10.0f, 0.90f, 3.0f, 1.08f, 3.4f, 0.68f), {"windPlant", "plantWind1"});
    registerAnimatedGroup("blueFlower1", blueFlower1Textures, blueFlower1, blueFlower1Sprites, makePlantMotion(3.0f, 0.86f, 1.2f, 0.92f, 4.8f, 0.78f, 0.022f, 1.48f), {"blueFlower"});
    registerAnimatedGroup("blueFlower2", blueFlower2Textures, blueFlower2, blueFlower2Sprites, makePlantMotion(2.8f, 0.82f, 1.1f, 0.88f, 4.0f, 0.74f, 0.019f, 1.40f), {"blueFlowerClosed"});
    registerAnimatedGroup("poisonPlant", plant8PoisonTextures, plant8Poison, plant8PoisonSprites, makePlantMotion(7.2f, 0.84f, 2.3f, 0.98f, 3.8f, 0.66f, 0.014f, 1.18f), {"plant8", "plant8Poison", "poisonPlant8"});
    registerAnimatedGroup("cat", cat1Textures, catHelper, cat1Sprites, makePlantMotion(0.4f, 0.24f, 0.0f, 0.0f, 1.6f, 0.34f, 0.012f, 0.78f));
    registerAnimatedGroup("portalGreen", portalGreenTextures, portalGreen, portalGreenSprites, makePortalMotion(2.0f, 6.0f, 0.96f, 0.030f, 1.52f));
    registerAnimatedGroup("portalViolet", portalVioletTextures, portalViolet, portalVioletSprites, makePortalMotion(2.0f, 6.0f, 0.96f, 0.030f, 1.52f));
    registerAnimatedGroup("portalBlue1", portalBlue1Textures, portalBlue1, portal1BlueSprites, makePortalMotion(1.8f, 6.6f, 1.02f, 0.032f, 1.58f));
    registerAnimatedGroup("portalBlue2", portalBlue2Textures, portalBlue2, portal2BlueSprites, makePortalMotion(1.8f, 6.4f, 1.04f, 0.031f, 1.60f));
    registerAnimatedGroup("portalBlue3", portalBlue3Textures, portalBlue3, portal3BlueSprites, makePortalMotion(1.9f, 6.7f, 1.06f, 0.033f, 1.62f));
    registerAnimatedGroup("portalBlue4", portalBlue4Textures, portalBlue4, portal4BlueSprites, makePortalMotion(1.9f, 6.9f, 1.08f, 0.034f, 1.64f));
    registerAnimatedGroup("portalBlue5", portalBlue5Textures, portalBlue5, portal5BlueSprites, makePortalMotion(2.0f, 7.0f, 1.10f, 0.034f, 1.66f));
    registerAnimatedGroup("portalBlue6", portalBlue6Textures, portalBlue6, portal6BlueSprites, makePortalMotion(2.0f, 7.1f, 1.12f, 0.035f, 1.68f));
    registerAnimatedGroup("portalBlue7", portalBlue7Textures, portalBlue7, portal7BlueSprites, makePortalMotion(2.1f, 7.2f, 1.14f, 0.035f, 1.70f));
    registerAnimatedGroup("portalBlue8", portalBlue8Textures, portalBlue8, portal8BlueSprites, makePortalMotion(2.1f, 7.3f, 1.16f, 0.036f, 1.72f));

    spriteMaps.push_back(&staticSprites);
}

Decoration::~Decoration() = default;

void Decoration::registerAnimatedGroup(const std::string& name,
                                       std::vector<sf::Texture>* textures,
                                       texturesIterHelper& helper,
                                       DecorationSpriteMap& sprites,
                                       const DecorationMotionProfile& motion,
                                       std::initializer_list<const char*> aliases)
{
    const std::size_t groupIndex = animatedGroups.size();
    animatedGroups.push_back(AnimatedDecorationGroup{textures, &helper, &sprites, motion});
    animatedGroupLookup.emplace(name, groupIndex);

    for (const char* alias : aliases)
    {
        animatedGroupLookup.emplace(alias, groupIndex);
    }

    spriteMaps.push_back(&sprites);
}

void Decoration::addDecoration(std::string name,
                               sf::Vector2f position,
                               sf::Vector2f scale,
                               sf::Vector2f parallaxFactor,
                               int z,
                               sf::Color color)
{
    const auto animatedIt = animatedGroupLookup.find(name);
    if (animatedIt != animatedGroupLookup.end())
    {
        initAnimatedDecoration(position, scale, parallaxFactor, z, color, animatedGroups.at(animatedIt->second));
        return;
    }

    initStaticDecoration(name, position, scale, parallaxFactor, z, color);
}

void Decoration::initAnimatedDecoration(sf::Vector2f position,
                                        sf::Vector2f scale,
                                        sf::Vector2f parallaxFactor,
                                        int z,
                                        sf::Color color,
                                        AnimatedDecorationGroup& group)
{
    if (!group.textures || group.textures->empty())
    {
        throw std::runtime_error("Animated decoration group has no textures");
    }

    auto sprite = std::make_unique<sf::Sprite>(group.textures->front());
    setSpriteOriginToMiddle(*sprite);
    sprite->setPosition(position);
    sprite->setScale(scale);
    sprite->setColor(color);

    registerMotionState(*sprite, position, scale, z, group.motion);

    all_Z.insert(z);
    auto inserted = group.sprites->emplace(Vector2fPairWithZ(std::pair(parallaxFactor, position), z), std::move(sprite));
    orderedSprites.push_back(inserted->second.get());
}

void Decoration::initStaticDecoration(const std::string& name,
                                      sf::Vector2f position,
                                      sf::Vector2f scale,
                                      sf::Vector2f parallaxFactor,
                                      int z,
                                      sf::Color color)
{
    if (!staticTextures)
    {
        throw std::runtime_error("Static decoration atlas is not available");
    }

    const auto textureIt = staticTextures->find(name);
    if (textureIt == staticTextures->end())
    {
        throw std::runtime_error("Unknown decoration texture: " + name);
    }

    auto sprite = std::make_unique<sf::Sprite>(textureIt->second);
    setSpriteOriginToMiddle(*sprite);
    sprite->setPosition(position);
    sprite->setScale(scale);
    sprite->setColor(color);

    registerMotionState(*sprite, position, scale, z, resolveStaticMotionProfile(name));

    all_Z.insert(z);
    auto inserted = staticSprites.emplace(Vector2fPairWithZ(std::pair(parallaxFactor, position), z), std::move(sprite));
    orderedSprites.push_back(inserted->second.get());
}

void Decoration::registerMotionState(const sf::Sprite& sprite,
                                     sf::Vector2f position,
                                     sf::Vector2f scale,
                                     int z,
                                     const DecorationMotionProfile& motion)
{
    if (!motion.hasMotion())
    {
        return;
    }

    const float phase = seededNoise(position, z, 0.13f) * kPi * 2.0f;
    const float amplitudeMultiplier = 0.82f + seededNoise(position, z, 0.57f) * 0.42f;
    const float speedMultiplier = 0.86f + seededNoise(position, z, 1.19f) * 0.34f;

    motionStates.emplace(&sprite, DecorationMotionState{
        motion,
        scale,
        phase,
        amplitudeMultiplier,
        speedMultiplier
    });
}

DecorationMotionProfile Decoration::resolveStaticMotionProfile(const std::string& name) const
{
    if (name.rfind("MossyHangingPlants_", 0) == 0)
    {
        return makePlantMotion(6.0f, 0.62f, 1.4f, 0.74f, 2.0f, 0.46f);
    }

    if (name.rfind("MossyBackgroundDecoration_", 0) == 0)
    {
        DecorationMotionProfile profile;
        profile.driftAmplitude = {1.4f, 2.6f};
        profile.driftSpeed = 0.30f;
        return profile;
    }

    if (name.rfind("MossyDecorationHazard_", 0) == 0)
    {
        DecorationMotionProfile profile;
        profile.driftAmplitude = {1.0f, 1.8f};
        profile.driftSpeed = 0.54f;
        profile.scalePulseAmplitude = 0.010f;
        profile.scalePulseSpeed = 1.10f;
        return profile;
    }

    return {};
}

void Decoration::switchToNextSprite(DecorationSpriteMap& spritesArray,
                                    std::vector<sf::Texture>& texturesArray,
                                    texturesIterHelper& iterHelper)
{
    if (spritesArray.empty() || texturesArray.empty())
    {
        return;
    }

    if (texturesArray.size() == 1)
    {
        for (auto& entry : spritesArray)
        {
            entry.second->setTexture(texturesArray.front(), true);
        }
        return;
    }

    if (iterHelper.iterationCounter < iterHelper.iterationsTillSwitch)
    {
        ++iterHelper.iterationCounter;
        return;
    }

    iterHelper.iterationCounter = 0;

    const int lastIndex = static_cast<int>(texturesArray.size()) - 1;
    iterHelper.ptrToTexture = std::clamp(iterHelper.ptrToTexture, 0, lastIndex);

    for (auto& entry : spritesArray)
    {
        entry.second->setTexture(texturesArray.at(static_cast<std::size_t>(iterHelper.ptrToTexture)), true);
    }

    if (iterHelper.goForward)
    {
        if (iterHelper.ptrToTexture >= lastIndex)
        {
            iterHelper.goForward = false;
            --iterHelper.ptrToTexture;
        }
        else
        {
            ++iterHelper.ptrToTexture;
        }
    }
    else
    {
        if (iterHelper.ptrToTexture <= 0)
        {
            iterHelper.goForward = true;
            ++iterHelper.ptrToTexture;
        }
        else
        {
            --iterHelper.ptrToTexture;
        }
    }
}

void Decoration::updateParallax()
{
    for (DecorationSpriteMap* spriteMap : spriteMaps)
    {
        for (auto& entry : *spriteMap)
        {
            applyParalaxes(entry.first.posData, entry.second);
        }
    }
}

void Decoration::applyParalaxes(const std::pair<sf::Vector2f, sf::Vector2f>& vectorPair,
                                const std::unique_ptr<sf::Sprite>& sprite)
{
    if (!sprite)
    {
        return;
    }

    const sf::Vector2f baseObjectPos = vectorPair.second;
    const sf::Vector2f parallaxFactor = vectorPair.first;
    const sf::Vector2f cameraOffset = camera->getCameraCenterPos() - BASE_CAMERAPOS;
    const sf::Vector2f basePosition = {
        baseObjectPos.x + cameraOffset.x * parallaxFactor.x,
        baseObjectPos.y + cameraOffset.y * parallaxFactor.y
    };

    sprite->setPosition(basePosition);

    const auto motionIt = motionStates.find(sprite.get());
    if (motionIt == motionStates.end())
    {
        return;
    }

    const DecorationMotionState& state = motionIt->second;
    const float time = ambientClock.getElapsedTime().asSeconds() * state.speedMultiplier;

    sf::Vector2f motionOffset = {
        std::sin(time * state.profile.swaySpeed + state.phase) * state.profile.swayAmplitude,
        std::cos(time * state.profile.driftSpeed + state.phase * 0.89f) * state.profile.driftAmplitude.y
    };

    motionOffset.x += std::sin(time * state.profile.driftSpeed * 0.76f + state.phase * 1.33f)
        * state.profile.driftAmplitude.x;

    motionOffset *= state.amplitudeMultiplier;

    const float rotation = std::sin(time * state.profile.tiltSpeed + state.phase * 0.71f)
        * state.profile.tiltAmplitude
        * state.amplitudeMultiplier;

    const float pulseX = 1.0f + std::sin(time * state.profile.scalePulseSpeed + state.phase * 1.47f)
        * state.profile.scalePulseAmplitude
        * state.amplitudeMultiplier;
    const float pulseY = 1.0f + std::cos(time * state.profile.scalePulseSpeed * 0.82f + state.phase * 0.93f)
        * state.profile.scalePulseAmplitude
        * 0.55f
        * state.amplitudeMultiplier;

    sprite->setPosition(basePosition + motionOffset);
    sprite->setRotation(sf::degrees(rotation));
    sprite->setScale({state.baseScale.x * pulseX, state.baseScale.y * pulseY});
}

void Decoration::updateTextures()
{
    for (AnimatedDecorationGroup& group : animatedGroups)
    {
        if (group.textures && group.helper && group.sprites)
        {
            switchToNextSprite(*group.sprites, *group.textures, *group.helper);
        }
    }

    updateParallax();
}

void Decoration::drawByZOrder(sf::RenderWindow& window)
{
    for (int z : all_Z)
    {
        for (DecorationSpriteMap* spriteMap : spriteMaps)
        {
            for (const auto& entry : *spriteMap)
            {
                if (entry.first.z == z && entry.second)
                {
                    window.draw(*entry.second);
                }
            }
        }
    }
}

void Decoration::draw(sf::RenderWindow& window)
{
    drawByZOrder(window);
}

void Decoration::drawInstance(sf::RenderWindow& window, const std::size_t index) const
{
    if (index >= orderedSprites.size() || orderedSprites[index] == nullptr)
    {
        return;
    }

    window.draw(*orderedSprites[index]);
}

std::size_t Decoration::getInstanceCount() const
{
    return orderedSprites.size();
}

void Decoration::clearDecorations()
{
    for (DecorationSpriteMap* spriteMap : spriteMaps)
    {
        spriteMap->clear();
    }

    motionStates.clear();
    all_Z.clear();
    orderedSprites.clear();
}
