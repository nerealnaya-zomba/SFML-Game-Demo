#include "GameCamera.h"

GameCamera::GameCamera(sf::View &view, Player &player)
{
    // Привязываю ссылки к указателям
    this->player = &player;
    this->view = &view;


}

GameCamera::~GameCamera()
{
}

void GameCamera::update()
{
    if(this->isConditionSuccessed)
    {


        return;
    }
}