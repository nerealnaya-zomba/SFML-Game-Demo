#pragma once
#include "GameCamera.h"
#include "GameData.h"
#include "GameLevel.h"
#include<SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <vector>

////////////////////////
/// Представляет собой меню для выбора места назначения телепорта
////////////////////////

class ChooseDestination
{
private:
	// Внешние ссылки на объекты
	GameData*   data;
	GameCamera* camera;
	GameLevelManager* manager;
	////////////////////////////
	
	struct LevelDestination{
		bool isOpened = true;
		bool isVisible = true;
		bool isSelected = false;
		bool isPlayerThere = false;	

		GameLevel* level;
	};

	// Представляет собой отдельный элемент уровня в контейнере, содержащий: , иконку,
	//
	/////////////////////////////////////// 
	struct LevelDestinationRect{

		LevelDestination leveldestination;

		sf::Sprite icon;

		sf::RectangleShape selectionRect;
		
		void draw(sf::RenderWindow& w);
	};

	sf::Sprite background;
	std::vector<LevelDestinationRect> levels;
	std::vector<LevelDestinationRect>::iterator levelIt;

	void addLevelInMap(GameLevel& level, LevelDestinationRect l);

	//////////////////////////
	/// Draw methods
	void drawLevelDestinations(sf::RenderWindow& window);

public:
  ChooseDestination(const ChooseDestination &) = default;
  ChooseDestination(ChooseDestination &&) = delete;
  ChooseDestination &operator=(const ChooseDestination &) = default;
  ChooseDestination &operator=(ChooseDestination &&) = delete;
  ChooseDestination(GameData &d, GameCamera &c, GameLevelManager &lm);
  ~ChooseDestination() = default;

  ////////////////////////
  // Основые методы
  ////////////////////////
  void updateControls();
  void update();
  void draw(sf::RenderWindow& w);
  ////////////////////////

};
