#include <WorldInteractable.h>

#include <GameLevel.h>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace
{
constexpr float kPi = 3.14159265359f;
constexpr sf::Keyboard::Scancode kInteractKey = sf::Keyboard::Scancode::Enter;
constexpr sf::Keyboard::Scancode kCloseKey = sf::Keyboard::Scancode::Escape;

void configureText(sf::Text& text, unsigned int size, sf::Color color)
{
    text.setCharacterSize(size);
    text.setFillColor(color);
}

const sf::Texture& resolveInteractiveTexture(GameData& data, const std::string& textureName)
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

    throw std::runtime_error("Cannot resolve interactive texture: " + textureName);
}
}

WorldInteractable::WorldInteractable(
    GameData& gameData,
    GameCamera& gameCamera,
    GameLevelManager& levelManager,
    Player& controlledPlayer,
    const Config& config
)
    : InteractiveObject(config.position, const_cast<sf::Texture&>(resolveInteractiveTexture(gameData, config.textureName)))
    , data(&gameData)
    , camera(&gameCamera)
    , manager(&levelManager)
    , player(&controlledPlayer)
    , type_(config.type)
    , anchorPosition_(config.position)
    , baseScale_(config.scale)
    , baseColor_(config.color)
    , accentColor_(config.accentColor)
    , goldReward_(config.goldReward)
    , singleUse_(config.singleUse)
    , grantsCheckpoint_(config.grantsCheckpoint)
    , restoreVitality_(config.restoreVitality)
    , hasCustomSpawnOffset_(config.hasCustomSpawnOffset)
    , spawnOffset_(config.spawnOffset)
    , hiddenUntilNearby_(config.hiddenUntilNearby)
    , discovered_(!config.hiddenUntilNearby)
    , revealRadius_(config.revealRadius)
    , revealTitle_(config.revealTitle)
    , revealBody_(config.revealBody)
    , promptText_(*gameData.gameFont)
    , titleText_(*gameData.gameFont)
    , bodyText_(*gameData.gameFont)
    , hintText_(*gameData.gameFont)
{
    animationPhase_ = anchorPosition_.x * 0.011f + anchorPosition_.y * 0.007f;

    setSpriteOriginToMiddle(*sprite);
    setScale(baseScale_);
    setCalculationsScale(baseScale_);
    setPosition(anchorPosition_);
    sprite->setColor(baseColor_);

    offsetToInteract = config.interactRadius;

    shadow_.setRadius(26.f);
    shadow_.setOrigin({shadow_.getRadius(), shadow_.getRadius()});
    shadow_.setScale({1.55f, 0.50f});
    shadow_.setFillColor(sf::Color(0, 0, 0, 92));

    halo_.setRadius(34.f);
    halo_.setOrigin({halo_.getRadius(), halo_.getRadius()});
    halo_.setFillColor(sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 46));

    innerHalo_.setRadius(18.f);
    innerHalo_.setOrigin({innerHalo_.getRadius(), innerHalo_.getRadius()});
    innerHalo_.setFillColor(sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 88));

    promptShadow_.setFillColor(sf::Color(0, 0, 0, 88));
    promptPlate_.setFillColor(sf::Color(18, 14, 18, 232));
    promptPlate_.setOutlineThickness(1.5f);
    promptPlate_.setOutlineColor(sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 220));

    configureText(promptText_, 14, sf::Color(245, 233, 214, 255));
    promptText_.setString(config.prompt.empty() ? "Enter to interact" : config.prompt);

    panelShadow_.setFillColor(sf::Color(0, 0, 0, 126));
    panelBack_.setFillColor(sf::Color(17, 12, 16, 242));
    panelBack_.setOutlineThickness(2.f);
    panelBack_.setOutlineColor(sf::Color(accentColor_.r, accentColor_.g, accentColor_.b, 230));
    panelAccent_.setFillColor(accentColor_);

    configureText(titleText_, 26, sf::Color(247, 233, 210, 255));
    titleText_.setStyle(sf::Text::Bold);
    titleText_.setString(config.title);

    configureText(bodyText_, 18, sf::Color(222, 217, 210, 246));
    bodyText_.setLineSpacing(1.08f);
    bodyText_.setString(wrapText(config.body, 44));

    configureText(hintText_, 15, sf::Color(176, 160, 150, 228));
    hintText_.setString("Enter or Esc to close");

    updatePromptLayout();
    updatePanelLayout();
}

WorldInteractable::~WorldInteractable()
{
    if (panelOpen_ && player)
    {
        player->unblockControls();
    }
}

