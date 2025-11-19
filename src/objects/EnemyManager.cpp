#include "EnemyManager.h"

template <typename T>
void EnemyManager<T>::removeIfNotAlive()
{
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(), 
            [](auto& enemy) {
                return !enemy->isAlive;
            }),
        enemies.end()
    );
}

template <typename T>
void EnemyManager<T>::add(T *enemy)
{
    enemies.push_back(enemy);
}

template<typename T>
void EnemyManager<T>::updateAI_all()
{
    for (auto enemy : enemies) {
        enemy->updateAI();
    }
}

template<typename T>
void EnemyManager<T>::updatePhysics_all()
{
    for (auto enemy : enemies) {
        enemy->updatePhysics();
    }
}

template<typename T>
void EnemyManager<T>::updateTextures_all()
{
    for (auto enemy : enemies) {
        enemy->updateTextures();
    }
    this->removeIfNotAlive();
}

template<typename T>
void EnemyManager<T>::draw_all()
{
    for (auto enemy : enemies) {
        enemy->draw();
    }
}

template<typename T>
EnemyManager<T>::EnemyManager()
{
}

template<typename T>
EnemyManager<T>::~EnemyManager()
{
}

template class EnemyManager<Skeleton>;