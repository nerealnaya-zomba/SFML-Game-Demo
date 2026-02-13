#include "GameCamera.h"
#include "GameData.h"
#include "GameLevel.h"
#include<ChooseDestination.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

ChooseDestination::ChooseDestination(GameData& d, GameCamera& c, GameLevelManager& lm)
	: data(&d), camera(&c), background(d.guiTextures.at("GUI_10.png")), manager(&lm)
{
	
}

void ChooseDestination::addLevelInMap(GameLevel& level,LevelDestinationRect l){
	
	l.leveldestination.level =     &level;
	l.leveldestination.isOpened =  false;
	l.leveldestination.isVisible = true;
	
	this->levels.push_back(l);
}

void ChooseDestination::drawLevelDestinations(sf::RenderWindow& window){
	for (auto& level : levels) {
		level.draw(window);					
	}
}

void ChooseDestination::update(){
	this->updateControls();
}

void ChooseDestination::updateControls(){
	
}

void ChooseDestination::draw(sf::RenderWindow& w){
	//TODO
	// draw box that consist elements
	// ...

	drawLevelDestinations(w);
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

void ChooseDestination::updateLevelDestinations(){
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



	//TODO write logic for positioning background
	// ...
	// ...	

	sf::Vector2f firstElementIconSize = static_cast<sf::Vector2f>( levels.begin()->icon.getTexture().getSize( ));
	for (auto& level : this->levels) {
		//TODO write logic for positioning icons		
		// ...
		// ...
	}
}

