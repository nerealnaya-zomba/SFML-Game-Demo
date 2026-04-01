#pragma once
#include <LoadingProgress.h>
#include <MenuBackground.h>
#include <Mounting.h>
#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

class LoadingScreen
{
public:
    LoadingScreen(sf::RenderWindow& window, sf::Font& font);
    ~LoadingScreen();

    void update(float deltaTime, const LoadingProgressSnapshot& snapshot);
    void draw();
    bool isReadyToClose() const;

private:
    struct Ember
    {
        sf::Vector2f position;
        float speed = 0.f;
        float drift = 0.f;
        float radius = 0.f;
        float alpha = 0.f;
        float phase = 0.f;
    };

    sf::RenderWindow* window_m = nullptr;
    sf::Font* font_m = nullptr;
    MenuBackground background_m;

    std::vector<Ember> embers_{};
    std::vector<std::string> loreLines_{};

    sf::Text titleText_;
    sf::Text chapterText_;
    sf::Text statusText_;
    sf::Text loreText_;
    sf::Text percentText_;
    sf::Text counterText_;

    float time_m = 0.f;
    float displayedProgress_m = 0.f;
    float targetProgress_m = 0.f;
    float loreTimer_m = 0.f;
    int completedOperations_m = 0;
    int totalOperations_m = 0;
    bool failed_m = false;
    std::size_t loreIndex_m = 0;
    std::string stageLabel_m{"Listening to the ruins"};
    std::string errorMessage_m{};

    void configureText(sf::Text& text, unsigned int size, sf::Color fillColor, float letterSpacing = 1.f) const;
    void updateTexts();
    void updateEmbers(float deltaTime);
    void drawBackdrop();
    void drawArchitecture();
    void drawSigils();
    void drawProgressPanel();
    void drawEmbers();
};
