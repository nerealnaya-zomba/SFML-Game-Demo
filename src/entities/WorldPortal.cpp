#include <WorldPortal.h>

#include <Localization.h>

#include <algorithm>
#include <cmath>

namespace
{
constexpr sf::Keyboard::Scancode kInteractKey = sf::Keyboard::Scancode::Enter;

sf::Texture& resolveFirstPortalTexture(GameData& data, const std::string& portalTexture)
{
    if (portalTexture == "portalViolet" && !data.portalVioletTextures.empty())
    {
        return data.portalVioletTextures[0];
    }

    return data.portalGreenTextures[0];
}

void attachPortalTextures(GameData& data, const std::string& portalTexture, std::vector<sf::Texture>*& textures, texturesIterHelper& helper)
{
    if (portalTexture == "portalViolet" && !data.portalVioletTextures.empty())
    {
        attachTexture(data.portalVioletTextures, textures, data.portalViolet, helper);
        return;
    }

    attachTexture(data.portalGreenTextures, textures, data.portalGreen, helper);
}
}

WorldPortal::WorldPortal(
    GameData& data,
    GameCamera& camera,
    GameLevelManager& manager,
    Player& player,
    const Config& config
)
    : InteractiveObject(config.position, resolveFirstPortalTexture(data, config.portalTexture))
    , camera_(&camera)
    , manager_(&manager)
    , player_(&player)
    , config_(config)
    , promptText_(*data.gameFont)
{
    attachPortalTextures(data, config_.portalTexture, portalTextures_, portalHelper_);

    setSpriteOriginToMiddle(*sprite);
    sprite->setPosition(config_.position);
    sprite->setScale(config_.scale);
    sprite->setColor(config_.color);
    setCalculationsScale({std::max(0.08f, config_.scale.x * 0.60f), std::max(0.08f, config_.scale.y * 0.86f)});
    offsetToInteract = config_.interactRadius;

    outerGlow_.setRadius(44.f);
    outerGlow_.setOrigin({44.f, 44.f});
    outerGlow_.setFillColor(sf::Color(config_.accentColor.r, config_.accentColor.g, config_.accentColor.b, 54));

    innerGlow_.setRadius(22.f);
    innerGlow_.setOrigin({22.f, 22.f});
    innerGlow_.setFillColor(sf::Color(220, 246, 255, 112));

    promptBack_.setFillColor(sf::Color(8, 14, 24, 232));
    promptBack_.setOutlineThickness(1.5f);
    promptBack_.setOutlineColor(config_.accentColor);

    promptText_.setCharacterSize(14u);
    promptText_.setFillColor(sf::Color(244, 250, 255, 255));
    Localization::setText(promptText_, config_.prompt.empty()
        ? (Localization::isRussian() ? Localization::tr("world.enter_portal") : "Enter portal")
        : config_.prompt);

    updateVisuals();
    updatePromptLayout();
}

void WorldPortal::updateInteractionState()
{
    const bool playerInActivationArea = player_ != nullptr &&
        (config_.hasActivationArea
            ? config_.activationArea.contains(player_->getCenterPosition())
            : isInAreaOfInteraction(player_->getCenterPosition()));
    isCanInteract =
        player_ != nullptr
        && !player_->isControlsBlocked
        && !player_->isMiniLocationTransitionActive()
        && !player_->isCDMenuOpened()
        && playerInActivationArea;
}

void WorldPortal::updateVisuals()
{
    if (portalTextures_ != nullptr)
    {
        gameUtils::switchToNextSprite(
            sprite.get(),
            *portalTextures_,
            portalHelper_,
            switchSprite_SwitchOption::Loop
        );
    }

    const float time = animationClock_.getElapsedTime().asSeconds();
    const float pulse = 1.f + std::sin(time * 2.8f + config_.position.x * 0.002f) * 0.08f;
    const float bob = std::sin(time * 1.55f + config_.position.y * 0.004f) * 3.2f;
    const sf::Vector2f center{config_.position.x, config_.position.y + bob};

    sprite->setPosition(center);
    sprite->setScale({config_.scale.x * pulse, config_.scale.y * (0.96f + pulse * 0.04f)});
    sprite->setColor(config_.color);

    outerGlow_.setPosition(center);
    outerGlow_.setScale({1.18f * pulse, 0.92f * pulse});
    innerGlow_.setPosition(center);
    innerGlow_.setScale({1.0f * pulse, 0.82f * pulse});
}

void WorldPortal::updatePromptLayout()
{
    const sf::FloatRect textBounds = promptText_.getLocalBounds();
    const sf::Vector2f size{std::max(154.f, textBounds.size.x + 28.f), 32.f};
    const sf::Vector2f topLeft{
        sprite->getGlobalBounds().getCenter().x - size.x * 0.5f,
        sprite->getGlobalBounds().position.y - 46.f
    };

    promptBack_.setSize(size);
    promptBack_.setPosition(topLeft);
    promptText_.setPosition({
        topLeft.x + (size.x - textBounds.size.x) * 0.5f - textBounds.position.x,
        topLeft.y + 7.f - textBounds.position.y
    });
}

void WorldPortal::update()
{
    if (transitioning_ && player_ != nullptr && !player_->isMiniLocationTransitionActive())
    {
        transitioning_ = false;
    }

    updateInteractionState();
    updateVisuals();
    updatePromptLayout();
}

void WorldPortal::draw(sf::RenderWindow& window)
{
    window.draw(outerGlow_);
    window.draw(innerGlow_);
    window.draw(*sprite);

    if (isCanInteract)
    {
        window.draw(promptBack_);
        window.draw(promptText_);
    }
}

bool WorldPortal::activate()
{
    if (!manager_ || !player_ || transitioning_)
    {
        return false;
    }

    const Target& target = config_.target;
    const sf::Vector2f portalCenter = sprite ? sprite->getGlobalBounds().getCenter() : config_.position;
    if (target.type == TargetType::Level)
    {
        if (target.levelId.empty())
        {
            return false;
        }

        return player_->beginPortalTransition(
            [this, target]() {
                if (target.spawnPosition.has_value())
                {
                    return manager_->teleportPlayerToLevelPosition(target.levelId, *target.spawnPosition);
                }

                return manager_->teleportPlayerToLevelSpawn(target.levelId);
            },
            portalCenter,
            config_.accentColor
        );
    }

    const sf::Vector2f destination = target.spawnPosition.value_or(target.position);
    if (target.type == TargetType::MiniLocation)
    {
        return player_->beginPortalTransition(
            [this, target, destination]() {
                return manager_->teleportPlayerToCurrentMiniLocationPosition(
                    target.miniLocationId,
                    destination,
                    player_->getFeetPosition()
                );
            },
            portalCenter,
            config_.accentColor
        );
    }

    return player_->beginPortalTransition(
        [this, destination]() {
            manager_->exitCurrentMiniLocation();
            return manager_->teleportPlayerToCurrentLevelPosition(destination);
        },
        portalCenter,
        config_.accentColor
    );
}

bool WorldPortal::handleEvent(const sf::Event& event)
{
    if (!isCanInteract)
    {
        return false;
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == kInteractKey)
        {
            transitioning_ = activate();
            return transitioning_;
        }
    }

    return false;
}

bool WorldPortal::blocksPlayerInput() const
{
    return player_ != nullptr && player_->isMiniLocationTransitionActive();
}
