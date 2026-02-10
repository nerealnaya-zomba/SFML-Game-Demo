#pragma once
#include<SFML/Graphics.hpp>
#include <map>

////////////////////////
/// Представляет собой меню для выбора места назначения телепорта
////////////////////////

class ChooseDestination
{
private:
	sf::Sprite background;
	std::map<std::string, sf::Sprite> levels;
	std::map<std::string, sf::Sprite>::iterator levelIt;

	bool isCalled;

public: 
	////////////////////////
	// Основые методы
	////////////////////////
	void updateControls();
	void update();
	void draw();
	////////////////////////

};
