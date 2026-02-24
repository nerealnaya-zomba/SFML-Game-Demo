#pragma once
#include<SFML/Graphics.hpp>
#include<Mounting.h>
#include<TGUI/TGUI.hpp>
#include<TGUI/Backend/SFML-Graphics.hpp>
#include<TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>
#include<MainMenu.h>
#include<vector>

//Yes Button Color
// IDLE
sf::Color BASE_YES_IDLE_COLOR(60, 180, 140);      // Мятно-зеленый
// HOVER
sf::Color BASE_YES_HOVER_COLOR(80, 200, 160);      // Светло-мятный
// CLICK
sf::Color BASE_YES_CLICK_COLOR(40, 140, 110);      // Темно-мятный
// IDLE
sf::Color BASE_YES_TEXT_IDLE_COLOR(0, 0, 0);      // Мятно-зеленый
// HOVER
sf::Color BASE_YES_TEXT_HOVER_COLOR(255, 255, 255);      // Светло-мятный
// CLICK
sf::Color BASE_YES_TEXT_CLICK_COLOR(127, 127, 127);      // Темно-мятный

//No Button COlor
// IDLE
sf::Color BASE_NO_IDLE_COLOR(230, 150, 70);       // Янтарный
// HOVER
sf::Color BASE_NO_HOVER_COLOR(250, 180, 100);     // Светлый янтарь
// CLICK
sf::Color BASE_NO_CLICK_COLOR(200, 120, 40);      // Темный янтарь
// IDLE
sf::Color BASE_NO_TEXT_IDLE_COLOR(0, 0, 0);      // Мятно-зеленый
// HOVER
sf::Color BASE_NO_TEXT_HOVER_COLOR(255, 255, 255);      // Светло-мятный
// CLICK
sf::Color BASE_NO_TEXT_CLICK_COLOR(127, 127, 127);      // Темно-мятный

class AskDialogue{
public:
    AskDialogue(sf::Vector2f pos,sf::Vector2f size, std::string text,tgui::Font& font, sf::Color color,sf::RectangleShape& mouseRect, sf::RenderWindow& window);
    virtual ~AskDialogue();

    //Bools
    bool isCalled = false;

    //Settings
    int characterSize = 30;

    //Buttons
    tgui::Button::Ptr yesButton;
    tgui::Button::Ptr noButton;

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

    //Drawing
    void draw(sf::RenderWindow& window);

    //Settings
    void setOnYesClick(std::function<void()>& fnc);
    void setOnNoClick(std::function<void()>& fnc);

private:
std::function<void()> onYesClick;
std::function<void()> onNoClick;
};
