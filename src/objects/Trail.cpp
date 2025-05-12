#include <Trail.h>

Trail::Trail(sf::Sprite& target)
{
    this->source = &target;
}

Trail::~Trail()
{
}

void Trail::generateTrail(sf::RenderWindow& window)
{
    if(isGenerating)
    {
        //Instead of a regular pointer, we use a unique one for automatic memory clearing
        auto shape = std::make_unique<sf::Sprite>(source->getTexture());
        shape->setOrigin(source->getOrigin());
        shape->setScale(source->getScale());
        shape->setColor(this->trailColor);
        shape->setPosition(source->getPosition());
        
        shapes.push_front(std::move(shape));
    }
    
    
}

void Trail::drawTrail(sf::RenderWindow& window)
{
    for(std::unique_ptr<sf::Sprite>& i : shapes)
    {
        window.draw(*i);
    }
}

void Trail::makeTrailDisappear()
{
    //Decrease alfa by 1 on each element
    for (std::unique_ptr<sf::Sprite>& i : shapes)
    {
        sf::Color shapeColor = i->getColor();
        for (size_t i = 0; i < speedOfTrailDisappearing; i++)
        {
            shapeColor.a--;
        }
        
        i->setColor(shapeColor);
    }
    //If last element's alfa less or equeal 2, then remove it from array
    if(!shapes.empty() && shapes.back()->getColor().a <=speedOfTrailDisappearing)
    {
        shapes.pop_back();
    }
}

void Trail::stopTrail()
{
    isGenerating = false;
}

void Trail::startTrail()
{
    isGenerating = true;
}

void Trail::clearTrailArray()
{
    shapes.clear();
}
