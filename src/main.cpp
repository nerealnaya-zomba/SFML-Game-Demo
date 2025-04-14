#include <sfml-headers.h>

int main()
{
    //Loading font
    sf::Font font;
    if(font.openFromFile("fonts/Roboto_Condensed-Black.ttf"))
        std::cout << "Font opened" << std::endl;
    else
        std::cout << "Font error" << std::endl;

    //Loading text
    sf::Text txt(font, "uwu zombe (o'w'o)");
    txt.setPosition(sf::Vector2f(320.0f,350.0f));
    txt.setFillColor(sf::Color::White);
    


    //Window preferences
    auto window = sf::RenderWindow(sf::VideoMode({800u, 800u}), WINDOW_TITLE, sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(144);


    //Main loop
    while (window.isOpen())
    {

        while (const std::optional event = window.pollEvent())
        {
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

        window.clear(sf::Color(10u,20u,30u,100u));
        window.draw(txt);
        window.display();
    }



}
