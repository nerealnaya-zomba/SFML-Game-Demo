#include <GoldCoin.h>

#include <Ground.h>
#include <Mounting.h>
#include <Platform.h>
#include <Player.h>

#include <algorithm>
#include <cmath>

namespace
{
constexpr float kCoinGravity = 0.24f;
constexpr float kCoinMaxFallSpeed = 5.5f;
constexpr float kCoinPickupPadding = 6.f;
constexpr float kCoinGroundFriction = 0.93f;
}

GoldCoin::GoldCoin(sf::Vector2f position, int value, sf::Vector2f initialVelocity)
    : position_(position)
    , velocity_(initialVelocity)
    , value_(value)
    , phaseOffset_(random(0.f, 6.28f))
{
    glow_.setRadius(radius_ + 5.f);
    glow_.setOrigin({glow_.getRadius(), glow_.getRadius()});
    glow_.setFillColor(sf::Color(255, 196, 64, 65));

    outerCoin_.setRadius(radius_);
    outerCoin_.setOrigin({outerCoin_.getRadius(), outerCoin_.getRadius()});
    outerCoin_.setFillColor(sf::Color(255, 193, 45));
    outerCoin_.setOutlineThickness(2.f);
    outerCoin_.setOutlineColor(sf::Color(173, 103, 10));

    innerCoin_.setRadius(radius_ - 2.5f);
    innerCoin_.setOrigin({innerCoin_.getRadius(), innerCoin_.getRadius()});
    innerCoin_.setFillColor(sf::Color(255, 231, 132));

    shine_.setSize({radius_ * 0.65f, radius_ * 1.4f});
    setRectangleOriginToMiddle(shine_);
    shine_.setFillColor(sf::Color(255, 252, 219, 220));
    shine_.setRotation(sf::degrees(-24.f));

    updateVisuals();
}

sf::FloatRect GoldCoin::getBoundsAt(const sf::Vector2f& position) const
{
    return {
        {position.x - radius_, position.y - radius_},
        {radius_ * 2.f, radius_ * 2.f}
    };
}

void GoldCoin::updateVisuals()
{
    const float elapsed = animationClock_.getElapsedTime().asSeconds();
    const float bobOffset = grounded_ ? std::sin(elapsed * 4.5f + phaseOffset_) * 1.8f : 0.f;
    const float pulse = 0.92f + std::sin(elapsed * 5.f + phaseOffset_) * 0.08f;

    glow_.setScale({pulse, pulse});
    shine_.setRotation(sf::degrees(-24.f + std::sin(elapsed * 6.f + phaseOffset_) * 5.f));

    const sf::Vector2f visualPos = {position_.x, position_.y + bobOffset};
    glow_.setPosition(visualPos);
    outerCoin_.setPosition(visualPos);
    innerCoin_.setPosition(visualPos);
    shine_.setPosition({visualPos.x - radius_ * 0.1f, visualPos.y - radius_ * 0.05f});
}

void GoldCoin::resolveGroundCollision(Ground &ground)
{
    sf::FloatRect coinBounds = getBoundsAt(position_);
    const float groundTop = ground.getRect().getPosition().y;
    const float coinBottom = coinBounds.position.y + coinBounds.size.y;

    if(coinBottom >= groundTop)
    {
        position_.y = groundTop - radius_;
        velocity_.y = 0.f;
        grounded_ = true;
    }
}

void GoldCoin::resolvePlatformCollision(Platform &platform)
{
    for (auto&& platformRect : platform.getRects())
    {
        sf::FloatRect coinBounds = getBoundsAt(position_);
        const sf::FloatRect platformBounds = platformRect->getGlobalBounds();

        if(!coinBounds.findIntersection(platformBounds))
        {
            continue;
        }

        const float previousBottom = coinBounds.position.y + coinBounds.size.y - velocity_.y;
        if(previousBottom <= platformBounds.position.y + 6.f && velocity_.y >= 0.f)
        {
            position_.y = platformBounds.position.y - radius_;
            velocity_.y = 0.f;
            grounded_ = true;
        }
    }
}

void GoldCoin::update(Ground &ground, Platform &platform, Player &player, const sf::FloatRect& levelBounds)
{
    if(collected_)
    {
        return;
    }

    grounded_ = false;
    velocity_.y = std::min(velocity_.y + kCoinGravity, kCoinMaxFallSpeed);
    position_ += velocity_;

    if(position_.x - radius_ <= levelBounds.position.x)
    {
        position_.x = levelBounds.position.x + radius_;
        velocity_.x *= -0.35f;
    }
    else if(position_.x + radius_ >= levelBounds.position.x + levelBounds.size.x)
    {
        position_.x = levelBounds.position.x + levelBounds.size.x - radius_;
        velocity_.x *= -0.35f;
    }

    resolveGroundCollision(ground);
    resolvePlatformCollision(platform);

    if(grounded_)
    {
        velocity_.x *= kCoinGroundFriction;
        if(std::abs(velocity_.x) < 0.05f)
        {
            velocity_.x = 0.f;
        }
    }

    sf::FloatRect pickupBounds = getBoundsAt(position_);
    pickupBounds.position.x -= kCoinPickupPadding;
    pickupBounds.position.y -= kCoinPickupPadding;
    pickupBounds.size.x += kCoinPickupPadding * 2.f;
    pickupBounds.size.y += kCoinPickupPadding * 2.f;

    if(pickupBounds.findIntersection(player.playerRectangle_->getGlobalBounds()))
    {
        player.addGold(value_);
        collected_ = true;
        return;
    }

    updateVisuals();
}

void GoldCoin::draw(sf::RenderWindow &window)
{
    if(collected_)
    {
        return;
    }

    window.draw(glow_);
    window.draw(outerCoin_);
    window.draw(innerCoin_);
    window.draw(shine_);
}

bool GoldCoin::isCollected() const
{
    return collected_;
}
