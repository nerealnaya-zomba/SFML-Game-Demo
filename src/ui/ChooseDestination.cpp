#include "GameCamera.h"
#include "GameData.h"
#include "GameLevel.h"
#include<ChooseDestination.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

void ChooseDestination::handleEvents(sf::Event &ev)
{
	if(const auto* keyPressed = ev.getIf<sf::Event::KeyPressed>())
	{
		if(keyPressed->scancode == sf::Keyboard::Scan::Left)
		{
			moveLevelItLeft();
		}
		else if(keyPressed->scancode == sf::Keyboard::Scan::Right)
		{
			moveLevelItRight();
		}
	}
}

void ChooseDestination::handleMoveEvents(sf::Event& ev)
{
	if(const auto* keyPressed = ev.getIf<sf::Event::KeyPressed>())
	{
		if(keyPressed->scancode == sf::Keyboard::Scan::Left)
		{
			moveLevelItLeft();
		}
		else if(keyPressed->scancode == sf::Keyboard::Scan::Right)
		{
			moveLevelItRight();
		}
	}
}

void ChooseDestination::handleActivateEvent(sf::Event &ev)
{
	if(const auto* keyPressed = ev.getIf<sf::Event::KeyPressed>())
	{
		if(keyPressed->scancode == sf::Keyboard::Scan::Z)
		{
			moveLevelItLeft();
		}
	}
}

ChooseDestination::ChooseDestination(GameData &d, GameCamera &c, GameLevelManager &lm)
    : data(&d), camera(&c), background(d.guiTextures.at("GUI_10.png")), manager(&lm)
{
	
}

void ChooseDestination::open()
{
	isOpened = true;
	for (auto &&level : levels)
	{
		level.leveldestination.isSelected = false;
	}
	
	levelIt = levels.begin();
	levelIt->leveldestination.isSelected = true;
}

void ChooseDestination::close()
{
	isOpened = false;
	for (auto &&level : levels)
	{
		level.leveldestination.isSelected = false;
	}
}

void ChooseDestination::addLevelInVector(GameLevel& level,LevelDestinationRect l){
	
	l.leveldestination.level =     &level;
	l.leveldestination.isOpened =  false;
	l.leveldestination.isVisible = true;
	
	this->levels.push_back(l);
}

void ChooseDestination::drawLevelDestinations(sf::RenderWindow& window){

	drawLevelDestinationsBackground(window);
	drawLevelDestinationsLevels(window);
}

void ChooseDestination::drawLevelDestinationsBackground(sf::RenderWindow &window)
{
	window.draw(background);
}

void ChooseDestination::drawLevelDestinationsLevels(sf::RenderWindow &window)
{
	for (auto& level : levels) {
		level.draw(window);					
	}
}

void ChooseDestination::update(){
	if(!isOpened) return;
	//Menu back logic
		//Positioning menu relative to player screen
	this->positioningLevelDestinations();	
		//Updating contols
	this->updateControls();


}

void ChooseDestination::updateControls(){
	if(!isOpened) return;
	
}

void ChooseDestination::draw(sf::RenderWindow& w){
	if(!isOpened) return;
	//TODO
	// draw box that consist elements
	// ...

	drawLevelDestinations(w);
}

bool ChooseDestination::getIsOpened()
{
    return this->isOpened;
}

void ChooseDestination::LevelDestinationRect::draw(sf::RenderWindow& w){
	
	// Element's icon
	w.draw(this->icon);
	
	// Selection rect
	if(this->leveldestination.isSelected)
	{
		w.draw(this->selectionRect);
	}
}

void ChooseDestination::currentSelectedElementToDesiredDestination()
{
}

void ChooseDestination::moveLevelItLeft()
{
	levelIt->leveldestination.isSelected = false;
	if(levelIt==levels.begin())
	{
		levelIt == levels.end()-1;
	}
	else
	{
		levelIt--;
		levelIt->leveldestination.isSelected = true;
	}
}

void ChooseDestination::moveLevelItRight()
{
	if(levelIt == levels.end()-1)
	{
		levelIt = levels.begin();
	}
	else
	{
		levelIt++;
	}
}

void ChooseDestination::positioningLevelDestinations()
{

    positioningLevelDestinationsBackground();
	positioningLevelDestinationsLevels();
}

void ChooseDestination::positioningLevelDestinationsBackground()
{
	//Background scaling 
		// Correcting background size based on count of levels 
	sf::Vector2i sumOfLevelsRects = {0,0};
	
	for (auto& level: this->levels) {
		// For x we take width of every rect
		sumOfLevelsRects.x += static_cast<int>( static_cast<float>(level.icon.getTexture().getSize().x) * level.icon.getScale().x ) + BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN.x;	

	}
	// For y we take single height of first element in arr.
	sumOfLevelsRects.y = static_cast<int>(levels.begin()->icon.getTexture().getSize().y) + BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN.y*2; // 2 - is for top and down sides

	sf::Vector2f backGroundSize = static_cast<sf::Vector2f>( background.getTexture().getSize());
	sf::Vector2f backgroundScale = 
	{
		sumOfLevelsRects.x/backGroundSize.x,	
		sumOfLevelsRects.y/backGroundSize.y
	};
	this->background.setScale(backgroundScale);



	//Background positioning
	sf::Vector2f viewTopCenter = {camera->getCameraCenterPos().x, camera->getCameraCenterPos().y - (camera->getScreenViewSize().y/2)}; 	// Верхняя точка камеры на данный момент

	sf::Vector2f backgroundPos = 
	{
		viewTopCenter.x - (background.getTexture().getSize().x * backgroundScale.x), 	// Отступ по X, основываясь на ширине фона меню
		viewTopCenter.y + (background.getTexture().getSize().y * backgroundScale.y)		// Отступ по Y, основываясь на высоте фона меню
	};

	this->background.setPosition(backgroundPos);
}

void ChooseDestination::positioningLevelDestinationsLevels()
{
	//Icon scaling
	sf::Vector2f iconScale;
	for (auto &&level : levels)
	{
		sf::Vector2f iconSize  = static_cast<sf::Vector2f>(level.icon.getTexture().getSize());
		iconScale = 
		{
			BASE_DESTINATION_ICON_SIZE.x/iconSize.x, 
			BASE_DESTINATION_ICON_SIZE.y/iconSize.y
		};

		level.icon.setScale(iconScale);
	}
	/////////////////////////////////////


	// Positioning icons
	sf::Vector2f firstElementIconSizeConsiderScale = static_cast<sf::Vector2f>( levels.begin()->icon.getTexture().getSize( ));
	firstElementIconSizeConsiderScale = {firstElementIconSizeConsiderScale.x*iconScale.x,firstElementIconSizeConsiderScale.y*iconScale.y};

	float fullCalculatedElementXIndent = firstElementIconSizeConsiderScale.x+BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN.x;

	sf::Vector2f firstElementPos = 
	{
		background.getPosition().x+BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN.x,
		background.getPosition().y-BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN.y
	};
	
	for (auto& level : this->levels) {
		level.icon.setPosition(firstElementPos);

		//Indenting
		firstElementPos = 
		{
			(firstElementPos.x + fullCalculatedElementXIndent),
			firstElementPos.y
		};
	}
}
