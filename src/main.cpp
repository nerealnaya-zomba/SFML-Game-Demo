#include <sfml-headers.h>

int main()
{
    //Window preferences
    auto window = sf::RenderWindow(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), WINDOW_TITLE, sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(144);

    //Bools
    bool isMainMenuCalled = true;

    //Mouse
    sf::RectangleShape mouseRect({1.f,1.f});

    //Background
    sf::Color backGroundColor({0,0,0});
    
    //Loading font
    sf::Font font;
    if(font.openFromFile("fonts/Roboto_Condensed-Black.ttf"))
        std::cout << "Font opened" << std::endl;
    else
        std::cout << "Font error" << std::endl;

    //Main menu
    sf::RectangleShape playButton(sf::Vector2f{200.f,75.f});
    playButton.setFillColor(sf::Color::Black);
    playButton.setOrigin({playButton.getSize().x/2,playButton.getSize().y/2});
    playButton.setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT-325+50});
    sf::Text playButtonText(font,"Play");
    playButtonText.setFillColor(sf::Color::White);
    playButtonText.setOrigin({playButtonText.getGlobalBounds().size.x/2,playButtonText.getGlobalBounds().size.y/2});
    playButtonText.setPosition({playButton.getPosition().x, playButton.getPosition().y-2});
    sf::RectangleShape exitButton(sf::Vector2f{200.f,75.f});
    exitButton.setFillColor(sf::Color::Black);
    exitButton.setOrigin({exitButton.getSize().x/2,exitButton.getSize().y/2});
    exitButton.setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT-200+50});
    sf::Text exitButtonText(font,"Exit");
    exitButtonText.setFillColor(sf::Color::White);
    exitButtonText.setOrigin({exitButtonText.getGlobalBounds().size.x/2,exitButtonText.getGlobalBounds().size.y/2});
    exitButtonText.setPosition({exitButton.getPosition().x, exitButton.getPosition().y-6});

    



    //Main loop
    while (window.isOpen())
    {
        


        while (const std::optional event = window.pollEvent())
        {
            if(isMainMenuCalled)
            {
                
            }


            //Close window on "Close" button
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if(event->is<sf::Event::MouseWheelScrolled>())
            {
                std::cout << "MouseWheelScrolled" << std::endl;
            }
            
        }
        
        //Main menu
        if(isMainMenuCalled)
        {





            sizeUpRectangleOnHover(playButton,mouseRect,window, 0.03f , 0.02f);
            sizeUpRectangleOnHover(exitButton,mouseRect,window, 0.03f , 0.02f);

            rainbowWindowClear(window, backGroundColor);

            window.draw(playButton);
            window.draw(exitButton);
            window.draw(playButtonText);
            window.draw(exitButtonText);
            window.display();
            continue;
        }

        try
        {
            window.clear(sf::Color(10u,20u,30u,100u));
            
            window.display();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }



}
