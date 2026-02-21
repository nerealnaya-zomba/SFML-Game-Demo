#include "GameCamera.h"
#include "GameData.h"
#include "GameLevel.h"
#include<ChooseDestinationMenu.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <utility>

void ChooseDestinationMenu::handleEvents(const sf::Event &ev)
{
	if(!isOpened) return;

	handleMoveEvents(ev);
	handleActivateEvent(ev);
}

void ChooseDestinationMenu::handleMoveEvents(const sf::Event& ev)
{
	if(const auto* keyPressed = ev.getIf<sf::Event::KeyPressed>())
	{
		if(keyPressed->scancode == this->moveLeftKey)
		{
			moveLevelItLeft();
		}
		else if(keyPressed->scancode == this->moveRightKey)
		{
			moveLevelItRight();
		}
	}
}

void ChooseDestinationMenu::handleActivateEvent(const sf::Event &ev)
{
	if(const auto* keyPressed = ev.getIf<sf::Event::KeyPressed>())
	{
		if(keyPressed->scancode == selectKey)
		{
			currentSelectedElementToDesiredDestination();
			player->setPortalDestination(this->desiredDestination);
		}
	}
}

ChooseDestinationMenu::ChooseDestinationMenu(
    GameData& d, 
    GameCamera& c, 
    GameLevelManager& lm,
	Player& p,
    sf::Keyboard::Scan mvLeftKey, 
    sf::Keyboard::Scan mvRightKey, 
    sf::Keyboard::Scan slctKey)
    : data(&d)
    , camera(&c)
    , manager(&lm)
	, player(&p)
    , background(d.guiTextures.at("GUI_10.png"))
    , moveLeftKey(mvLeftKey)
    , moveRightKey(mvRightKey)
    , selectKey(slctKey)
    , displayingLevelName(LevelDestinationText{false}, *d.gameFont)
    , levelIt(levels.end()) 
    , desiredDestination(std::nullopt)
{
    if (!d.gameFont) {
        throw std::runtime_error("Font not loaded");
    }

}

void ChooseDestinationMenu::open()
{
	isOpened = true;
	checkWherePlayer();	// Sets  isPlayerThere = true  at current player's level.

	// Clearing all selections and setting it to the begin
	for (auto &&level : levels)
	{
		level.leveldestination.isSelected = false;
	}
	
	levelIt = levels.begin();
	levelIt->leveldestination.isSelected = true;
}

void ChooseDestinationMenu::close()
{
	isOpened = false;
	for (auto &&level : levels)
	{
		level.leveldestination.isSelected = false;
	}
}

void ChooseDestinationMenu::addLevelInVector(const GameLevel& level, sf::Texture& icon){
	
	LevelDestinationRect l(icon);
	l.leveldestination.level     = &level;
	l.leveldestination.isOpened  = false ;
	l.leveldestination.isVisible = true  ;

	mountSelectionRect(l.selectionRect,l.icon);
	mountCurrentLevelMarkRect(l.currentLevelMarkRect,l.icon);
	
	this->levels.push_back(l);

	initializeIsChoosed();
}

void ChooseDestinationMenu::addLevelInVector(const GameLevel& level, const sf::Texture& icon)
{
	LevelDestinationRect l(icon);
	l.leveldestination.level     = &level;
	l.leveldestination.isOpened  = false ;
	l.leveldestination.isVisible = true  ;

	mountSelectionRect(l.selectionRect,l.icon);
	mountCurrentLevelMarkRect(l.currentLevelMarkRect,l.icon);
	
	this->levels.push_back(l);

	initializeIsChoosed();
}

void ChooseDestinationMenu::drawLevelDestinations(sf::RenderWindow& window){

	drawLevelDestinationsBackground(window);
	drawLevelDestinationsLevels(window);
}

void ChooseDestinationMenu::drawLevelDestinationsBackground(sf::RenderWindow &window)
{
	window.draw(background);
}

void ChooseDestinationMenu::drawLevelDestinationsLevels(sf::RenderWindow &window)
{
	for (auto& level : levels) {
		level.draw(window);					
	}
}

void ChooseDestinationMenu::update(){
	if(!isOpened) return;
	//Menu back logic
		//Positioning menu relative to player screen
	this->positioningLevelDestinations();	
	checkWherePlayer();
}

void ChooseDestinationMenu::draw(sf::RenderWindow& w){
	if(!isOpened) return;

	drawLevelDestinations(w);

}