const sf::Texture& WorldInteractable::resolveTexture(const std::string& textureName) const
{
    return resolveInteractiveTexture(*data, textureName);
}

std::string WorldInteractable::wrapText(const std::string& text, std::size_t maxLineLength)
{
    if (text.empty() || maxLineLength == 0)
    {
        return text;
    }

    std::istringstream words(text);
    std::ostringstream wrapped;
    std::string word;
    std::size_t currentLineLength = 0;

    while (words >> word)
    {
        const std::size_t nextLength = currentLineLength == 0
            ? word.size()
            : currentLineLength + 1 + word.size();

        if (currentLineLength > 0 && nextLength > maxLineLength)
        {
            wrapped << '\n' << word;
            currentLineLength = word.size();
            continue;
        }

        if (currentLineLength > 0)
        {
            wrapped << ' ';
            ++currentLineLength;
        }

        wrapped << word;
        currentLineLength += word.size();
    }

    return wrapped.str();
}

void WorldInteractable::updateInteractionState()
{
    revealIfNearby();

    if (!discovered_)
    {
        isCanInteract = false;
        return;
    }

    if (panelOpen_)
    {
        isCanInteract = true;
        return;
    }

    isCanInteract = isInAreaOfInteraction(player->getCenterPosition());
}

void WorldInteractable::revealIfNearby()
{
    if (!hiddenUntilNearby_ || discovered_ || !player)
    {
        return;
    }

    const sf::Vector2f playerCenter = player->getCenterPosition();
    const sf::Vector2f delta = playerCenter - anchorPosition_;
    const float distanceSquared = delta.x * delta.x + delta.y * delta.y;
    if (distanceSquared > revealRadius_ * revealRadius_)
    {
        return;
    }

    discovered_ = true;
    if (manager)
    {
        const std::string revealTitle = revealTitle_.empty()
            ? titleText_.getString().toAnsiString()
            : revealTitle_;
        manager->pushNotification(
            revealTitle,
            revealBody_.empty() ? "A hidden object reveals itself nearby." : revealBody_,
            NotificationTone::Success
        );
    }
}

void WorldInteractable::updateAmbientMotion()
{
    const float time = animationClock_.getElapsedTime().asSeconds();
    const float bob = std::sin(time * 1.65f + animationPhase_) * 6.5f;
    const float sway = std::sin(time * 0.95f + animationPhase_ * 0.7f) * 3.0f;
    const float pulse = 1.f + std::sin(time * 2.55f + animationPhase_ * 0.9f) * 0.055f;
    const float floatScale = 0.985f + std::cos(time * 1.7f + animationPhase_ * 0.6f) * 0.025f;
    const sf::Vector2f spritePosition{anchorPosition_.x + sway, anchorPosition_.y + bob};

    sprite->setPosition(spritePosition);
    sprite->setScale({
        baseScale_.x * pulse,
        baseScale_.y * floatScale
    });

    shadow_.setPosition({anchorPosition_.x, anchorPosition_.y + 18.f});
    shadow_.setScale({
        1.55f * (1.02f - bob * 0.015f),
        0.50f * (1.02f - bob * 0.022f)
    });

    halo_.setPosition({sprite->getGlobalBounds().getCenter().x, sprite->getGlobalBounds().getCenter().y - 10.f});
    halo_.setScale({pulse * 1.18f, pulse * 1.02f});
    innerHalo_.setPosition(halo_.getPosition());
    innerHalo_.setScale({pulse * 0.96f, pulse * 0.92f});

    if (activated_ && singleUse_)
    {
        sprite->setColor(sf::Color(
            static_cast<std::uint8_t>(std::max(40, baseColor_.r - 95)),
            static_cast<std::uint8_t>(std::max(40, baseColor_.g - 95)),
            static_cast<std::uint8_t>(std::max(40, baseColor_.b - 95)),
            baseColor_.a
        ));
    }
}

void WorldInteractable::updatePromptLayout()
{
    const sf::FloatRect textBounds = promptText_.getLocalBounds();
    const sf::Vector2f promptSize = {
        std::max(148.f, textBounds.size.x + 26.f),
        30.f
    };
    const sf::Vector2f promptPos = {
        sprite->getGlobalBounds().getCenter().x - promptSize.x / 2.f,
        sprite->getGlobalBounds().position.y - 42.f
    };

    promptShadow_.setSize(promptSize);
    promptShadow_.setPosition({promptPos.x + 4.f, promptPos.y + 5.f});

    promptPlate_.setSize(promptSize);
    promptPlate_.setPosition(promptPos);

    promptText_.setPosition({
        promptPos.x + (promptSize.x - textBounds.size.x) / 2.f - textBounds.position.x,
        promptPos.y + 5.f - textBounds.position.y
    });
}

