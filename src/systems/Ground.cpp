#include "Ground.h"

#include <GameLevel.h>
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace
{
struct GroundStyleDefinition
{
    std::vector<std::string> surfaceTiles;
    std::vector<std::string> fillTiles;
    std::vector<std::string> accentTiles;
    sf::Color surfaceTint;
    sf::Color supportTint;
    sf::Color glowTint;
    unsigned int accentStride;
};

sf::Color withAlpha(sf::Color color, std::uint8_t alpha)
{
    color.a = alpha;
    return color;
}

const std::unordered_map<std::string, GroundStyleDefinition>& getGroundStyles()
{
    static const std::unordered_map<std::string, GroundStyleDefinition> styles{
        {"VerdantKeep",
            {
                {"TileSetGreen_02.png", "TileSetGreen_03.png", "TileSetGreen_04.png"},
                {"TileSetGreen_05.png", "TileSetGreen_06.png", "TileSetGreen_07.png", "TileSetGreen_12.png"},
                {"TileSetGreen_14.png", "TileSetGreen_15.png", "TileSetGreen_16.png"},
                sf::Color(32, 16, 18, 228),
                sf::Color(68, 34, 38, 170),
                sf::Color(112, 190, 124, 58),
                6u
            }},
        {"ObsidianRunes",
            {
                {"TileSetGreen_27.png", "TileSetGreen_28.png"},
                {"TileSetGreen_29.png", "TileSetGreen_30.png"},
                {"TileSetGreen_28.png", "TileSetGreen_27.png"},
                sf::Color(20, 18, 26, 234),
                sf::Color(46, 40, 60, 185),
                sf::Color(124, 160, 255, 56),
                5u
            }},
        {"CrimsonSanctum",
            {
                {"TileSetGreen_31.png", "TileSetGreen_32.png"},
                {"TileSetGreen_33.png", "TileSetGreen_34.png"},
                {"TileSetGreen_32.png", "TileSetGreen_31.png"},
                sf::Color(38, 12, 16, 234),
                sf::Color(88, 28, 34, 185),
                sf::Color(255, 112, 82, 62),
                4u
            }},
        {"BoneCrypt",
            {
                {"TileSetGreen_35.png", "TileSetGreen_36.png"},
                {"TileSetGreen_37.png", "TileSetGreen_38.png"},
                {"TileSetGreen_36.png", "TileSetGreen_35.png"},
                sf::Color(32, 24, 22, 230),
                sf::Color(86, 74, 66, 175),
                sf::Color(210, 202, 162, 54),
                7u
            }}
    };

    return styles;
}

std::string inferStyleFromGroundName(const std::string& groundFileName)
{
    if (groundFileName == "TileSetGreen_27.png" || groundFileName == "TileSetGreen_28.png" ||
        groundFileName == "TileSetGreen_29.png" || groundFileName == "TileSetGreen_30.png")
    {
        return "ObsidianRunes";
    }

    if (groundFileName == "TileSetGreen_31.png" || groundFileName == "TileSetGreen_32.png" ||
        groundFileName == "TileSetGreen_33.png" || groundFileName == "TileSetGreen_34.png")
    {
        return "CrimsonSanctum";
    }

    if (groundFileName == "TileSetGreen_35.png" || groundFileName == "TileSetGreen_36.png" ||
        groundFileName == "TileSetGreen_37.png" || groundFileName == "TileSetGreen_38.png")
    {
        return "BoneCrypt";
    }

    return "VerdantKeep";
}

void appendUnique(std::vector<std::string>& items, const std::string& value)
{
    if (!value.empty() && std::find(items.begin(), items.end(), value) == items.end())
    {
        items.push_back(value);
    }
}
}

Ground::Ground(GameData& gameTextures,
               GameLevel& level,
               std::string groundFileName,
               unsigned int point_begin,
               unsigned int point_end,
               unsigned int yPos,
               float offSet)
    : groundFileName_(std::move(groundFileName))
    , styleName_(inferStyleFromGroundName(groundFileName_))
    , pointBegin_(point_begin)
    , pointEnd_(point_end)
    , yPos_(yPos)
    , visualDepthRows_(0u)
    , levelWidth_(static_cast<unsigned int>(std::max(0, level.getLevelSize().x)))
    , levelHeight_(static_cast<unsigned int>(std::max(0, level.getLevelSize().y)))
    , offset_(offSet)
{
    attachTexture(gameTextures.TileSetGreenTextures, groundTextures_);

    const sf::Vector2u defaultTileSize = {32u, 32u};
    const sf::Texture* fallbackTexture = findTexture(groundFileName_);
    const sf::Vector2u fallbackSize = fallbackTexture ? fallbackTexture->getSize() : defaultTileSize;

    if (yPos_ == 0u)
    {
        yPos_ = levelHeight_ > fallbackSize.y ? levelHeight_ - fallbackSize.y : 0u;
    }

    rebuildVisualSet();
    rebuildCollisionRect();
}

