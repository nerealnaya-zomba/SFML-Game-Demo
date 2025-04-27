#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <ctime>
#include <iostream>

void moveRectToMouse(sf::RectangleShape& rect,sf::RenderWindow& window)
{
    int mouseX = sf::Mouse::getPosition(window).x;
    int mouseY = sf::Mouse::getPosition(window).y;
    rect.setPosition({static_cast<float>(mouseX),static_cast<float>(mouseY)});
}

void rainbowWindowClear(sf::RenderWindow& window, sf::Color& backgroundColor)
{
    srand(time(0));

    static bool rGoUp = true;
    static bool rGoDown = false;
    if(rGoUp)
    {
        backgroundColor.r+=rand()%2+1;

        if(backgroundColor.r>249)
        {
            rGoUp = false;
            rGoDown = true;
        }
    }
    else if(rGoDown)
    {
        backgroundColor.r-=rand()%2+1;

        if(backgroundColor.r<6)
        {
            rGoUp = true;
            rGoDown = false;
        }
    }
    /////////////////////////////////////////////////
    static bool gGoUp = true;
    static bool gGoDown = false;
    if(gGoUp)
    {
        backgroundColor.g+=rand()%2+1;

        if(backgroundColor.g>249)
        {
            gGoUp = false;
            gGoDown = true;
        }
    }
    else if(gGoDown)
    {
        backgroundColor.g-=rand()%2+1;

        if(backgroundColor.g<6)
        {
            gGoUp = true;
            gGoDown = false;
        }
    }

    static bool bGoUp = true;
    static bool bGoDown = false;
    if(bGoUp)
    {
        backgroundColor.b+=rand()%2+1;

        if(backgroundColor.b>249)
        {
            bGoUp = false;
            bGoDown = true;
        }
    }
    else if(bGoDown)
    {
        backgroundColor.b-=rand()%2+1;

        if(backgroundColor.b<6)
        {
            bGoUp = true;
            bGoDown = false;
        }
    }
    //std::cout << (int)backgroundColor.r << " " << (int)backgroundColor.g << " " << (int)backgroundColor.b << " " << std::endl;
    window.clear(backgroundColor);
}

//Size up rectangle when mouseRect based on mousePos intersects rect
void sizeUpRectangleOnHover(sf::RectangleShape& rect, sf::RectangleShape& mouseRect, sf::RenderWindow& window, float sizingUpSpeed=0.05f, float sizingDownSpeed=0.05f)
{
    bool isSizeUp = false;
    bool isSizeDown = false;
    int mouseX = sf::Mouse::getPosition(window).x;
    int mouseY = sf::Mouse::getPosition(window).y;
    mouseRect.setPosition({static_cast<float>(mouseX),static_cast<float>(mouseY)});
    //std::cout << "mouseRect: " << mouseRect.getGlobalBounds().position.x << "" << mouseRect.getGlobalBounds().position.y << "     "<< "rect: " << rect.getGlobalBounds().position.x << "" << rect.getGlobalBounds().position.y << std::endl;

    if(mouseRect.getGlobalBounds().findIntersection(rect.getGlobalBounds()))
    {
        isSizeUp = true;
        
    }
    else
    {
        isSizeUp = false;
        isSizeDown = true;
    }

    if(isSizeUp && (rect.getScale().x <=1.3f && rect.getScale().y <= 1.3f))
    {
        rect.setScale({rect.getScale().x+sizingUpSpeed, rect.getScale().y+sizingUpSpeed});
    }
    else
    {
        isSizeUp = false;
    }

    if(isSizeDown && (rect.getScale().x >=1.05f && rect.getScale().y >= 1.05f))
    {
        rect.setScale({rect.getScale().x-sizingDownSpeed, rect.getScale().y-sizingDownSpeed});
    }
    else
    {
        isSizeDown = false;
    }
    
}