bool ChooseDestinationMenu::getIsOpened()
{
    return this->isOpened;
}

std::optional<std::string> ChooseDestinationMenu::getSelectedLevel()
{
    return this->desiredDestination;
}

void ChooseDestinationMenu::LevelDestinationRect::draw(sf::RenderWindow& w){
	
	// Element's icon
	w.draw(this->icon);
	
	// Selection rect
	if(this->leveldestination.isSelected)
	{
		w.draw(this->selectionRect);
	}
	else if(this->leveldestination.isChoosed)
	{
		w.draw(this->currentLevelMarkRect);
	}
}

void ChooseDestinationMenu::currentSelectedElementToDesiredDestination()
{
	this->desiredDestination = std::make_optional<std::string>( levelIt->leveldestination.level->levelName);
	// Setting isChoosed=false to all levels
	for (auto &&level : levels)
	{
		level.leveldestination.isChoosed = false;
	}
	// Setting isChoosed=true to selected level
	levelIt->leveldestination.isChoosed  = true ;
}

std::string ChooseDestinationMenu::getCurrentLevelName()
{
	return manager->getIteratorReference()->second->levelName;
}

void ChooseDestinationMenu::mountSelectionRect(sf::RectangleShape &sr, sf::Sprite& icon)
{
	sf::Vector2f iconSize = static_cast<sf::Vector2f>(icon.getTexture().getSize());
	sf::Vector2f iconScale = icon.getScale();
	iconSize.x*= iconScale.x;
	iconSize.y*= iconScale.y;

	sr.setSize				(iconSize);
	sr.setFillColor			(sf::Color::Transparent);
	sr.setOutlineColor		(BASE_SELECTION_COLOR);
	sr.setOutlineThickness	(BASE_SELECTION_SIZE );
	
	sr.setPosition			(icon.getPosition()    );
}

void ChooseDestinationMenu::mountCurrentLevelMarkRect(sf::RectangleShape &sr, sf::Sprite &icon)
{
	sf::Vector2f iconSize = static_cast<sf::Vector2f>(icon.getTexture().getSize());
	sf::Vector2f iconScale = icon.getScale();
	iconSize.x*= iconScale.x;
	iconSize.y*= iconScale.y;

	sr.setSize				(iconSize);
	sr.setFillColor			(sf::Color::Transparent);
	sr.setOutlineColor		(BASE_LEVELMARK_COLOR  );
	sr.setOutlineThickness	(BASE_LEVELMARK_SIZE   );
	sr.setPosition			(icon.getPosition());
}

void ChooseDestinationMenu::initializeIsChoosed()
{
	checkWherePlayer();
	for (auto &&level : levels)
	{
		if(level.leveldestination.isPlayerThere)
		{
			level.leveldestination.isChoosed = true;
		}
		else
		{
			level.leveldestination.isChoosed = false;
		}
	}
}

void ChooseDestinationMenu::moveLevelItLeft()
{
	levelIt->leveldestination.isSelected = false;
	if(levelIt==levels.begin())
	{
		levelIt = levels.end()-1;
		levelIt->leveldestination.isSelected = true;
	}
	else
	{
		levelIt--;
		levelIt->leveldestination.isSelected = true;
	}
}

void ChooseDestinationMenu::moveLevelItRight()
{
	levelIt->leveldestination.isSelected = false;
	if(levelIt == levels.end()-1)
	{
		levelIt = levels.begin();
		levelIt->leveldestination.isSelected = true;
	}
	else
	{
		levelIt++;
		levelIt->leveldestination.isSelected = true;
	}
}

void ChooseDestinationMenu::positioningLevelDestinations()
{
    positioningLevelDestinationsBackground();
	positioningLevelDestinationsLevels();
}

