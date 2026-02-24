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
const sf::Color BASE_YES_IDLE_COLOR(60, 180, 140);      // Мятно-зеленый
// HOVER
const sf::Color BASE_YES_HOVER_COLOR(80, 200, 160);      // Светло-мятный
// CLICK
const sf::Color BASE_YES_CLICK_COLOR(40, 140, 110);      // Темно-мятный
// IDLE
const sf::Color BASE_YES_TEXT_IDLE_COLOR(0, 0, 0);      // Мятно-зеленый
// HOVER
const sf::Color BASE_YES_TEXT_HOVER_COLOR(255, 255, 255);      // Светло-мятный
// CLICK
const sf::Color BASE_YES_TEXT_CLICK_COLOR(127, 127, 127);      // Темно-мятный

//No Button COlor
// IDLE
const sf::Color BASE_NO_IDLE_COLOR(230, 150, 70);       // Янтарный
// HOVER
const sf::Color BASE_NO_HOVER_COLOR(250, 180, 100);     // Светлый янтарь
// CLICK
const sf::Color BASE_NO_CLICK_COLOR(200, 120, 40);      // Темный янтарь
// IDLE
const sf::Color BASE_NO_TEXT_IDLE_COLOR(0, 0, 0);      // Мятно-зеленый
// HOVER
const sf::Color BASE_NO_TEXT_HOVER_COLOR(255, 255, 255);      // Светло-мятный
// CLICK
const sf::Color BASE_NO_TEXT_CLICK_COLOR(127, 127, 127);      // Темно-мятный

//Label color
const sf::Color BASE_LABEL_TEXT_COLOR(255,255,255);         // Белый

//Background color
const sf::Color BASE_ASKDIALOGUE_BACKGROUND_COLOR(70, 130, 180);  // Стальной синий

class AskDialogue{
public:
    AskDialogue(sf::Vector2f pos,sf::Vector2f size, std::string text, sf::RenderWindow& window);
    virtual ~AskDialogue();

    //Main methods
        //Drawing
        void draw(sf::RenderWindow& window);
        //Handling event
        void handleEvent(const sf::Event &ev);
        //Open/close
        void open();
        void close();

    //Settings
    void setOnYesClick(std::function<void()> fnc);
    void setOnNoClick(std::function<void()> fnc);
    void connectTGUIFont(tgui::Font &font);

private:
    //TGUI
        //Group
        tgui::Gui gui;
        //Buttons
        tgui::Button::Ptr yesButton;
        tgui::Button::Ptr noButton;
        //Text
        tgui::Label::Ptr  label;
    ///////////////////////////////////////
    //SFML
        //Background
        sf::RectangleShape* main_rect_m;
        //Window
        sf::RenderWindow* window_m;
    ///////////////////////////////////////
    //System
        //Answer
        enum Answer
        {
            Yes,
            No,
            NoAnswer
        };
        Answer answer_m = NoAnswer;
        //Bools
        bool isCalled = false;
        //Settings
        int characterSize = 30;
        //Actions
        std::function<void()> onYesClick;
        std::function<void()> onNoClick;
};
