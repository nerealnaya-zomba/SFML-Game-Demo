#include <DeathRecovery.h>

#include <GameData.h>
#include <Mounting.h>
#include <Player.h>

#include <cmath>
#include <stdexcept>

namespace
{
constexpr float kRecoveryInteractionRadius = 92.f;
constexpr float kRecoveryBeamBaseHeight = 170.f;
constexpr float kRecoveryBeamWidth = 28.f;

void styleRecoveryText(sf::Text& text, unsigned int size, sf::Color fillColor)
{
    text.setCharacterSize(size);
    text.setFillColor(fillColor);
    text.setOutlineThickness(2.f);
    text.setOutlineColor(sf::Color(10, 16, 28, 220));
}
}

DeathRecovery::DeathRecovery(GameData& data, std::string levelName, sf::Vector2f anchorPosition, int goldAmount)
    : levelName_(std::move(levelName))
    , anchorPosition_(anchorPosition)
    , goldAmount_(goldAmount)
    , amountText_(*data.gameFont)
    , promptText_(*data.gameFont)
{
    if (!data.gameFont)
    {
        throw std::runtime_error("DeathRecovery requires a loaded font");
    }

    beam_.setSize({kRecoveryBeamWidth, kRecoveryBeamBaseHeight});
    beam_.setOrigin({beam_.getSize().x / 2.f, beam_.getSize().y});
    beam_.setFillColor(sf::Color(255, 232, 170, 120));

    beamGlow_.setRadius(34.f);
    beamGlow_.setOrigin({beamGlow_.getRadius(), beamGlow_.getRadius()});
    beamGlow_.setFillColor(sf::Color(255, 222, 130, 70));

    coreGlow_.setRadius(12.f);
    coreGlow_.setOrigin({coreGlow_.getRadius(), coreGlow_.getRadius()});
    coreGlow_.setFillColor(sf::Color(255, 239, 186, 220));

    groundRing_.setRadius(36.f);
    groundRing_.setOrigin({groundRing_.getRadius(), groundRing_.getRadius()});
    groundRing_.setFillColor(sf::Color(255, 213, 110, 18));
    groundRing_.setOutlineThickness(3.f);
    groundRing_.setOutlineColor(sf::Color(255, 215, 125, 170));

    promptBackground_.setFillColor(sf::Color(11, 18, 29, 205));
    promptBackground_.setOutlineThickness(1.5f);
    promptBackground_.setOutlineColor(sf::Color(116, 142, 176, 180));

    styleRecoveryText(amountText_, 20, sf::Color(255, 227, 140));
    amountText_.setString("Lost " + std::to_string(goldAmount_) + " gold");

    styleRecoveryText(promptText_, 17, sf::Color(235, 241, 255));
    promptText_.setString("F - Reclaim");

    for (auto& orbitCoin : orbitCoins_)
    {
        orbitCoin.setRadius(5.5f);
        orbitCoin.setOrigin({orbitCoin.getRadius(), orbitCoin.getRadius()});
        orbitCoin.setFillColor(sf::Color(255, 197, 54, 235));
        orbitCoin.setOutlineThickness(1.5f);
        orbitCoin.setOutlineColor(sf::Color(152, 93, 14, 255));
    }

    updateVisuals();
}

void DeathRecovery::update(Player& player)
{
    if (recovered_)
    {
        return;
    }

    const sf::Vector2f playerCenter = player.getCenterPosition();
    const sf::Vector2f delta = playerCenter - anchorPosition_;
    playerInRange_ = (delta.x * delta.x + delta.y * delta.y) <= (kRecoveryInteractionRadius * kRecoveryInteractionRadius);

    updateVisuals();
}

void DeathRecovery::updateVisuals()
{
    const float elapsed = animationClock_.getElapsedTime().asSeconds();
    const float pulse = 0.9f + std::sin(elapsed * 3.6f) * 0.08f;
    const float beamHeight = kRecoveryBeamBaseHeight + std::sin(elapsed * 2.7f) * 10.f;
    const float orbitRadiusX = 34.f + std::sin(elapsed * 1.8f) * 4.f;
    const float orbitRadiusY = 16.f + std::cos(elapsed * 2.2f) * 2.f;

    beam_.setSize({kRecoveryBeamWidth * pulse, beamHeight});
    beam_.setOrigin({beam_.getSize().x / 2.f, beam_.getSize().y});
    beam_.setPosition(anchorPosition_);
    beam_.setFillColor(sf::Color(255, 231, 168, static_cast<std::uint8_t>(118.f * pulse)));

    beamGlow_.setScale({pulse * 1.2f, pulse * 1.2f});
    beamGlow_.setPosition({anchorPosition_.x, anchorPosition_.y - beamHeight + 22.f});

    coreGlow_.setScale({pulse, pulse});
    coreGlow_.setPosition({anchorPosition_.x, anchorPosition_.y - 8.f});

    groundRing_.setScale({1.f + std::sin(elapsed * 2.f) * 0.06f, 0.45f + std::cos(elapsed * 1.7f) * 0.02f});
    groundRing_.setPosition(anchorPosition_);

    for (std::size_t index = 0; index < orbitCoins_.size(); ++index)
    {
        const float angle = elapsed * 1.8f + static_cast<float>(index) * (6.2831853f / static_cast<float>(orbitCoins_.size()));
        orbitCoins_[index].setPosition({
            anchorPosition_.x + std::cos(angle) * orbitRadiusX,
            anchorPosition_.y - 18.f + std::sin(angle) * orbitRadiusY
        });
    }

    setTextOriginToMiddle(amountText_);
    amountText_.setPosition({anchorPosition_.x, anchorPosition_.y - beamHeight - 18.f});

    setTextOriginToMiddle(promptText_);
    promptText_.setPosition({anchorPosition_.x, anchorPosition_.y - beamHeight - 48.f});

    const sf::FloatRect promptBounds = promptText_.getGlobalBounds();
    promptBackground_.setSize({promptBounds.size.x + 22.f, promptBounds.size.y + 12.f});
    setRectangleOriginToMiddle(promptBackground_);
    promptBackground_.setPosition({
        promptText_.getPosition().x,
        promptText_.getPosition().y + promptBounds.size.y * 0.15f
    });
}

void DeathRecovery::draw(sf::RenderWindow& window)
{
    if (recovered_)
    {
        return;
    }

    window.draw(beam_);
    window.draw(beamGlow_);
    window.draw(groundRing_);

    for (auto& orbitCoin : orbitCoins_)
    {
        window.draw(orbitCoin);
    }

    window.draw(coreGlow_);
    window.draw(amountText_);

    if (playerInRange_)
    {
        window.draw(promptBackground_);
        window.draw(promptText_);
    }
}

bool DeathRecovery::handleEvent(const sf::Event& event, Player& player)
{
    if (recovered_ || !playerInRange_)
    {
        return false;
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == BASE_DEATH_RECOVERY_KEY)
        {
            player.addGold(goldAmount_);
            recovered_ = true;
            return true;
        }
    }

    return false;
}

bool DeathRecovery::belongsToLevel(const std::string& levelName) const
{
    return !recovered_ && levelName_ == levelName;
}

bool DeathRecovery::isRecovered() const
{
    return recovered_;
}
