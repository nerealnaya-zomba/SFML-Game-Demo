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
	LevelDestinationRect lvldstrectLoc;

	lvldstrectLoc.leveldestination.level =     &level;
	lvldstrectLoc.leveldestination.isOpened =  false;
	lvldstrectLoc.leveldestination.isVisible = true;
	

	this->levels.push_back(lvdstLoc);
}

void ChooseDestination::drawLevelDestinations(sf::RenderWindow& window){
	for (auto& level : levels) {
		level.						
	}
}

void ChooseDestination::update(){
	this->updateControls();
}

void ChooseDestination::updateControls(){
	
}

void ChooseDestination::draw(){
		
}

void ChooseDestination::LevelDestinationRect::draw(sf::RenderWindow& w){

}
