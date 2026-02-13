#pragma once
#include "GameCamera.h"
#include "GameData.h"
#include "GameLevel.h"
#include<SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

const sf::Vector2i BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN = {10,10};
const sf::Vector2f BASE_DESTINATION_ICON_SIZE                = {30,30};

////////////////////////
/// Представляет собой меню для выбора места назначения телепорта
////////////////////////

class ChooseDestination
{
private:
	////////////////////////////
	// Внешние ссылки на объекты
	////////////////////////////
	GameData*   data;
	GameCamera* camera;
	GameLevelManager* manager;
	////////////////////////////
	
	//////////////////////////////////
	// Menu box general representation
	//////////////////////////////////
		struct LevelDestination{
			bool isOpened = true;
			bool isVisible = true;
			bool isSelected = false;
			bool isPlayerThere = false;	

			GameLevel* level;
		};

		//////////////////////////////////////////////////////////////////////////////////
		// Представляет собой отдельный элемент уровня в контейнере, содержащий: параметры отображения, иконку, квадрат выделения
		//////////////////////////////////////////////////////////////////////////////////
		struct LevelDestinationRect{

			LevelDestination leveldestination;

			sf::Sprite icon;

			sf::RectangleShape selectionRect;
			
			void draw(sf::RenderWindow& w);
		};
		///////////
		// Elements
		///////////
		sf::Sprite background;
		std::vector<LevelDestinationRect> levels;

		// Points to element
		std::vector<LevelDestinationRect>::iterator levelIt;
		// Utils to manipulate it
		void moveLevelItLeft();
		void moveLevelItRight();

		///////////
		//////////////////////////////
		// Element positioning methods
		//////////////////////////////
		void positioningLevelDestinations();
			void positioningLevelDestinationsBackground();
			void positioningLevelDestinationsLevels();
		//////////////////////////////

		//////////////////////////
		/// Draw methods
		//////////////////////////
		void drawLevelDestinations(sf::RenderWindow& window);
			void drawLevelDestinationsBackground(sf::RenderWindow& window);
			void drawLevelDestinationsLevels(sf::RenderWindow& window);
		//////////////////////////

		//////////////////
		/// Action methods
		//////////////////
		void addLevelInMap(GameLevel& level, LevelDestinationRect l);
		//////////////////

	//////////////////////////////////

	// Events handling
		// bools
		bool isKeyPressed = false;

		void handleEvents(sf::Event& ev);

public:
	ChooseDestination(const ChooseDestination &) = default;
	ChooseDestination(ChooseDestination &&) = delete;
	ChooseDestination &operator=(const ChooseDestination &) = default;
	ChooseDestination &operator=(ChooseDestination &&) = delete;
	ChooseDestination(GameData &d, GameCamera &c, GameLevelManager &lm);
	~ChooseDestination() = default;



	////////////////////////
	// Основые методы в mainLoop
	////////////////////////
	void updateControls();
	void update();
	void draw(sf::RenderWindow& w);
	////////////////////////

};