void WorldInteractable::updatePanelLayout()
{
    const sf::Vector2f viewCenter = camera->getCameraCenterPos();
    const sf::Vector2f viewSize = camera->getScreenViewSize();
    const sf::Vector2f panelSize = {
        std::min(620.f, viewSize.x - 72.f),
        244.f
    };
    const sf::Vector2f panelPos = {
        viewCenter.x - panelSize.x / 2.f,
        viewCenter.y - panelSize.y / 2.f - 24.f
    };

    panelShadow_.setSize(panelSize);
    panelShadow_.setPosition({panelPos.x + 8.f, panelPos.y + 10.f});

    panelBack_.setSize(panelSize);
    panelBack_.setPosition(panelPos);

    panelAccent_.setSize({panelSize.x, 6.f});
    panelAccent_.setPosition(panelPos);

    titleText_.setPosition({panelPos.x + 20.f, panelPos.y + 16.f});
    bodyText_.setPosition({panelPos.x + 20.f, panelPos.y + 56.f});
    hintText_.setPosition({panelPos.x + 20.f, panelPos.y + panelSize.y - 34.f});
}

void WorldInteractable::openPanel()
{
    if (panelOpen_)
    {
        return;
    }

    panelOpen_ = true;
    player->blockControls();
}

void WorldInteractable::closePanel()
{
    if (!panelOpen_)
    {
        return;
    }

    panelOpen_ = false;
    player->unblockControls();
}

sf::Vector2f WorldInteractable::calculateSpawnPoint() const
{
    if (hasCustomSpawnOffset_)
    {
        return anchorPosition_ + spawnOffset_;
    }

    const sf::FloatRect bounds = sprite->getGlobalBounds();
    const sf::Vector2f playerSize = player->playerRectangle_->getSize();

    return {
        bounds.position.x + bounds.size.x / 2.f - playerSize.x / 2.f,
        bounds.position.y + bounds.size.y - playerSize.y - 14.f
    };
}

void WorldInteractable::performActivation()
{
    const bool canGrantEffect = !activated_ || !singleUse_;

    if (canGrantEffect)
    {
        if (restoreVitality_)
        {
            player->restoreVitalResources();
        }

        if (grantsCheckpoint_)
        {
            manager->setCurrentLevelSpawn(calculateSpawnPoint());
        }

        if (goldReward_ > 0)
        {
            player->addGold(goldReward_);
        }

        activated_ = true;
    }

    if (manager)
    {
        manager->pushNotification(
            titleText_.getString().toAnsiString(),
            activated_ && singleUse_ ? "The find has been woven into your current run." : "Its echo answers again.",
            NotificationTone::Info
        );
    }

    openPanel();
}

void WorldInteractable::draw(sf::RenderWindow& window)
{
    if (!discovered_)
    {
        return;
    }

    window.draw(shadow_);
    window.draw(halo_);
    window.draw(innerHalo_);
    window.draw(*sprite);
}

void WorldInteractable::drawOverlay(sf::RenderWindow& window)
{
    if (!discovered_)
    {
        return;
    }

    if (isCanInteract && !panelOpen_)
    {
        window.draw(promptShadow_);
        window.draw(promptPlate_);
        window.draw(promptText_);
    }

    if (panelOpen_)
    {
        window.draw(panelShadow_);
        window.draw(panelBack_);
        window.draw(panelAccent_);
        window.draw(titleText_);
        window.draw(bodyText_);
        window.draw(hintText_);
    }
}

void WorldInteractable::update()
{
    updateInteractionState();
    updateAmbientMotion();
    updatePromptLayout();

    if (panelOpen_)
    {
        updatePanelLayout();
    }
}

bool WorldInteractable::handleEvent(const sf::Event& event)
{
    if (!discovered_)
    {
        return false;
    }

    if (panelOpen_)
    {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->scancode == kInteractKey || keyPressed->scancode == kCloseKey)
            {
                closePanel();
            }
        }

        return true;
    }

    if (!isCanInteract)
    {
        return false;
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == kInteractKey)
        {
            performActivation();
            return true;
        }
    }

    return false;
}

bool WorldInteractable::blocksPlayerInput() const
{
    return panelOpen_;
}
