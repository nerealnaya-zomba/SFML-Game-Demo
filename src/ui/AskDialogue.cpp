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

    // First answer rectangle
    answer1_rect_m = new sf::RectangleShape;
    answer1_rect_m->setSize({size.x/5, size.y/3});
    setRectangleOriginToMiddle(*answer1_rect_m);

    answer1_rect_m->setPosition({answer1_position_x, answer1_position_y-10});
    answer1_rect_m->setFillColor(sf::Color::Green);

    // Second answer rectangle
    answer2_rect_m = new sf::RectangleShape;  
    answer2_rect_m->setSize({size.x/5, size.y/3});
    setRectangleOriginToMiddle(*answer2_rect_m);

    answer2_rect_m->setPosition({answer2_position_x, answer2_position_y-10});
    answer2_rect_m->setFillColor(sf::Color::Red);

    // Top message
    main_text_m = new sf::Text(font,text); 
    main_text_m->setCharacterSize(characterSize);
    main_text_m->setFillColor(sf::Color::White);
    setTextOriginToMiddle(*main_text_m);
    float main_text_m_x = main_rect_m->getPosition().x;
    float main_text_m_y = main_rect_m->getPosition().y - main_rect_m->getSize().y/2 + characterSize + 5;
    main_text_m->setPosition({main_text_m_x, main_text_m_y});

    // First answer message
    answer1_text_m = new sf::Text(font,"Yes");  // Добавлен текст
    answer1_text_m->setCharacterSize(characterSize);
    answer1_text_m->setFillColor(sf::Color::White);
    setTextOriginToMiddle(*answer1_text_m);
    answer1_text_m->setPosition({answer1_rect_m->getPosition().x,answer1_rect_m->getPosition().y-10});

    // Second answer message
    answer2_text_m = new sf::Text(font,"No");  // Добавлен текст
    answer2_text_m->setCharacterSize(characterSize);
    answer2_text_m->setStyle(sf::Text::Italic);
    answer2_text_m->setFillColor(sf::Color::White);
    setTextOriginToMiddle(*answer2_text_m);
    answer2_text_m->setPosition({answer2_rect_m->getPosition().x,answer2_rect_m->getPosition().y-10});
}
AskDialogue::~AskDialogue()
{
    delete main_rect_m, answer1_rect_m, answer2_rect_m, main_text_m;
}

void AskDialogue::draw(sf::RenderWindow& window)
{
    window.draw(*main_rect_m);
    window.draw(*answer1_rect_m);
    window.draw(*answer2_rect_m);
    window.draw(*main_text_m);
    window.draw(*answer1_text_m);
    window.draw(*answer2_text_m);
}

void AskDialogue::setOnYesClick(std::function<void()> &fnc)
{
    this->onYesClick = fnc;
}

void AskDialogue::setOnNoClick(std::function<void()> &fnc)
{
    this->onNoClick = fnc;
}
