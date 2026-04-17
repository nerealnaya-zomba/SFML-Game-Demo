#pragma once

#include <Mounting.h>
#include <SFML/Graphics.hpp>
#include <TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <functional>

const sf::Color BASE_YES_IDLE_COLOR(78, 49, 28);
const sf::Color BASE_YES_HOVER_COLOR(109, 66, 36);
const sf::Color BASE_YES_CLICK_COLOR(137, 83, 44);
const sf::Color BASE_YES_TEXT_IDLE_COLOR(244, 225, 196);
const sf::Color BASE_YES_TEXT_HOVER_COLOR(255, 246, 230);
const sf::Color BASE_YES_TEXT_CLICK_COLOR(255, 236, 211);

const sf::Color BASE_NO_IDLE_COLOR(59, 17, 19);
const sf::Color BASE_NO_HOVER_COLOR(88, 25, 28);
const sf::Color BASE_NO_CLICK_COLOR(119, 31, 34);
const sf::Color BASE_NO_TEXT_IDLE_COLOR(239, 222, 208);
const sf::Color BASE_NO_TEXT_HOVER_COLOR(255, 245, 234);
const sf::Color BASE_NO_TEXT_CLICK_COLOR(255, 234, 214);

const sf::Color BASE_LABEL_TEXT_COLOR(235, 223, 209);
const sf::Color BASE_ASKDIALOGUE_BACKGROUND_COLOR(14, 10, 12);

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
    void attachWindow(sf::RenderWindow& window);

    void setOnYesClick(std::function<void()> fnc);
    void setOnNoClick(std::function<void()> fnc);
    void connectTGUIFont(tgui::Font& font);

private:
    void refreshLayout();

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
    sf::Vector2f desiredSize_{0.f, 0.f};
    std::function<void()> onYesClick;
    std::function<void()> onNoClick;
};
