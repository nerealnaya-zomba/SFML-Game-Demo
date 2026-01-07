#include "Ground.h"
#include<GameLevel.h>

Ground::Ground(GameData& gameTextures, GameLevelManager& levelManager, std::string groundFileName, unsigned int point_begin, unsigned int point_end, unsigned int yPos, float offSet) 
{
    // Копирование
    this->point_begin = point_begin;
    this->point_end   = point_end;
    this->yPos        = yPos;
    this->offSet      = offSet;

    // Привязывание текстуры
    attachTexture(gameTextures.TileSetGreenTextures.find(groundFileName)->second,this->ground1Texture_m);

    // Инициализация спрайта
    ground1Sprite_m = new sf::Sprite(*ground1Texture_m);

    // Инициализация хитбокса
    ground1Rect_m = new sf::RectangleShape();
    ground1Rect_m->setSize({WINDOW_WIDTH,static_cast<float>(ground1Sprite_m->getTexture().getSize().y)});
    ground1Rect_m->setPosition({0,0});

    

    // Получаем размер текстуры в пикселях
    this->tilesetsize = ground1Sprite_m->getTexture().getSize();

    // Выставляем стандартный отступ от нижней части экрана, если в конструкторе не указан yPos
    if(this->yPos==0U)
    {
        this->yPos = WINDOW_HEIGHT-tilesetsize.y;
    }

    // Выставляем позицию хитбокса пола по yPos, и offSet от спрайта
    ground1Rect_m->setPosition({0,this->yPos+this->offSet});    
}
Ground::~Ground()
{
    delete ground1Rect_m;
    delete ground1Sprite_m;
}
void Ground::draw(sf::RenderWindow& window)
{
    unsigned int nextPos = this->point_begin;
    // Заполнение пространства хитбокса спрайтом
    for (size_t i = 0; i < point_end/tilesetsize.x; i++)
    {
        ground1Sprite_m->setPosition({static_cast<float>(nextPos),static_cast<float>(yPos)});
    
        window.draw(*ground1Sprite_m);

        nextPos += tilesetsize.x;
    }
    // ground1Rect_m->setFillColor(sf::Color::Red);
    // window.draw(*ground1Rect_m);
}

sf::RectangleShape &Ground::getRect()
{
    if(this->ground1Rect_m==nullptr)
    {
        throw std::runtime_error("ground is nullptr");
    }
    return *this->ground1Rect_m;
}

void Ground::setOffset(float offset)
{
    this->offset = offset;
}
