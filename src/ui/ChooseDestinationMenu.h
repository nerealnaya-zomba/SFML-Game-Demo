#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

class GameCamera;
class GameData;
class GameLevel;
class GameLevelManager;
class Player;

const sf::Vector2i BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN = {18, 18};
const sf::Vector2f BASE_DESTINATION_ICON_SIZE = {140.f, 82.f};

const sf::Color BASE_SELECTION_COLOR = sf::Color(228, 194, 142, 255);
const float BASE_SELECTION_SIZE = 3.f;

const sf::Color BASE_LEVELMARK_COLOR = sf::Color(190, 82, 64, 255);
const float BASE_LEVELMARK_SIZE = 2.f;

const float BASE_DESTINATION_BACKGROUND_TOPMARGIN = 42.f;

class ChooseDestinationMenu
{
private:
    GameData* data = nullptr;
    GameCamera* camera = nullptr;
    GameLevelManager* manager = nullptr;
    Player* player = nullptr;

    bool isOpened = false;
    std::optional<std::string> desiredDestination{};

    sf::Keyboard::Scan moveLeftKey;
    sf::Keyboard::Scan moveRightKey;
    sf::Keyboard::Scan selectKey;
    sf::Keyboard::Scan closeKey;

public:
    struct LevelDestination
    {
        bool isOpened = true;
        bool isVisible = true;
        bool isSelected = false;
        bool isChoosed = false;
        bool isPlayerThere = false;

        const GameLevel* level = nullptr;
    };

    struct LevelDestinationRect
    {
        explicit LevelDestinationRect(const sf::Texture& iconTexture)
            : icon(iconTexture)
        {
        }

        LevelDestination leveldestination{};
        sf::Sprite icon;
        sf::RectangleShape selectionRect;
        sf::RectangleShape currentLevelMarkRect;

        void draw(sf::RenderWindow& w) const;
    };

private:
    sf::RectangleShape overlayVeil;
    sf::RectangleShape panelShadow;
    sf::RectangleShape panelFrame;
    sf::RectangleShape panelInset;
    sf::RectangleShape titleBand;
    sf::RectangleShape footerBand;
    sf::RectangleShape previewFrame;
    sf::RectangleShape previewInset;
    sf::RectangleShape previewVeil;

    sf::Text titleText;
    sf::Text subtitleText;
    sf::Text displayingLevelName;
    sf::Text destinationStateText;
    sf::Text destinationDescriptionText;
    sf::Text legendText;

    std::vector<LevelDestinationRect> levels;
    std::vector<LevelDestinationRect>::iterator levelIt;

    sf::Vector2f panelPosition{0.f, 0.f};
    sf::Vector2f panelSize{1120.f, 408.f};
    sf::Vector2f previewPosition{0.f, 0.f};
    sf::Vector2f previewSize{420.f, 176.f};
    float animationTime = 0.f;

    void moveLevelItLeft();
    void moveLevelItRight();

    void positioningLevelDestinations();
    void positioningLevelDestinationsBackground();
    void positioningLevelDestinationsLevels();
    void positioningLevelDestinationsText();

    void checkWherePlayer();
    void updateDisplayedTexts();

    void drawLevelDestinations(sf::RenderWindow& window);
    void drawLevelDestinationsBackground(sf::RenderWindow& window);
    void drawLevelDestinationsLevels(sf::RenderWindow& window);
    void drawLevelDestinationsText(sf::RenderWindow& window);
    void drawControlHints(sf::RenderWindow& window);

    void currentSelectedElementToDesiredDestination();
    std::string getCurrentLevelName();
    void mountSelectionRect(sf::RectangleShape& sr, const sf::FloatRect& bounds);
    void mountCurrentLevelMarkRect(sf::RectangleShape& sr, const sf::FloatRect& bounds);
    void setDisplayingLevelNameString(const std::string& str);
    void applyIconScale(sf::Sprite& icon);
    void initializeIsChoosed();

    void handleMoveEvents(const sf::Event& ev);
    void handleActivateEvent(const sf::Event& ev);
    void handleCloseEvent(const sf::Event& ev);

public:
    ChooseDestinationMenu(const ChooseDestinationMenu&) = default;
    ChooseDestinationMenu(ChooseDestinationMenu&&) = delete;
    ChooseDestinationMenu& operator=(const ChooseDestinationMenu&) = default;
    ChooseDestinationMenu& operator=(ChooseDestinationMenu&&) = delete;
    ChooseDestinationMenu(
        GameData& d,
        GameCamera& c,
        GameLevelManager& lm,
        Player& p,
        sf::Keyboard::Scan moveLeftKey,
        sf::Keyboard::Scan moveRightKey,
        sf::Keyboard::Scan selectKey,
        sf::Keyboard::Scan closeKey
    );
    ~ChooseDestinationMenu() = default;

    void addLevelInVector(const GameLevel& level, sf::Texture& icon);
    void addLevelInVector(const GameLevel& level, const sf::Texture& icon);
    void open();
    void close();

    void handleEvents(const sf::Event& ev);
    void update();
    void draw(sf::RenderWindow& w);

    bool getIsOpened();
    std::optional<std::string> getSelectedLevel();
};