void ChooseDestinationMenu::positioningLevelDestinationsBackground()
{
    if (levels.empty()) return;
    
    // Размеры иконок после масштабирования (уже известны)
    float iconWidth = BASE_DESTINATION_ICON_SIZE.x;
    float iconHeight = BASE_DESTINATION_ICON_SIZE.y;
    float marginX = static_cast<float>(BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN.x);
    float marginY = static_cast<float>(BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN.y);
    
    // Вычисляем нужный размер фона
    int totalWidth = static_cast<int>(
        levels.size() * iconWidth +          // суммарная ширина всех иконок
        (levels.size() + 1) * marginX        // отступы слева, справа и между иконками
    );
    
    int totalHeight = static_cast<int>(
        iconHeight +                         // высота одной иконки
        2 * marginY                          // отступы сверху и снизу
    );
    
    // Масштабируем фон
    sf::Vector2f bgTextureSize = static_cast<sf::Vector2f>(background.getTexture().getSize());
    sf::Vector2f bgScale = {
        static_cast<float>(totalWidth) / bgTextureSize.x,
        static_cast<float>(totalHeight) / bgTextureSize.y
    };
    background.setScale(bgScale);
    
    // Позиционируем фон относительно камеры
    sf::Vector2f viewTopCenter = {
        camera->getCameraCenterPos().x,
        camera->getCameraCenterPos().y - (camera->getScreenViewSize().y / 2)
    };
    
    // Фон позиционируется так, чтобы его верхний левый угол был в точке viewTopCenter
    // (или как тебе нужно - можешь скорректировать формулу)
    sf::Vector2f bgPosition = {
        viewTopCenter.x - (totalWidth/2),
        viewTopCenter.y + BASE_DESTINATION_BACKGROUND_TOPMARGIN
    };
    
    background.setPosition(bgPosition);
}

void ChooseDestinationMenu::positioningLevelDestinationsLevels()
{
    if (levels.empty()) return;
    
    // 1. Сначала масштабируем все иконки под единый размер
    for (auto& level : levels) {
        sf::Vector2f iconSize = static_cast<sf::Vector2f>(level.icon.getTexture().getSize());
        sf::Vector2f scale = {
            BASE_DESTINATION_ICON_SIZE.x / iconSize.x,
            BASE_DESTINATION_ICON_SIZE.y / iconSize.y
        };
        level.icon.setScale(scale);
    }
    
    // 2. Теперь позиционируем иконки
    float iconWidth = BASE_DESTINATION_ICON_SIZE.x;
    float iconHeight = BASE_DESTINATION_ICON_SIZE.y;
    float marginX = static_cast<float>(BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN.x);
    float marginY = static_cast<float>(BASE_DESTINATION_ICON_TOPDOWNRIGHT_MARGIN.y);
    
    // Стартовая позиция: левый верхний угол первой иконки
    sf::Vector2f startPos = {
        background.getPosition().x + marginX,
        background.getPosition().y + marginY
    };
    
    sf::Vector2f currentPos = startPos;
    
    for (auto& level : levels) {
        level.icon.setPosition(currentPos);
        
        mountSelectionRect(level.selectionRect, level.icon);
        mountCurrentLevelMarkRect(level.currentLevelMarkRect, level.icon);
        
        currentPos.x += iconWidth + marginX;  // сдвиг вправо на ширину иконки + отступ
    }
}

void ChooseDestinationMenu::positioningLevelDestinationsText()
{
	sf::Vector2f displayingLevelNamePos;		
	sf::Vector2f displayingLevelNameSize;		
	
	sf::Vector2f backgroundPos = this->background.getPosition();
	sf::Vector2f backgroundScale = this->background.getScale();
	sf::Vector2f backgroundSize = static_cast<sf::Vector2f>( this->background.getTexture().getSize() );
	sf::Vector2f backgroundScaledSize = {( backgroundSize.x * backgroundScale.x ), ( backgroundSize.y * backgroundScale.y )};

	displayingLevelNamePos = this->displayingLevelName.second.getPosition();
	displayingLevelNameSize = this->displayingLevelName.second.getGlobalBounds().size;

	if(displayingLevelNameSize.x>0 && displayingLevelNameSize.y>0)
	{
		this->displayingLevelName.first.isVisible = true;	
	}
	else
	{
		this->displayingLevelName.first.isVisible = false;
	}

	displayingLevelName.second.setPosition
			(
			{
				
			}
			);

}

void ChooseDestinationMenu::checkWherePlayer()
{
	std::string currentLevelName = getCurrentLevelName();

	// Setting false for all levels
	for (auto &&level : levels)
	{
		level.leveldestination.isPlayerThere = false;
	}
	
	// Find first level that matches level name, and set its  isPlayerThere  to  true
	for (auto &&level : levels)
	{
		if(level.leveldestination.level->levelName == currentLevelName)
		{
			level.leveldestination.isPlayerThere = true;
			return;
		}
	}

}
