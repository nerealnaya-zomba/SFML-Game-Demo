#include<Enemy.h>

void Enemy::switchTexture(std::vector<std::unique_ptr<sf::Sprite>> &spritesArray, std::vector<sf::Texture> &texturesArray, texturesIterHelper &iterHelper)
{
    if(iterHelper.iterationCounter<iterHelper.iterationsTillSwitch)
    {
        iterHelper.iterationCounter++;
    }
    else
    {
        //Forward-backward logic
        if(iterHelper.ptrToTexture == iterHelper.countOfTextures)
        {
            iterHelper.goForward = false;
        }
        else if(iterHelper.ptrToTexture == 0)
        {
            iterHelper.goForward = true;
        }

        //Switch sprites
        for (auto &i : spritesArray)
        {
            i->setTexture(texturesArray.at(iterHelper.ptrToTexture));
        }

        //Forward-backward logic
        if(iterHelper.goForward)
        {
            iterHelper.ptrToTexture++;
        }
        else
        {
            iterHelper.ptrToTexture--;
        }

        //reset iteration counter after all switches
        iterHelper.iterationCounter = 0;
    }
}

Enemy::Enemy(GameData &gameData)
{

}

Enemy::~Enemy()
{

}
