#pragma once
#include <SFML/Graphics.hpp>
#include<deque>
#include<iostream>
#include<algorithm>

class Trail
{
    public:
    Trail(sf::Sprite& target);
    ~Trail();

    sf::Color trailColor = sf::Color::Black;
    int speedOfTrailDisappearing = 30;

    void generateTrail(sf::RenderWindow& window);
    void drawTrail(sf::RenderWindow& window);

    void makeTrailDisappear();

    void stopTrail();
    void startTrail();
    void clearTrailArray();

    private:
    
    bool isGenerating = true;
    
    sf::Sprite* source;

    std::deque<std::unique_ptr<sf::Sprite>> shapes;

};