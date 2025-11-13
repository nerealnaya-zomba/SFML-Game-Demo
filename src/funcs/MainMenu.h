#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <Defines.h>
#include <ctime>
#include <iostream>

static void moveRectToMouse(sf::RectangleShape& rect,sf::RenderWindow& window)
{
    int mouseX = sf::Mouse::getPosition(window).x;
    int mouseY = sf::Mouse::getPosition(window).y;
    rect.setPosition({static_cast<float>(mouseX),static_cast<float>(mouseY)});
}



//Size up rectangle when mouseRect based on mousePos intersects rect
static void sizeUpRectangleOnHover(sf::RectangleShape& rect, sf::RectangleShape& mouseRect, sf::RenderWindow& window, float sizingUpSpeed=0.05f, float sizingDownSpeed=0.05f)
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
//Size up rectangle and text when mouseRect based on mousePos intersects rect
static void sizeUpRectangleOnHover(sf::RectangleShape& rect, sf::Text& text, sf::RectangleShape& mouseRect, sf::RenderWindow& window, float sizingUpSpeed=0.05f, float sizingDownSpeed=0.05f)
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
        text.setScale({text.getScale().x+sizingUpSpeed, text.getScale().y+sizingUpSpeed});
    }
    else
    {
        isSizeUp = false;
    }

    if(isSizeDown && (rect.getScale().x >=1.05f && rect.getScale().y >= 1.05f))
    {
        rect.setScale({rect.getScale().x-sizingDownSpeed, rect.getScale().y-sizingDownSpeed});
        text.setScale({text.getScale().x-sizingDownSpeed, text.getScale().y-sizingDownSpeed});
    }
    else
    {
        isSizeDown = false;
    }
    
}
