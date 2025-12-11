#include<GameLoadingScreen.h>

LoadingScreen::LoadingScreen(sf::RenderWindow* window,sf::Font* font, int operations_count)
{
    //Window reference initialization
    window_m = window;

    //Step size calculation
    loadingBar_step_m = loadingBar_width_m/operations_count;

    //Loading bar initialization
    loadingBar_back_m.setPosition({WINDOW_WIDTH/4,WINDOW_HEIGHT/2+100.f});
    loadingBar_back_m.setFillColor(sf::Color::White);
    loadingBar_back_m.setOutlineThickness(3);
    loadingBar_back_m.setOutlineColor(sf::Color::Black);
    loadingBar_back_m.setSize({loadingBar_width_m,50.f});
    
    loadingBar_front_m.setPosition({WINDOW_WIDTH/4,WINDOW_HEIGHT/2+100.f});
    loadingBar_front_m.setFillColor({210,210,210,255});
    loadingBar_front_m.setSize({10.f,50.f});

    //Text initialization
    loading_text_m = new sf::Text(*font);
    loading_text_m->setString("Loading...");
    loading_text_m->setCharacterSize(30);
    loading_text_m->setFillColor(sf::Color::White);
    setTextOriginToMiddle(*loading_text_m);
    loading_text_m->setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT/2});

    
}

LoadingScreen::~LoadingScreen()
{

}

void LoadingScreen::update(int succesedOperationsCount)
{
    loadingBar_front_m.setSize({loadingBar_step_m*succesedOperationsCount,50.f});
}

void LoadingScreen::draw()
{
    window_m->clear(sf::Color::Black);
    window_m->draw(loadingBar_back_m);
    window_m->draw(loadingBar_front_m);
    window_m->draw(*loading_text_m);
    window_m->display();
}
