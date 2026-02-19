#pragma once
#include "GameCamera.h"
#include "GameData.h"
#include "GameLevel.h"
#include<SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <utility>
#include <vector>

class GameLevelManager;

//LevelRect
const sf::Vector2i BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN = {10,10};
const sf::Vector2f BASE_DESTINATION_ICON_SIZE                = {30,30};

//SelectionRect
const sf::Color BASE_SELECTION_COLOR = sf::Color::Blue;
const float     BASE_SELECTION_SIZE  = 2.f;

//CurrentLevelMarkRect
const sf::Color BASE_LEVELMARK_COLOR = sf::Color::Red;
const float     BASE_LEVELMARK_SIZE  = 2.f;

////////////////////////
/// Представляет собой меню для выбора места назначения телепорта
////////////////////////

class ChooseDestinationMenu
{
private:
	////////////////////////////
	// Внешние ссылки на объекты
	////////////////////////////
	GameData*   		data;
	GameCamera* 		camera;
	GameLevelManager* 	manager;
	////////////////////////////

	///////
	// Main 
	///////
		/// Bools ///
		bool isOpened = false;
		std::optional<std::string> desiredDestination;
		/// Keys  ///
		sf::Keyboard::Scan  moveLeftKey;
		sf::Keyboard::Scan moveRightKey;
		sf::Keyboard::Scan    selectKey;

	//////////////////////////////////
	// Menu box general representation
	//////////////////////////////////
public:
		struct LevelDestination{
			bool isOpened 		=  true;
			bool isVisible 		=  true;
			bool isSelected 	= false;
			bool isChoosed		= false;
			bool isPlayerThere 	= false;

			const GameLevel* level;
		};

		//////////////////////////////////////////////////////////////////////////////////
		// Представляет собой отдельный элемент уровня в контейнере, содержащий: параметры отображения, иконку, квадрат выделения
		//////////////////////////////////////////////////////////////////////////////////
		struct LevelDestinationRect
		{
			LevelDestinationRect(sf::Texture& iconTexture) : icon(iconTexture){};
			LevelDestinationRect(const sf::Texture& iconTexture) : icon(iconTexture){};

			LevelDestination 	leveldestination;

			sf::Sprite  		icon;

			sf::RectangleShape 	selectionRect;

			sf::RectangleShape 	currentLevelMarkRect;
			
			void draw(sf::RenderWindow& w);
		};

		struct LevelDestinationText
		{
			bool isVisible = false;
		};
private:
		///////////
		// Elements
		///////////
		sf::Sprite background;
		std::pair<LevelDestinationText, sf::Text> displayingLevelName;
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
			void positioningLevelDestinationsText();
		//////////////////////////////

		//////////////////////////////
		// Update methods
		//////////////////////////////
		void checkWherePlayer();
		//////////////////////////////

		//////////////////////////
		/// Draw methods
		//////////////////////////
		void drawLevelDestinations(sf::RenderWindow& window);
			void drawLevelDestinationsBackground(sf::RenderWindow& window);
			void drawLevelDestinationsLevels(sf::RenderWindow& window);
			void drawLevelDestinationsText(sf::RenderWindow& window);		// IMPLEMENTME
		//////////////////////////

		//////////////////
		/// Action methods
		//////////////////
		void currentSelectedElementToDesiredDestination();
			/// Returns level name
			std::string getCurrentLevelName();
			// Mount selection rect to its icon's parametrs
			void mountSelectionRect(sf::RectangleShape &sr, sf::Sprite& icon);	
			void mountCurrentLevelMarkRect(sf::RectangleShape &sr, sf::Sprite& icon);	
		//////////////////

	//////////////////////////////////

	// Events handling
		// bools
		bool isKeyPressed = false;
			void handleMoveEvents(const sf::Event& ev);
			void handleActivateEvent(const sf::Event& ev);

public:
	ChooseDestinationMenu(const ChooseDestinationMenu &) 			= default;
	ChooseDestinationMenu(ChooseDestinationMenu &&) 				= delete;
	ChooseDestinationMenu &operator=(const ChooseDestinationMenu &) = default;
	ChooseDestinationMenu &operator=(ChooseDestinationMenu &&) 		= delete;
	ChooseDestinationMenu(GameData &d, GameCamera &c, GameLevelManager &lm, sf::Keyboard::Scan moveLeftKey, sf::Keyboard::Scan moveRightKey, sf::Keyboard::Scan selectKey);
	~ChooseDestinationMenu() = default;

	/////////////////////////////////
	// Основные методы взаимодействия
	/////////////////////////////////
	void addLevelInVector(const GameLevel& level, sf::Texture& icon);
	void addLevelInVector(const GameLevel& level, const sf::Texture& icon);
	void open();
	void close();
	/////////////////////////////////

	////////////////////////////
	// Основые методы в mainLoop
	////////////////////////////
	void handleEvents(const sf::Event& ev);
	void update();
	void draw(sf::RenderWindow& w);
	////////////////////////

	//Getters
	bool getIsOpened();
	std::optional<std::string> getSelectedLevel();

};