void Ground::draw(sf::RenderWindow& window)
{
    if (surfaceTextures_.empty())
    {
        return;
    }

    const float spanWidth = static_cast<float>(pointEnd_ > pointBegin_ ? pointEnd_ - pointBegin_ : 0u);
    if (spanWidth <= 0.f)
    {
        return;
    }

    const float startX = static_cast<float>(pointBegin_);
    const float drawTop = static_cast<float>(yPos_);
    const float tileHeight = static_cast<float>(surfaceTextures_.front()->getSize().y);
    const float crestTop = drawTop - 10.f;
    const unsigned int depthRows = resolveVisualDepthRows(tileHeight);
    const float bodyTop = drawTop + tileHeight * 0.55f;
    const float bodyHeight = std::max(44.f, static_cast<float>(levelHeight_) - bodyTop + 48.f);

    sf::RectangleShape glowBand({spanWidth, 18.f});
    glowBand.setPosition({startX, drawTop - 14.f});
    glowBand.setFillColor(glowTint_);
    window.draw(glowBand);

    sf::RectangleShape body({spanWidth, bodyHeight});
    body.setPosition({startX, bodyTop});
    body.setFillColor(surfaceTint_);
    body.setOutlineThickness(1.f);
    body.setOutlineColor(withAlpha(supportTint_, 110));
    window.draw(body);

    const float supportWidth = 14.f;
    const float supportStep = 160.f;
    for (float supportX = startX + 46.f; supportX < startX + spanWidth - 24.f; supportX += supportStep)
    {
        sf::RectangleShape support({supportWidth, bodyHeight - 10.f});
        support.setPosition({supportX, bodyTop + 6.f});
        support.setFillColor(withAlpha(supportTint_, 96));
        support.setOutlineThickness(1.f);
        support.setOutlineColor(withAlpha(glowTint_, 90));
        window.draw(support);
    }

    std::size_t columnIndex = 0u;
    for (float x = startX; x < startX + spanWidth - 0.5f; ++columnIndex)
    {
        const sf::Texture& surfaceTexture = *surfaceTextures_[columnIndex % surfaceTextures_.size()];
        const float tileWidth = static_cast<float>(surfaceTexture.getSize().x);

        for (unsigned int row = 1u; row < depthRows; ++row)
        {
            const sf::Texture& fillTexture = *fillTextures_[(columnIndex + row) % fillTextures_.size()];
            drawTile(window, fillTexture, {x, drawTop + static_cast<float>(row) * tileHeight});
        }

        if (!accentTextures_.empty() && accentStride_ > 0u && columnIndex % accentStride_ == accentStride_ / 2u)
        {
            const sf::Texture& accentTexture = *accentTextures_[columnIndex % accentTextures_.size()];
            drawTile(window, accentTexture, {x, crestTop}, withAlpha(sf::Color::White, 236));
        }

        drawTile(window, surfaceTexture, {x, drawTop});
        x += tileWidth;
    }
}

void Ground::clearGround()
{
    surfaceTextures_.clear();
    fillTextures_.clear();
    accentTextures_.clear();
}

sf::RectangleShape& Ground::getRect()
{
    return groundRect_;
}

const sf::RectangleShape& Ground::getRect() const
{
    return groundRect_;
}

void Ground::setOffset(float offset)
{
    offset_ = offset;
    rebuildCollisionRect();
}

void Ground::setSpan(unsigned int pointBegin, unsigned int pointEnd)
{
    pointBegin_ = pointBegin;
    pointEnd_ = pointEnd;
    rebuildCollisionRect();
}

void Ground::setYPos(unsigned int yPos)
{
    yPos_ = yPos;
    rebuildCollisionRect();
}

void Ground::setVisualDepthRows(unsigned int rows)
{
    visualDepthRows_ = rows;
}

bool Ground::setStyle(const std::string& styleName)
{
    if (styleName.empty())
    {
        styleName_ = inferStyleFromGroundName(groundFileName_);
        rebuildVisualSet();
        return true;
    }

    if (getGroundStyles().find(styleName) == getGroundStyles().end())
    {
        return false;
    }

    styleName_ = styleName;
    rebuildVisualSet();
    return true;
}

float Ground::getSurfaceY() const
{
    return static_cast<float>(yPos_) + offset_;
}

sf::FloatRect Ground::getSurfaceBounds() const
{
    return groundRect_.getGlobalBounds();
}

