#include<AskDialogue.h>

AskDialogue::AskDialogue(sf::Vector2f pos, sf::Vector2f size, std::string text, tgui::Font& font, 
                        sf::Color rectColor, sf::RectangleShape& mouseRect, sf::RenderWindow& window)
    : onYesClick([](){}), onNoClick([](){})
{
    window_m = &window;

    //TGUI Buttons
        //Yes
        this->yesButton = tgui::Button::create();
        yesButton->onClick(onYesClick);
        yesButton->setSize(size.x/5,size.y/3);
        float answer1_position_x = pos.x - (size.x/4);  
        float answer1_position_y = pos.y + (size.y/4);
        yesButton->setOrigin(0.5,0.5); //Middle 
        yesButton->setPosition(answer1_position_x,answer1_position_y);
            //Renderer
            yesButton->getRenderer()->setBackgroundColor(BASE_YES_IDLE_COLOR);
            yesButton->getRenderer()->setBackgroundColorHover(BASE_YES_HOVER_COLOR);
            yesButton->getRenderer()->setBackgroundColorDown(BASE_YES_CLICK_COLOR);
            yesButton->getRenderer()->setTextColor(BASE_YES_TEXT_IDLE_COLOR);
            yesButton->getRenderer()->setTextColorDown(BASE_YES_TEXT_CLICK_COLOR);
            yesButton->getRenderer()->setTextColorHover(BASE_YES_TEXT_HOVER_COLOR);

        //No
        this->noButton = tgui::Button::create();
        noButton->onClick(onNoClick);
        noButton->setSize(size.x/5,size.y/3);
        float answer2_position_x = pos.x + (size.x/4);
        float answer2_position_y = pos.y + (size.y/4);
        noButton->setOrigin(0.5,0.5); //Middle 
        noButton->setPosition(answer2_position_x,answer2_position_y);
            //Renderer
            noButton->getRenderer()->setBackgroundColor(BASE_NO_IDLE_COLOR);
            noButton->getRenderer()->setBackgroundColorHover(BASE_NO_HOVER_COLOR);
            noButton->getRenderer()->setBackgroundColorDown(BASE_NO_CLICK_COLOR);
            noButton->getRenderer()->setTextColor(BASE_NO_TEXT_IDLE_COLOR);
            noButton->getRenderer()->setTextColorDown(BASE_NO_TEXT_CLICK_COLOR);
            noButton->getRenderer()->setTextColorHover(BASE_NO_TEXT_HOVER_COLOR);

    // Main dialogue rectangle
    main_rect_m = new sf::RectangleShape;
    main_rect_m->setSize(size);
    setRectangleOriginToMiddle(*main_rect_m);
    main_rect_m->setPosition(pos);
    main_rect_m->setFillColor(rectColor);
}
AskDialogue::~AskDialogue()
{
    delete main_rect_m;
}

void AskDialogue::draw(sf::RenderWindow& window)
{
    window.draw(*main_rect_m);

}

void AskDialogue::setOnYesClick(std::function<void()> &fnc)
{
    this->onYesClick = fnc;
}

void AskDialogue::setOnNoClick(std::function<void()> &fnc)
{
    this->onNoClick = fnc;
}
