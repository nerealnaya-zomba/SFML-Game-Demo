#pragma once

#include <Defines.h>
#include <GameData.h>
#include <Mounting.h>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

inline constexpr float BASE_GROUND_OFFSET = 8.f;

class GameLevel;

class Ground
{
public:
    Ground(GameData& gameTextures,
           GameLevel& level,
           std::string groundFileName,
           unsigned int point_begin,
           unsigned int point_end,
           unsigned int yPos = 0u,
           float offSet = BASE_GROUND_OFFSET);
    ~Ground() = default;

    void draw(sf::RenderWindow& window);
    void clearGround();

    sf::RectangleShape& getRect();
    const sf::RectangleShape& getRect() const;

    void setOffset(float offset);
    void setSpan(unsigned int pointBegin, unsigned int pointEnd);
    void setYPos(unsigned int yPos);
    void setVisualDepthRows(unsigned int rows);
    bool setStyle(const std::string& styleName);

    float getSurfaceY() const;
    sf::FloatRect getSurfaceBounds() const;
    bool containsX(float x) const;
    std::string getStyleName() const;

private:
    sf::RectangleShape groundRect_{};
    std::string groundFileName_{};
    std::string styleName_{};

    unsigned int pointBegin_{};
    unsigned int pointEnd_{};
    unsigned int yPos_{};
    unsigned int visualDepthRows_{};
    unsigned int levelWidth_{};
    unsigned int levelHeight_{};
    unsigned int accentStride_{5u};

    float offset_{BASE_GROUND_OFFSET};

    std::map<std::string, sf::Texture>* groundTextures_{};
    std::vector<const sf::Texture*> surfaceTextures_{};
    std::vector<const sf::Texture*> fillTextures_{};
    std::vector<const sf::Texture*> accentTextures_{};

    sf::Color surfaceTint_{34, 16, 20, 220};
    sf::Color supportTint_{74, 32, 38, 170};
    sf::Color glowTint_{180, 78, 60, 54};

    const sf::Texture* findTexture(const std::string& textureName) const;
    void rebuildCollisionRect();
    void rebuildVisualSet();
    unsigned int resolveVisualDepthRows(float tileHeight) const;
    void drawTile(sf::RenderWindow& window,
                  const sf::Texture& texture,
                  const sf::Vector2f& position,
                  sf::Color color = sf::Color::White) const;
};
