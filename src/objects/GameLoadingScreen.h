#pragma once
#include<SFML/Graphics.hpp>
#include<Defines.h>
#include<Mounting.h>

class LoadingScreen{
public:
    LoadingScreen(sf::RenderWindow* window,sf::Font* font, int operations_count);
    ~LoadingScreen();


    //Drawing
    void update(int succesedOperationsCount);
    void draw();
private:
    //Loading bar
        //Rectangles
    sf::RectangleShape loadingBar_back_m;
    sf::RectangleShape loadingBar_front_m;
    float loadingBar_width_m = 960.f;
        //Bar_front step
    float loadingBar_step_m{};

    //Text
    sf::Text* loading_text_m;

    
    //Window to draw on
    sf::RenderWindow* window_m;

};