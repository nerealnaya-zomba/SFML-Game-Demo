#pragma once

#include <string>
#include <string_view>

#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>

enum class Language
{
    English,
    Russian
};

namespace Localization
{
    void setLanguage(Language language);
    Language getLanguage();
    Language languageFromString(std::string_view value);
    std::string languageToString(Language language);
    std::string languageDisplayName(Language language);
    Language nextLanguage(Language language);
    bool isRussian();
    std::string tr(std::string_view key);
    std::string weaponName(std::string_view value);
    sf::String toSfString(std::string_view utf8Text);
    void setText(sf::Text& text, std::string_view utf8Text);
}
