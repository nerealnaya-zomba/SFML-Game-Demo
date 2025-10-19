#include<SFML/Graphics.hpp>
#include<iostream>

class LoadScreen
{
public:
    LoadScreen();
    ~LoadScreen();


private:
    sf::RectangleShape* progressBar_back_m;
    sf::RectangleShape* progressBar_front_m;


};