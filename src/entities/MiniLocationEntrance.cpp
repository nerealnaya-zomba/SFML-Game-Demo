#include <MiniLocationEntrance.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
constexpr float kPi = 3.14159265359f;
constexpr sf::Keyboard::Scancode kInteractKey = sf::Keyboard::Scancode::Enter;

void configureText(sf::Text& text, unsigned int size, sf::Color color)
{
    text.setCharacterSize(size);
    text.setFillColor(color);
}

const sf::Texture& resolveMiniLocationTexture(GameData& data, const std::string& textureName)
{
    const auto tryResolve = [&](const auto& textures) -> const sf::Texture* {
        const auto it = textures.find(textureName);
        return it != textures.end() ? &it->second : nullptr;
    };

    if (const sf::Texture* texture = tryResolve(data.allStaticTextures))
    {
        return *texture;
    }
    if (const sf::Texture* texture = tryResolve(data.itemsTextures))
    {
        return *texture;
    }
    if (const sf::Texture* texture = tryResolve(data.guiTextures))
    {
        return *texture;
    }
    if (const sf::Texture* texture = tryResolve(data.TileSetGreenTextures))
    {
        return *texture;
    }
    if (const sf::Texture* texture = tryResolve(data.backgroundTextures))
    {
        return *texture;
    }

    throw std::runtime_error("Cannot resolve mini-location texture: " + textureName);
}
}

MiniLocationEntrance::MiniLocationEntrance(
    GameData& data,
    GameCamera& gameCamera,
    Player& controlledPlayer,
    const Config& config
)
    : InteractiveObject(config.position, const_cast<sf::Texture&>(resolveMiniLocationTexture(data, config.textureName)))
    , camera(&gameCamera)
    , player(&controlledPlayer)
    , anchorPosition_(config.position)
    , destinationSupportPoint_(config.destinationSupportPoint)
    , baseScale_(config.scale)
    , baseColor_(config.color)
    , accentColor_(config.accentColor)
    , promptText_(*data.gameFont)
    , subtitleText_(*data.gameFont)
{
    const sf::FloatRect localBounds = sprite->getLocalBounds();
    sprite->setOrigin({
        localBounds.position.x + localBounds.size.x * 0.5f,
        localBounds.position.y + localBounds.size.y * 0.82f
    });
    sprite->setPosition(anchorPosition_);
    sprite->setScale(baseScale_);
    sprite->setColor(baseColor_);

    setCalculationsScale({std::max(0.2f, baseScale_.x * 0.82f), std::max(0.2f, baseScale_.y * 0.82f)});
    offsetToInteract = config.interactRadius;

    shadowMouth_.setRadius(48.f);
    shadowMouth_.setOrigin({shadowMouth_.getRadius(), shadowMouth_.getRadius()});
    shadowMouth_.setScale({1.55f, 0.68f});
    shadowMouth_.setFillColor(sf::Color(10, 10, 14, 210));

    outerGlow_.setRadius(56.f);
    outerGlow_.setOrigin({outerGlow_.getRadius(), outerGlow_.getRadius()});
    outerGlow_.setScale({1.26f, 0.84f});
    outerGlow_.setFillColor(sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 42));

    innerGlow_.setRadius(30.f);
    innerGlow_.setOrigin({innerGlow_.getRadius(), innerGlow_.getRadius()});
    innerGlow_.setScale({1.05f, 0.82f});
    innerGlow_.setFillColor(sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 88));

    promptShadow_.setFillColor(sf::Color(0, 0, 0, 86));
    promptPlate_.setFillColor(sf::Color(16, 12, 15, 236));
    promptPlate_.setOutlineThickness(1.5f);
    promptPlate_.setOutlineColor(sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 220));

    configureText(promptText_, 14, sf::Color(246, 236, 222, 255));
    promptText_.setString(config.prompt.empty() ? "Enter to descend" : config.prompt);

    configureText(subtitleText_, 13, sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 240));
    subtitleText_.setStyle(sf::Text::Italic);
    subtitleText_.setString(config.subtitle);

    updateAmbientMotion();
    updatePromptLayout();
}

void MiniLocationEntrance::updateInteractionState()
{
    isCanInteract = isInAreaOfInteraction(player->getCenterPosition()) && !player->isCDMenuOpened();
}

void MiniLocationEntrance::updateAmbientMotion()
{
    const float time = animationClock_.getElapsedTime().asSeconds();
    const float bob = std::sin(time * 1.35f + anchorPosition_.x * 0.006f) * 4.5f;
    const float pulse = 1.f + std::sin(time * 2.8f + anchorPosition_.y * 0.01f) * 0.08f;
    const float shimmer = 0.92f + std::sin(time * 3.6f + kPi * 0.35f) * 0.08f;

    sprite->setPosition({anchorPosition_.x, anchorPosition_.y + bob});
    sprite->setScale({
        baseScale_.x * (0.985f + std::sin(time * 1.6f + kPi * 0.2f) * 0.02f),
        baseScale_.y * (1.0f + std::cos(time * 1.25f + kPi * 0.35f) * 0.03f)
    });
    sprite->setColor(sf::Color(
        static_cast<std::uint8_t>(std::clamp(baseColor_.r * shimmer, 0.f, 255.f)),
        static_cast<std::uint8_t>(std::clamp(baseColor_.g * shimmer, 0.f, 255.f)),
        static_cast<std::uint8_t>(std::clamp(baseColor_.b * shimmer, 0.f, 255.f)),
        baseColor_.a
    ));

    const sf::Vector2f mouthPosition = {anchorPosition_.x, anchorPosition_.y + 10.f + bob * 0.18f};
    shadowMouth_.setPosition(mouthPosition);
    shadowMouth_.setRotation(sf::degrees(std::sin(time * 0.9f + anchorPosition_.x * 0.002f) * 2.8f));

    outerGlow_.setPosition({anchorPosition_.x, anchorPosition_.y - 22.f + bob * 0.22f});
    outerGlow_.setScale({1.26f * pulse, 0.84f * (0.94f + pulse * 0.08f)});
    innerGlow_.setPosition({anchorPosition_.x, anchorPosition_.y - 14.f + bob * 0.24f});
    innerGlow_.setScale({1.05f * pulse, 0.82f * pulse});
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
        sprite->getGlobalBounds().position.y - 56.f
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
    if (!isCanInteract)
    {
        return false;
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == kInteractKey)
        {
            player->teleportToSupportPoint(destinationSupportPoint_);
            return true;
        }
    }

    return false;
}
