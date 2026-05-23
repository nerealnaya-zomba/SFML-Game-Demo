#include <NotificationFeed.h>

#include <Localization.h>

#include <algorithm>
#include <array>
#include <utility>

namespace
{
struct NotificationPalette
{
    sf::Color frame;
    sf::Color fill;
    sf::Color accent;
    sf::Color title;
    sf::Color body;
};

constexpr float kNotificationWidth = 360.f;
constexpr float kNotificationHeight = 72.f;
constexpr float kNotificationGap = 12.f;
constexpr float kNotificationMargin = 24.f;
constexpr std::size_t kMaxNotifications = 4;

NotificationPalette getPalette(NotificationTone tone)
{
    switch (tone)
    {
        case NotificationTone::Success:
            return {
                sf::Color(96, 160, 120, 255),
                sf::Color(12, 24, 18, 236),
                sf::Color(118, 214, 160, 255),
                sf::Color(214, 246, 220, 255),
                sf::Color(202, 229, 210, 255)
            };
        case NotificationTone::Warning:
            return {
                sf::Color(176, 110, 76, 255),
                sf::Color(22, 12, 10, 236),
                sf::Color(235, 156, 98, 255),
                sf::Color(255, 221, 186, 255),
                sf::Color(238, 219, 204, 255)
            };
        case NotificationTone::Info:
        default:
            return {
                sf::Color(82, 113, 154, 255),
                sf::Color(10, 14, 21, 236),
                sf::Color(126, 174, 234, 255),
                sf::Color(219, 233, 255, 255),
                sf::Color(214, 221, 232, 255)
            };
    }
}

float computeVisibility(const float elapsedSeconds, const float durationSeconds)
{
    if (durationSeconds <= 0.f)
    {
        return 0.f;
    }

    float alphaFactor = 1.f;
    if (elapsedSeconds < 0.18f)
    {
        alphaFactor = std::clamp(elapsedSeconds / 0.18f, 0.f, 1.f);
    }
    else if (elapsedSeconds > durationSeconds - 0.45f)
    {
        alphaFactor = std::clamp((durationSeconds - elapsedSeconds) / 0.45f, 0.f, 1.f);
    }

    return alphaFactor;
}
}

NotificationFeed::NotificationFeed(sf::Font& font)
    : font_(&font)
{
}

void NotificationFeed::push(
    std::string title,
    std::string body,
    const NotificationTone tone,
    const sf::Time duration
)
{
    trimExpired();

    NotificationEntry entry;
    entry.title = clampText(std::move(title), 34u);
    entry.body = clampText(std::move(body), 68u);
    entry.tone = tone;
    entry.duration = duration;
    entry.lifetime.restart();

    entries_.push_front(std::move(entry));
    while (entries_.size() > kMaxNotifications)
    {
        entries_.pop_back();
    }
}

void NotificationFeed::update()
{
    trimExpired();
}

void NotificationFeed::draw(sf::RenderWindow& window) const
{
    if (font_ == nullptr || entries_.empty())
    {
        return;
    }

    const sf::View& currentView = window.getView();
    const sf::Vector2f viewSize = currentView.getSize();
    const sf::Vector2f viewCenter = currentView.getCenter();
    const sf::Vector2f viewTopLeft = {
        viewCenter.x - viewSize.x * 0.5f,
        viewCenter.y - viewSize.y * 0.5f
    };

    const float width = std::min(kNotificationWidth, std::max(240.f, viewSize.x - kNotificationMargin * 2.f));
    float y = viewTopLeft.y + viewSize.y - kNotificationMargin - kNotificationHeight;

    for (const auto& entry : entries_)
    {
        const float elapsedSeconds = entry.lifetime.getElapsedTime().asSeconds();
        const float durationSeconds = entry.duration.asSeconds();
        const float visibility = computeVisibility(elapsedSeconds, durationSeconds);
        if (visibility <= 0.f)
        {
            continue;
        }

        const NotificationPalette palette = getPalette(entry.tone);
        const std::uint8_t alpha = static_cast<std::uint8_t>(std::clamp(visibility * 255.f, 0.f, 255.f));
        const float slideOffset = (1.f - visibility) * 26.f;
        const float x = viewTopLeft.x + viewSize.x - width - kNotificationMargin + slideOffset;

        sf::RectangleShape shadow({width, kNotificationHeight});
        shadow.setPosition({x + 6.f, y + 7.f});
        shadow.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(82.f * visibility)));

        sf::RectangleShape panel({width, kNotificationHeight});
        panel.setPosition({x, y});
        panel.setFillColor(sf::Color(palette.fill.r, palette.fill.g, palette.fill.b, alpha));
        panel.setOutlineThickness(2.f);
        panel.setOutlineColor(sf::Color(palette.frame.r, palette.frame.g, palette.frame.b, alpha));

        sf::RectangleShape accent({width, 6.f});
        accent.setPosition({x, y});
        accent.setFillColor(sf::Color(palette.accent.r, palette.accent.g, palette.accent.b, alpha));

        sf::RectangleShape sigil({10.f, 10.f});
        sigil.setPosition({x + 16.f, y + 18.f});
        sigil.setFillColor(sf::Color(palette.accent.r, palette.accent.g, palette.accent.b, alpha));

        sf::Text title(*font_);
        title.setCharacterSize(16);
        title.setOutlineThickness(1.2f);
        title.setOutlineColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(150.f * visibility)));
        title.setFillColor(sf::Color(palette.title.r, palette.title.g, palette.title.b, alpha));
        Localization::setText(title, entry.title);
        title.setPosition({x + 34.f, y + 10.f});

        sf::Text body(*font_);
        body.setCharacterSize(14);
        body.setOutlineThickness(1.f);
        body.setOutlineColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(138.f * visibility)));
        body.setFillColor(sf::Color(palette.body.r, palette.body.g, palette.body.b, alpha));
        Localization::setText(body, entry.body);
        body.setPosition({x + 18.f, y + 36.f});

        window.draw(shadow);
        window.draw(panel);
        window.draw(accent);
        window.draw(sigil);
        window.draw(title);
        window.draw(body);

        y -= kNotificationHeight + kNotificationGap;
    }
}

void NotificationFeed::trimExpired()
{
    entries_.erase(
        std::remove_if(entries_.begin(), entries_.end(),
            [](const NotificationEntry& entry) {
                return entry.lifetime.getElapsedTime() > entry.duration;
            }),
        entries_.end()
    );
}

std::string NotificationFeed::clampText(const std::string& text, const std::size_t limit)
{
    const sf::String decoded = Localization::toSfString(text);
    const std::u32string utf32 = decoded.toUtf32();
    if (utf32.size() <= limit)
    {
        return text;
    }

    const std::size_t keepCount = limit > 3 ? limit - 3 : 0;
    const sf::String clamped = sf::String::fromUtf32(utf32.begin(), utf32.begin() + static_cast<std::ptrdiff_t>(keepCount));
    const std::string suffix = "...";
    const auto bytes = clamped.toUtf8();
    return std::string(bytes.begin(), bytes.end()) + suffix;
}
