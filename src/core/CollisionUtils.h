#pragma once

#include <SFML/Graphics.hpp>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace collision
{
constexpr float kPlatformLandingHorizontalInset = 1.f;
constexpr float kPlatformSupportHorizontalInset = 1.f;
constexpr float kGroundHorizontalInset = 2.f;

struct MoveResult
{
    bool landed = false;
    bool hitCeiling = false;
    bool blockedLeft = false;
    bool blockedRight = false;
    bool touchedGround = false;
    const sf::RectangleShape* supportRect = nullptr;
};

inline float left(const sf::FloatRect& rect)
{
    return rect.position.x;
}

inline float right(const sf::FloatRect& rect)
{
    return rect.position.x + rect.size.x;
}

inline float top(const sf::FloatRect& rect)
{
    return rect.position.y;
}

inline float bottom(const sf::FloatRect& rect)
{
    return rect.position.y + rect.size.y;
}

inline bool overlapsOnAxis(
    const float minA,
    const float maxA,
    const float minB,
    const float maxB,
    const float inset = 0.f
)
{
    return maxA > minB + inset && minA < maxB - inset;
}

inline bool overlapsHorizontally(const sf::FloatRect& lhs, const sf::FloatRect& rhs, const float inset = 2.f)
{
    return overlapsOnAxis(left(lhs), right(lhs), left(rhs), right(rhs), inset);
}

inline bool overlapsVertically(const sf::FloatRect& lhs, const sf::FloatRect& rhs, const float inset = 2.f)
{
    return overlapsOnAxis(top(lhs), bottom(lhs), top(rhs), bottom(rhs), inset);
}

inline const sf::RectangleShape* findSupportingPlatform(
    const sf::RectangleShape& body,
    const std::vector<std::shared_ptr<sf::RectangleShape>>& platforms,
    const float tolerance = 4.f
)
{
    const sf::FloatRect bodyBounds = body.getGlobalBounds();
    const float bodyBottom = bottom(bodyBounds);
    const sf::RectangleShape* support = nullptr;
    float bestGap = std::numeric_limits<float>::max();

    for (const auto& platform : platforms)
    {
        if (!platform)
        {
            continue;
        }

        const sf::FloatRect platformBounds = platform->getGlobalBounds();
        if (!overlapsHorizontally(bodyBounds, platformBounds, kPlatformSupportHorizontalInset))
        {
            continue;
        }

        const float gap = std::abs(bodyBottom - top(platformBounds));
        if (gap <= tolerance && gap < bestGap)
        {
            bestGap = gap;
            support = platform.get();
        }
    }

    return support;
}

inline bool isStandingOnGround(
    const sf::RectangleShape& body,
    const sf::RectangleShape& ground,
    const float tolerance = 2.5f
)
{
    const sf::FloatRect bodyBounds = body.getGlobalBounds();
    const sf::FloatRect groundBounds = ground.getGlobalBounds();
    return overlapsHorizontally(bodyBounds, groundBounds, kGroundHorizontalInset) &&
        std::abs(bottom(bodyBounds) - top(groundBounds)) <= tolerance;
}

inline MoveResult moveBodyWithWorldCollisions(
    sf::RectangleShape& body,
    const sf::Vector2f delta,
    const std::vector<std::shared_ptr<sf::RectangleShape>>& platforms,
    const sf::RectangleShape* ground,
    const float worldWidth,
    const float maxStepDistance = 6.f,
    const float worldLeft = 0.f
)
{
    MoveResult result;
    const float longestAxis = std::max(std::abs(delta.x), std::abs(delta.y));
    const int stepCount = std::max(1, static_cast<int>(std::ceil(longestAxis / std::max(1.f, maxStepDistance))));
    float remainingX = delta.x;
    float remainingY = delta.y;

    for (int stepIndex = 0; stepIndex < stepCount; ++stepIndex)
    {
        const float stepY = (stepIndex == stepCount - 1) ? remainingY : delta.y / static_cast<float>(stepCount);
        remainingY -= stepY;

        if (std::abs(stepY) > 0.0001f)
        {
            const sf::FloatRect previousBounds = body.getGlobalBounds();
            body.move({0.f, stepY});
            const sf::FloatRect movedBounds = body.getGlobalBounds();

            if (stepY > 0.f)
            {
                const sf::RectangleShape* bestSupport = nullptr;
                float bestTop = std::numeric_limits<float>::max();

                if (ground != nullptr)
                {
                    const sf::FloatRect groundBounds = ground->getGlobalBounds();
                    const bool crossedGroundTop =
                        overlapsHorizontally(movedBounds, groundBounds, kGroundHorizontalInset) &&
                        bottom(previousBounds) <= top(groundBounds) + 0.75f &&
                        bottom(movedBounds) >= top(groundBounds) - 0.75f;
                    const bool intersectsGround =
                        overlapsHorizontally(movedBounds, groundBounds, kGroundHorizontalInset) &&
                        bottom(movedBounds) >= top(groundBounds) &&
                        top(movedBounds) < top(groundBounds);

                    if (crossedGroundTop || intersectsGround)
                    {
                        bestTop = top(groundBounds);
                    }
                }

                for (const auto& platform : platforms)
                {
                    if (!platform)
                    {
                        continue;
                    }

                    const sf::FloatRect platformBounds = platform->getGlobalBounds();
                    const bool crossedPlatformTop =
                        overlapsHorizontally(movedBounds, platformBounds, kPlatformLandingHorizontalInset) &&
                        bottom(previousBounds) <= top(platformBounds) + 0.75f &&
                        bottom(movedBounds) >= top(platformBounds) - 0.75f;
                    const bool intersectsPlatform =
                        overlapsHorizontally(movedBounds, platformBounds, kPlatformLandingHorizontalInset) &&
                        bottom(movedBounds) >= top(platformBounds) &&
                        top(movedBounds) < top(platformBounds) &&
                        movedBounds.findIntersection(platformBounds).has_value();

                    if ((crossedPlatformTop || intersectsPlatform) && top(platformBounds) < bestTop)
                    {
                        bestTop = top(platformBounds);
                        bestSupport = platform.get();
                    }
                }

                if (bestSupport != nullptr || bestTop < std::numeric_limits<float>::max())
                {
                    body.setPosition({body.getPosition().x, bestTop - body.getSize().y});
                    result.landed = true;
                    result.touchedGround = bestSupport == nullptr;
                    result.supportRect = bestSupport;
                    remainingY = 0.f;
                }
            }
            else
            {
                float bestBottom = -std::numeric_limits<float>::max();

                for (const auto& platform : platforms)
                {
                    if (!platform)
                    {
                        continue;
                    }

                    const sf::FloatRect platformBounds = platform->getGlobalBounds();
                    const bool crossedPlatformBottom =
                        overlapsHorizontally(movedBounds, platformBounds, 4.f) &&
                        top(previousBounds) >= bottom(platformBounds) - 0.75f &&
                        top(movedBounds) <= bottom(platformBounds) + 0.75f;
                    const bool intersectsPlatform =
                        overlapsHorizontally(movedBounds, platformBounds, 4.f) &&
                        top(movedBounds) <= bottom(platformBounds) &&
                        bottom(movedBounds) > bottom(platformBounds) &&
                        movedBounds.findIntersection(platformBounds).has_value();

                    if ((crossedPlatformBottom || intersectsPlatform) && bottom(platformBounds) > bestBottom)
                    {
                        bestBottom = bottom(platformBounds);
                    }
                }

                if (bestBottom > -std::numeric_limits<float>::max())
                {
                    body.setPosition({body.getPosition().x, bestBottom});
                    result.hitCeiling = true;
                    remainingY = 0.f;
                }
            }
        }

        const float stepX = (stepIndex == stepCount - 1) ? remainingX : delta.x / static_cast<float>(stepCount);
        remainingX -= stepX;

        if (std::abs(stepX) > 0.0001f)
        {
            const sf::FloatRect previousBounds = body.getGlobalBounds();
            body.move({stepX, 0.f});
            const sf::FloatRect movedBounds = body.getGlobalBounds();

            if (stepX > 0.f)
            {
                float bestLeft = std::numeric_limits<float>::max();

                for (const auto& platform : platforms)
                {
                    if (!platform)
                    {
                        continue;
                    }

                    const sf::FloatRect platformBounds = platform->getGlobalBounds();
                    const bool crossedPlatformSide =
                        overlapsVertically(movedBounds, platformBounds, 3.f) &&
                        right(previousBounds) <= left(platformBounds) + 0.75f &&
                        right(movedBounds) >= left(platformBounds) - 0.75f;
                    const bool intersectsPlatform =
                        overlapsVertically(movedBounds, platformBounds, 3.f) &&
                        movedBounds.findIntersection(platformBounds).has_value();

                    if ((crossedPlatformSide || intersectsPlatform) && left(platformBounds) < bestLeft)
                    {
                        bestLeft = left(platformBounds);
                    }
                }

                if (bestLeft < std::numeric_limits<float>::max())
                {
                    body.setPosition({bestLeft - body.getSize().x, body.getPosition().y});
                    result.blockedRight = true;
                    remainingX = 0.f;
                }
            }
            else
            {
                float bestRight = -std::numeric_limits<float>::max();

                for (const auto& platform : platforms)
                {
                    if (!platform)
                    {
                        continue;
                    }

                    const sf::FloatRect platformBounds = platform->getGlobalBounds();
                    const bool crossedPlatformSide =
                        overlapsVertically(movedBounds, platformBounds, 3.f) &&
                        left(previousBounds) >= right(platformBounds) - 0.75f &&
                        left(movedBounds) <= right(platformBounds) + 0.75f;
                    const bool intersectsPlatform =
                        overlapsVertically(movedBounds, platformBounds, 3.f) &&
                        movedBounds.findIntersection(platformBounds).has_value();

                    if ((crossedPlatformSide || intersectsPlatform) && right(platformBounds) > bestRight)
                    {
                        bestRight = right(platformBounds);
                    }
                }

                if (bestRight > -std::numeric_limits<float>::max())
                {
                    body.setPosition({bestRight, body.getPosition().y});
                    result.blockedLeft = true;
                    remainingX = 0.f;
                }
            }
        }

        const float minX = worldLeft;
        const float maxX = worldLeft + std::max(0.f, worldWidth - body.getSize().x);
        if (body.getPosition().x < minX)
        {
            body.setPosition({minX, body.getPosition().y});
            result.blockedLeft = true;
            remainingX = 0.f;
        }
        else if (body.getPosition().x > maxX)
        {
            body.setPosition({maxX, body.getPosition().y});
            result.blockedRight = true;
            remainingX = 0.f;
        }
    }

    return result;
}
}
