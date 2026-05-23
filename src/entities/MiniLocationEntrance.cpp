#include <MiniLocationEntrance.h>

#include <GameLevel.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{
constexpr float kPi = 3.14159265359f;
constexpr float kPortalVerticalOffset = -26.f;
constexpr sf::Keyboard::Scancode kInteractKey = sf::Keyboard::Scancode::Enter;
const sf::Color kPortalSpriteColor(228, 255, 236, 248);
const sf::Color kPortalAccentColor(104, 224, 158, 255);

void configureText(sf::Text& text, unsigned int size, sf::Color color)
{
    text.setCharacterSize(size);
    text.setFillColor(color);
}
}

MiniLocationEntrance::MiniLocationEntrance(
    GameData& data,
    GameCamera& gameCamera,
    GameLevelManager& manager,
    Player& controlledPlayer,
    const Config& config
)
    : InteractiveObject(config.position, data.portalGreenTextures[0])
    , camera(&gameCamera)
    , levelManager(&manager)
    , player(&controlledPlayer)
    , anchorPosition_(config.position)
    , destinationSupportPoint_(config.destinationSupportPoint)
    , baseScale_(config.scale)
    , baseColor_(kPortalSpriteColor)
    , accentColor_(kPortalAccentColor)
    , exitsMiniLocation_(config.exitsMiniLocation)
    , miniLocationId_(config.miniLocationId)
    , promptText_(*data.gameFont)
    , subtitleText_(*data.gameFont)
{
    attachTexture(data.portalGreenTextures, portalTextures_, data.portalGreen, portalTextureHelper_);

    const sf::FloatRect localBounds = sprite->getLocalBounds();
    sprite->setOrigin({
        localBounds.position.x + localBounds.size.x * 0.5f,
        localBounds.position.y + localBounds.size.y * 0.5f
    });
    sprite->setScale(baseScale_);
    sprite->setColor(baseColor_);

    setCalculationsScale({
        std::max(0.08f, baseScale_.x * 0.72f),
        std::max(0.08f, baseScale_.y * 0.80f)
    });
    offsetToInteract = config.interactRadius;

    shadowMouth_.setRadius(22.f);
    shadowMouth_.setOrigin({shadowMouth_.getRadius(), shadowMouth_.getRadius()});
    shadowMouth_.setScale({1.95f, 0.42f});
    shadowMouth_.setFillColor(sf::Color(6, 18, 10, 200));

    outerGlow_.setRadius(32.f);
    outerGlow_.setOrigin({outerGlow_.getRadius(), outerGlow_.getRadius()});
    outerGlow_.setScale({1.35f, 0.92f});
    outerGlow_.setFillColor(sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 48));

    innerGlow_.setRadius(18.f);
    innerGlow_.setOrigin({innerGlow_.getRadius(), innerGlow_.getRadius()});
    innerGlow_.setScale({1.1f, 0.84f});
    innerGlow_.setFillColor(sf::Color(214, 255, 228, 120));

    promptShadow_.setFillColor(sf::Color(0, 0, 0, 86));
    promptPlate_.setFillColor(sf::Color(10, 18, 14, 236));
    promptPlate_.setOutlineThickness(1.5f);
    promptPlate_.setOutlineColor(sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 220));

    configureText(promptText_, 14, sf::Color(244, 248, 238, 255));
    promptText_.setString(config.prompt.empty() ? "Enter to descend" : config.prompt);

    configureText(subtitleText_, 13, sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 240));
    subtitleText_.setStyle(sf::Text::Italic);
    subtitleText_.setString(config.subtitle);

    updateAmbientMotion();
    updatePromptLayout();
}

void MiniLocationEntrance::updateInteractionState()
{
    isCanInteract =
        player != nullptr
        && !player->isControlsBlocked
        && !player->isMiniLocationTransitionActive()
        && !player->isCDMenuOpened()
        && isInAreaOfInteraction(player->getCenterPosition());
}

