#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include <deque>
#include <string>

enum class NotificationTone
{
    Info,
    Success,
    Warning
};

class NotificationFeed
{
public:
    explicit NotificationFeed(sf::Font& font);

    void push(
        std::string title,
        std::string body,
        NotificationTone tone = NotificationTone::Info,
        sf::Time duration = sf::seconds(3.8f)
    );
    void update();
    void draw(sf::RenderWindow& window) const;

private:
    struct NotificationEntry
    {
        std::string title{};
        std::string body{};
        NotificationTone tone = NotificationTone::Info;
        sf::Time duration = sf::seconds(3.8f);
        sf::Clock lifetime{};
    };

    sf::Font* font_ = nullptr;
    std::deque<NotificationEntry> entries_{};

    void trimExpired();
    static std::string clampText(const std::string& text, std::size_t limit);
};
