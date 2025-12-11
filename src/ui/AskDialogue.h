#pragma once
#include<SFML/Graphics.hpp>
#include<Mounting.h>
#include<MainMenu.h>
#include<vector>
class AskDialogue{
public:
    AskDialogue(sf::Vector2f pos,sf::Vector2f size, std::string text,sf::Font& font, sf::Color color,sf::RectangleShape& mouseRect, sf::RenderWindow& window);
    virtual ~AskDialogue();

    //Bools
    bool isCalled = false;

    //Settings
    int characterSize = 30;

    //Rects
    sf::RectangleShape* main_rect_m;
    sf::RectangleShape* answer1_rect_m;
    sf::RectangleShape* answer2_rect_m;
        //external mouseRect(that defined in main.cpp)
    sf::RectangleShape* mouseRect_m;
    //Window
    sf::RenderWindow* window_m;

    //Answer
    enum Answer
    {
        Yes,
        No,
        NoAnswer
    };
    Answer answer_m = NoAnswer;

    //Message
        //Top
    sf::Text* main_text_m;
        //First answer
    sf::Text* answer1_text_m;
        //Second answer
    sf::Text* answer2_text_m;

    //Logical
    void checkAnswer();

    //Drawing
    void draw(sf::RenderWindow& window);




};
