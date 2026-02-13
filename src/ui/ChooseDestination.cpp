#include "GameCamera.h"
#include "GameData.h"
#include "GameLevel.h"
#include<ChooseDestination.h>
#include <SFML/Graphics/RenderWindow.hpp>

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