void MiniLocationEntrance::updateAmbientMotion()
{
    const float time = animationClock_.getElapsedTime().asSeconds();
    const float bob = std::sin(time * 1.8f + anchorPosition_.x * 0.004f) * 3.4f;
    const float pulse = 1.f + std::sin(time * 3.2f + anchorPosition_.y * 0.01f) * 0.06f;
    const float shimmer = 0.94f + std::sin(time * 4.2f + kPi * 0.35f) * 0.06f;
    const sf::Vector2f portalPosition = {
        anchorPosition_.x,
        anchorPosition_.y + kPortalVerticalOffset + bob
    };

    if (portalTextures_ != nullptr)
    {
        gameUtils::switchToNextSprite(
            sprite.get(),
            *portalTextures_,
            portalTextureHelper_,
            switchSprite_SwitchOption::Loop
        );
    }

    sprite->setPosition(portalPosition);
    sprite->setScale({
        baseScale_.x * (0.985f + std::sin(time * 2.0f + kPi * 0.2f) * 0.025f),
        baseScale_.y * (0.985f + std::cos(time * 1.65f + kPi * 0.4f) * 0.025f)
    });
    sprite->setColor(sf::Color(
        static_cast<std::uint8_t>(std::clamp(baseColor_.r * shimmer, 0.f, 255.f)),
        static_cast<std::uint8_t>(std::clamp(baseColor_.g * shimmer, 0.f, 255.f)),
        static_cast<std::uint8_t>(std::clamp(baseColor_.b * shimmer, 0.f, 255.f)),
        baseColor_.a
    ));

    shadowMouth_.setPosition({anchorPosition_.x, anchorPosition_.y + 12.f + bob * 0.15f});
    shadowMouth_.setRotation(sf::degrees(std::sin(time * 1.2f + anchorPosition_.x * 0.003f) * 3.f));

    outerGlow_.setPosition({portalPosition.x, portalPosition.y - 2.f});
    outerGlow_.setScale({1.35f * pulse, 0.92f * (0.96f + pulse * 0.08f)});

    innerGlow_.setPosition({portalPosition.x, portalPosition.y + 2.f});
    innerGlow_.setScale({1.1f * pulse, 0.84f * pulse});
}

void MiniLocationEntrance::updatePromptLayout()
{
    const sf::FloatRect promptBounds = promptText_.getLocalBounds();
    const sf::FloatRect subtitleBounds = subtitleText_.getLocalBounds();
    const float contentWidth = std::max(promptBounds.size.x, subtitleBounds.size.x);
    const sf::Vector2f plateSize = {
        std::max(176.f, contentWidth + 30.f),
        subtitleText_.getString().isEmpty() ? 32.f : 48.f
    };
    const sf::Vector2f platePos = {
        sprite->getGlobalBounds().getCenter().x - plateSize.x / 2.f,
        sprite->getGlobalBounds().position.y - 52.f
    };

    promptShadow_.setSize(plateSize);
    promptShadow_.setPosition({platePos.x + 4.f, platePos.y + 5.f});
    promptPlate_.setSize(plateSize);
    promptPlate_.setPosition(platePos);

    promptText_.setPosition({
        platePos.x + (plateSize.x - promptBounds.size.x) / 2.f - promptBounds.position.x,
        platePos.y + 6.f - promptBounds.position.y
    });

    subtitleText_.setPosition({
        platePos.x + (plateSize.x - subtitleBounds.size.x) / 2.f - subtitleBounds.position.x,
        platePos.y + (subtitleText_.getString().isEmpty() ? 0.f : 24.f) - subtitleBounds.position.y
    });
}

void MiniLocationEntrance::draw(sf::RenderWindow& window)
{
    window.draw(shadowMouth_);
    window.draw(outerGlow_);
    window.draw(innerGlow_);
    window.draw(*sprite);

    if (isCanInteract)
    {
        window.draw(promptShadow_);
        window.draw(promptPlate_);
        window.draw(promptText_);
        if (!subtitleText_.getString().isEmpty())
        {
            window.draw(subtitleText_);
        }
    }
}

void MiniLocationEntrance::update()
{
    updateInteractionState();
    updateAmbientMotion();
    updatePromptLayout();
}

bool MiniLocationEntrance::handleEvent(const sf::Event& event)
{
    if (!isCanInteract || player == nullptr || player->isControlsBlocked)
    {
        return false;
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == kInteractKey)
        {
            return player->beginPortalTransition(
                [this]() {
                    if (levelManager != nullptr)
                    {
                        if (exitsMiniLocation_)
                        {
                            const sf::Vector2f returnSupportPoint = levelManager->exitCurrentMiniLocation(destinationSupportPoint_);
                            player->teleportToSupportPoint(returnSupportPoint);
                            return true;
                        }
                        else if (!miniLocationId_.empty())
                        {
                            if (!levelManager->enterCurrentMiniLocation(miniLocationId_, player->getFeetPosition()))
                            {
                                return false;
                            }
                        }
                    }
                    player->teleportToSupportPoint(destinationSupportPoint_);
                    return true;
                },
                sprite->getGlobalBounds().getCenter(),
                accentColor_
            );
        }
    }

    return false;
}

bool MiniLocationEntrance::blocksPlayerInput() const
{
    return player != nullptr && player->isMiniLocationTransitionActive();
}
