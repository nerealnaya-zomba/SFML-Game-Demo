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
    // Удаляем ВСЕ спрайты с достаточно низкой прозрачностью
    while(!shapes.empty() && shapes.back()->getColor().a <= speedOfTrailDisappearing)
    {
        shapes.pop_back();
    }
    
    // Или альтернативный вариант - удалять по прозрачности, а не по позиции:
    shapes.erase(
        std::remove_if(shapes.begin(), shapes.end(),
            [this](const std::unique_ptr<sf::Sprite>& sprite) {
                return sprite->getColor().a <= this->speedOfTrailDisappearing;
            }),
        shapes.end()
    );
    
    // Уменьшаем alpha для оставшихся спрайтов
    for(std::unique_ptr<sf::Sprite>& i : shapes)
    {
        sf::Color shapeColor = i->getColor();
        shapeColor.a = std::max(0, shapeColor.a - speedOfTrailDisappearing);
        i->setColor(shapeColor);
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
