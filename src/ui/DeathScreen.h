#pragma once

#include <SFML/Graphics.hpp>

#include <optional>

enum class DeathScreenAction
{
    RestartLevel,
    ReturnToMainMenu
};

class DeathScreen
{
public:
    DeathScreen(sf::RenderWindow& window, sf::Font& font);

    void open();
    void close();
    void update();
    void draw();

    [[nodiscard]] std::optional<DeathScreenAction> handleEvent(const sf::Event& event);
    [[nodiscard]] bool isOpen() const;

private:
    void refreshLayout();
    void refreshVisuals(float elapsedSeconds);
    void moveSelection(int delta);

    sf::RenderWindow* window_ = nullptr;

    bool isOpen_ = false;
    int selectedOption_ = 0;

    sf::Clock animationClock_;

    sf::RectangleShape titleBand_;
    sf::RectangleShape optionHighlight_;
    sf::RectangleShape dividerLine_;

    sf::Text titleText_;
    sf::Text descriptionText_;
    sf::Text restartOptionText_;
    sf::Text mainMenuOptionText_;
    sf::Text hintText_;
};
