#include <sfml-headers.h>

int main()
{
    //Window preferences
    auto window = sf::RenderWindow(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), WINDOW_TITLE, sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(144);
    
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

        try
        {
            window.clear(sf::Color(10u,20u,30u,100u));
            window.draw(txt);
            window.display();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }



}