float Ground::getCameraClampRight() const
{
    const float pointBegin = static_cast<float>(pointBegin_);
    const float pointEnd = static_cast<float>(pointEnd_);
    if (surfaceTextures_.empty())
    {
        return pointEnd;
    }

    const float tileWidth = static_cast<float>(surfaceTextures_.front()->getSize().x);
    if (tileWidth <= 0.f)
    {
        return pointEnd;
    }

    const float spanWidth = pointEnd - pointBegin;
    const float remainder = std::fmod(spanWidth, tileWidth);
    if (remainder <= 0.01f || std::abs(remainder - tileWidth) <= 0.01f)
    {
        return pointEnd;
    }

    return std::max(pointBegin, pointEnd - (tileWidth - remainder));
}

bool Ground::containsX(float x) const
{
    return x >= static_cast<float>(pointBegin_) && x <= static_cast<float>(pointEnd_);
}

std::string Ground::getStyleName() const
{
    return styleName_;
}

const sf::Texture* Ground::findTexture(const std::string& textureName) const
{
    if (!groundTextures_)
    {
        return nullptr;
    }

    const auto textureIt = groundTextures_->find(textureName);
    if (textureIt == groundTextures_->end())
    {
        return nullptr;
    }

    return &textureIt->second;
}

void Ground::rebuildCollisionRect()
{
    const float width = static_cast<float>(pointEnd_ > pointBegin_ ? pointEnd_ - pointBegin_ : 0u);
    const float groundTop = getSurfaceY();
    const float height = std::max(32.f, static_cast<float>(levelHeight_) - groundTop + 64.f);

    groundRect_.setSize({width, height});
    groundRect_.setPosition({static_cast<float>(pointBegin_), groundTop});
    groundRect_.setFillColor(sf::Color::Transparent);
}

void Ground::rebuildVisualSet()
{
    surfaceTextures_.clear();
    fillTextures_.clear();
    accentTextures_.clear();

    const auto styles = getGroundStyles();
    const auto styleIt = styles.find(styleName_);
    const GroundStyleDefinition* style = styleIt != styles.end() ? &styleIt->second : nullptr;

    std::vector<std::string> surfaceNames;
    std::vector<std::string> fillNames;
    std::vector<std::string> accentNames;

    appendUnique(surfaceNames, groundFileName_);
    if (style)
    {
        for (const std::string& name : style->surfaceTiles)
        {
            appendUnique(surfaceNames, name);
        }
        for (const std::string& name : style->fillTiles)
        {
            appendUnique(fillNames, name);
        }
        for (const std::string& name : style->accentTiles)
        {
            appendUnique(accentNames, name);
        }

        surfaceTint_ = style->surfaceTint;
        supportTint_ = style->supportTint;
        glowTint_ = style->glowTint;
        accentStride_ = style->accentStride;
    }

    for (const std::string& name : surfaceNames)
    {
        if (const sf::Texture* texture = findTexture(name))
        {
            surfaceTextures_.push_back(texture);
        }
    }

    for (const std::string& name : fillNames)
    {
        if (const sf::Texture* texture = findTexture(name))
        {
            fillTextures_.push_back(texture);
        }
    }

    for (const std::string& name : accentNames)
    {
        if (const sf::Texture* texture = findTexture(name))
        {
            accentTextures_.push_back(texture);
        }
    }

    if (surfaceTextures_.empty())
    {
        if (const sf::Texture* fallbackTexture = findTexture(groundFileName_))
        {
            surfaceTextures_.push_back(fallbackTexture);
        }
        else if (groundTextures_ && !groundTextures_->empty())
        {
            surfaceTextures_.push_back(&groundTextures_->begin()->second);
        }
    }

    if (fillTextures_.empty())
    {
        fillTextures_ = surfaceTextures_;
    }

    if (accentTextures_.empty())
    {
        accentTextures_ = surfaceTextures_;
    }
}

unsigned int Ground::resolveVisualDepthRows(float tileHeight) const
{
    if (visualDepthRows_ > 0u)
    {
        return visualDepthRows_;
    }

    const float remainingHeight = std::max(0.f, static_cast<float>(levelHeight_) - static_cast<float>(yPos_));
    const float rowHeight = std::max(1.f, tileHeight);
    return std::max(2u, static_cast<unsigned int>(std::ceil(remainingHeight / rowHeight)));
}

void Ground::drawTile(sf::RenderWindow& window,
                      const sf::Texture& texture,
                      const sf::Vector2f& position,
                      sf::Color color) const
{
    sf::Sprite sprite(texture);
    sprite.setPosition({std::round(position.x), std::round(position.y)});
    sprite.setColor(color);
    window.draw(sprite);
}
