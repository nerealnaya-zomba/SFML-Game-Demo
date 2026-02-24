#include<Menu.h>


Menu::Menu(sf::Font& font, sf::RenderWindow& window, sf::RectangleShape& mouseRect)
    : background(WINDOW_WIDTH,WINDOW_WIDTH)
{
    //External references
    mouseRect_m = &mouseRect;
    window_m = &window;
    gui.setWindow(window);

    //Exit dialogue init
    exitDialogue = new AskDialogue(sf::Vector2f(WINDOW_WIDTH/2,WINDOW_HEIGHT/2),sf::Vector2f(350,150),"Quit the game?",font,sf::Color::Black,*mouseRect_m,*window_m);

    //Settings button init
    settingsButton = tgui::Button::create();
    settingsButton->setSize({200.f,75.f});
    settingsButton->setPosition({(WINDOW_WIDTH/2)-(settingsButton->getSize().x/2),WINDOW_HEIGHT*0.80});
    settingsButton->setText(BASE_SETTINGS_BUTTON_TEXT);
    settingsButton->setTextSize(BASE_MENU_BUTTONS_CHARACTER_SIZE);
    settingsButton->onClick([this](){this->settingsButtonOnClick();});
    //Renderer
        settingsButton->getRenderer()->setBackgroundColor(BASE_IDLE_COLOR);
        settingsButton->getRenderer()->setBackgroundColorHover(BASE_HOVER_COLOR);
        settingsButton->getRenderer()->setBackgroundColorDown(BASE_CLICK_COLOR);
        settingsButton->getRenderer()->setTextColor(BASE_TEXT_IDLE_COLOR);
        settingsButton->getRenderer()->setTextColorDown(BASE_TEXT_CLICK_COLOR);
        settingsButton->getRenderer()->setTextColorHover(BASE_TEXT_HOVER_COLOR);

    exitButton = tgui::Button::create();
    exitButton->setSize({200.f,75.f});
    exitButton->setPosition({(WINDOW_WIDTH/2)-(settingsButton->getSize().x/2),WINDOW_HEIGHT*0.70});
    exitButton->setText(BASE_EXIT_BUTTON_TEXT);
    exitButton->setTextSize(BASE_MENU_BUTTONS_CHARACTER_SIZE);
    exitButton->onClick([this](){this->exitButtonOnClick();});
    //Renderer
        exitButton->getRenderer()->setBackgroundColor(BASE_IDLE_COLOR);
        exitButton->getRenderer()->setBackgroundColorHover(BASE_HOVER_COLOR);
        exitButton->getRenderer()->setBackgroundColorDown(BASE_CLICK_COLOR);
        exitButton->getRenderer()->setTextColor(BASE_TEXT_IDLE_COLOR);
        exitButton->getRenderer()->setTextColorDown(BASE_TEXT_CLICK_COLOR);
        exitButton->getRenderer()->setTextColorHover(BASE_TEXT_HOVER_COLOR);

    playButton = tgui::Button::create();
    playButton->setSize({200.f,75.f});
    playButton->setPosition({(WINDOW_WIDTH/2)-(settingsButton->getSize().x/2),WINDOW_HEIGHT*0.60});
    playButton->setText(BASE_PLAY_BUTTON_TEXT);
    playButton->setTextSize(BASE_MENU_BUTTONS_CHARACTER_SIZE);
    playButton->onClick([this](){this->playButtonOnClick();});
        //Renderer
        playButton->getRenderer()->setBackgroundColor(BASE_IDLE_COLOR);
        playButton->getRenderer()->setBackgroundColorHover(BASE_HOVER_COLOR);
        playButton->getRenderer()->setBackgroundColorDown(BASE_CLICK_COLOR);
        playButton->getRenderer()->setTextColor(BASE_TEXT_IDLE_COLOR);
        playButton->getRenderer()->setTextColorDown(BASE_TEXT_CLICK_COLOR);
        playButton->getRenderer()->setTextColorHover(BASE_TEXT_HOVER_COLOR);

    gui.add(settingsButton);
    gui.add(exitButton);
    gui.add(playButton);
}

Menu::~Menu()
{

}

void Menu::connectTGUIFont(tgui::Font &font)
{
    this->gui.setFont(font);
}

void Menu::playButtonOnClick()
{
    this->isMainMenuCalled = false;
}

void Menu::settingsButtonOnClick()
{
}

void Menu::exitButtonOnClick()
{
    this->exitDialogue->isCalled = true;
}

void Menu::windowClear(sf::RenderWindow &window, sf::Color &backgroundColor)
{
    window.clear(backgroundColor);
}

void Menu::menuDraw(sf::RenderWindow& window)
{
    background.update(1.f/60.f);
    background.draw(window);
    if(exitDialogue->isCalled) exitDialogue->draw(window);
    gui.draw();
    window.display();
}

void Menu::menuHandleEvents(const sf::Event &ev)
{
    gui.handleEvent(ev);
}
