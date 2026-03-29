#pragma once

#include <Mounting.h>
#include <SFML/Graphics.hpp>
#include <TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <functional>

const sf::Color BASE_YES_IDLE_COLOR(60, 180, 140);
const sf::Color BASE_YES_HOVER_COLOR(80, 200, 160);
const sf::Color BASE_YES_CLICK_COLOR(40, 140, 110);
const sf::Color BASE_YES_TEXT_IDLE_COLOR(0, 0, 0);
const sf::Color BASE_YES_TEXT_HOVER_COLOR(255, 255, 255);
const sf::Color BASE_YES_TEXT_CLICK_COLOR(127, 127, 127);

const sf::Color BASE_NO_IDLE_COLOR(230, 150, 70);
const sf::Color BASE_NO_HOVER_COLOR(250, 180, 100);
const sf::Color BASE_NO_CLICK_COLOR(200, 120, 40);
const sf::Color BASE_NO_TEXT_IDLE_COLOR(0, 0, 0);
const sf::Color BASE_NO_TEXT_HOVER_COLOR(255, 255, 255);
const sf::Color BASE_NO_TEXT_CLICK_COLOR(127, 127, 127);

const sf::Color BASE_LABEL_TEXT_COLOR(255, 255, 255);
const sf::Color BASE_ASKDIALOGUE_BACKGROUND_COLOR(70, 130, 180);

class AskDialogue
{
public:
    AskDialogue(sf::Vector2f pos, sf::Vector2f size, std::string text, sf::RenderWindow& window);
    virtual ~AskDialogue();

    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& ev);
    void open();
    void close();
    bool isOpen() const;

    void setOnYesClick(std::function<void()> fnc);
    void setOnNoClick(std::function<void()> fnc);
    void connectTGUIFont(tgui::Font& font);

private:
    tgui::Gui gui;
    tgui::Button::Ptr yesButton;
    tgui::Button::Ptr noButton;
    tgui::Label::Ptr label;

    sf::RectangleShape mainRect_m;
    sf::RenderWindow* window_m = nullptr;

    enum Answer
    {
        Yes,
        No,
        NoAnswer
    };

    Answer answer_m = NoAnswer;
    bool isCalled = false;
    int characterSize = 30;
    std::function<void()> onYesClick;
    std::function<void()> onNoClick;
};
