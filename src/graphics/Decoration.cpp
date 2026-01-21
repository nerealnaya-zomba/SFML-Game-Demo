#include "Decoration.h"

Decoration::Decoration(GameData& gameTextures, GameCamera& c)
: data(&gameTextures), camera(&c)
{
    //Plants
    attachTexture(gameTextures.plant1Textures,      this->plant1Textures,       gameTextures.plant1,        this->plant1        );
    attachTexture(gameTextures.plant2Textures,      this->plant2Textures,       gameTextures.plant2,        this->plant2        );
    attachTexture(gameTextures.plant3Textures,      this->plant3Textures,       gameTextures.plant3,        this->plant3        );
    attachTexture(gameTextures.plant4Textures,      this->plant4Textures,       gameTextures.plant4,        this->plant4        );
    attachTexture(gameTextures.plant5Textures,      this->plant5Textures,       gameTextures.plant5,        this->plant5        );
    attachTexture(gameTextures.plant6Textures,      this->plant6Textures,       gameTextures.plant6,        this->plant6        );
    attachTexture(gameTextures.plant7Textures,      this->plant7Textures,       gameTextures.plant7,        this->plant7        );
    //Cat
    attachTexture(gameTextures.jumpPlantTextures,   this->jumpPlantTextures,    gameTextures.jumpPlant,     this->jumpPlant     );
    //jumpPlant
    attachTexture(gameTextures.cat1Textures,        this->cat1Textures,         gameTextures.catHelper,     this->catHelper     );
    //portalGreen
    attachTexture(gameTextures.portalGreenTextures, this->portalGreenTextures,  gameTextures.portalGreen,   this->portalGreen   );
    //Static-textures
    attachTexture(gameTextures.allStaticTextures,   this->staticTextures                                                        );

    pushPointersOfUnorderedMultimapsToVector();
}

Decoration::~Decoration()
{
}

void Decoration::addDecoration(std::string name,sf::Vector2f position, sf::Vector2f scale, sf::Vector2f parallaxFactor, int z, sf::Color color)
{
    if(name == "plant1")
    {
        initDecoration(position,scale,parallaxFactor,z,color,plant1Sprites,plant1Textures);
    }
    else if(name == "plant2")
    {
        initDecoration(position,scale,parallaxFactor,z,color,plant2Sprites,plant2Textures);
    }
    else if(name == "plant3")
    {
        initDecoration(position,scale,parallaxFactor,z,color,plant3Sprites,plant3Textures);
    }
    else if(name == "plant4")
    {
        initDecoration(position,scale,parallaxFactor,z,color,plant4Sprites,plant4Textures);
    }
    else if(name == "plant5")
    {
        initDecoration(position,scale,parallaxFactor,z,color,plant5Sprites,plant5Textures);
    }
    else if(name == "plant6")
    {
        initDecoration(position,scale,parallaxFactor,z,color,plant6Sprites,plant6Textures);
    }
    else if(name == "plant7")
    {
        initDecoration(position,scale,parallaxFactor,z,color,plant7Sprites,plant7Textures);
    }
    else if(name == "cat")
    {
        initDecoration(position,scale,parallaxFactor,z,color,cat1Sprites,cat1Textures);
    }
    else if(name == "jumpPlant")
    {
        initDecoration(position,scale,parallaxFactor,z,color,jumpPlantSprites,jumpPlantTextures);
    }
    else if(name == "portalGreen")
    {
        initDecoration(position,scale,parallaxFactor,z,color,portalGreenSprites,portalGreenTextures);
    }
    //Static-textures assertion
    else{
        try{
            initDecoration(name,position,scale,parallaxFactor,z,color,staticSprites,staticTextures);
        }
        catch(std::out_of_range& ex){
            std::cout << ex.what() << std::endl;
            
            exit(1);
        }
    }
    
    
}

void Decoration::pushPointersOfUnorderedMultimapsToVector()
{
    multimap_pointers.push_back(&this->plant1Sprites);
    multimap_pointers.push_back(&this->plant2Sprites);   
    multimap_pointers.push_back(&this->plant3Sprites);   
    multimap_pointers.push_back(&this->plant4Sprites);   
    multimap_pointers.push_back(&this->plant5Sprites);   
    multimap_pointers.push_back(&this->plant6Sprites);   
    multimap_pointers.push_back(&this->plant7Sprites);   
    multimap_pointers.push_back(&this->staticSprites);   
    multimap_pointers.push_back(&this->jumpPlantSprites);
    multimap_pointers.push_back(&this->cat1Sprites);   
    multimap_pointers.push_back(&this->portalGreenSprites);      
}

void Decoration::switchToNextSprite(std::vector<std::unique_ptr<sf::Sprite>> &spritesArray, std::vector<sf::Texture> &texturesArray, texturesIterHelper &iterHelper)
{
    
    if(iterHelper.iterationCounter<iterHelper.iterationsTillSwitch)
    {
        iterHelper.iterationCounter++;
    }
    else
    {
        //Forward-backward logic
        if(iterHelper.ptrToTexture == iterHelper.countOfTextures)
        {
            iterHelper.goForward = false;
        }
        else if(iterHelper.ptrToTexture == 0)
        {
            iterHelper.goForward = true;
        }

        //Switch sprites
        for (auto &i : spritesArray)
        {
            i->setTexture(texturesArray.at(iterHelper.ptrToTexture));
        }

        //Forward-backward logic
        if(iterHelper.goForward)
        {
            iterHelper.ptrToTexture++;
        }
        else
        {
            iterHelper.ptrToTexture--;
        }

        //reset iteration counter after all switches
        iterHelper.iterationCounter = 0;
    }
}

void Decoration::switchToNextSprite(std::unordered_multimap<
        Vector2fPairWithZ,
        std::unique_ptr<sf::Sprite>,Vector2fPairWithZHash,Vector2fPairWithZEqual>& spritesArray, 
        std::vector<sf::Texture>& texturesArray, 
        texturesIterHelper& iterHelper)
{
    if(iterHelper.iterationCounter<iterHelper.iterationsTillSwitch)
    {
        iterHelper.iterationCounter++;
    }
    else
    {
        //Forward-backward logic
        if(iterHelper.ptrToTexture == iterHelper.countOfTextures)
        {
            iterHelper.goForward = false;
        }
        else if(iterHelper.ptrToTexture == 0)
        {
            iterHelper.goForward = true;
        }

        //Switch sprites
        for (auto &&i : spritesArray)
        {
            i.second->setTexture(texturesArray.at(iterHelper.ptrToTexture));
        }

        //Forward-backward logic
        if(iterHelper.goForward)
        {
            iterHelper.ptrToTexture++;
        }
        else
        {
            iterHelper.ptrToTexture--;
        }

        //reset iteration counter after all switches
        iterHelper.iterationCounter = 0;
    }
}

void Decoration::updateParallax()
{
    for (auto &i : plant1Sprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
    for (auto &i : plant2Sprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
    for (auto &i : plant3Sprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
    for (auto &i : plant4Sprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
    for (auto &i : plant5Sprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
    for (auto &i : plant6Sprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
    for (auto &i : plant7Sprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
    for (auto &i : cat1Sprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
    for (auto &i : jumpPlantSprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
    for (auto &&i : portalGreenSprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
    for (auto &&i : staticSprites)
    {
        applyParalaxes(i.first.posData,i.second);
    }
}

void Decoration::applyParalaxes(
        const std::pair<sf::Vector2f, sf::Vector2f>& vectorPair ,   // for std::pair : first - parallaxFactor, second - baseObjectPos  
        const std::unique_ptr<sf::Sprite>& sprite                   // second arg. in std::unordered_map
) {
    sf::Vector2f baseObjectPos = vectorPair.second;
    sf::Vector2f parallaxFactor = vectorPair.first;
    
    // Добавляем статическую переменную для хранения начальной позиции камеры
    static sf::Vector2f initialCameraPos = camera->getCameraCenterPos();
    sf::Vector2f currentCameraPos = camera->getCameraCenterPos();
    
    // Вычисляем смещение камеры от её начальной позиции
    sf::Vector2f cameraOffset = currentCameraPos - initialCameraPos;

    sf::Vector2f difference = baseObjectPos - camera->getCameraCenterPos();
    
    if (sprite.get()) {
        sprite.get()->setPosition({
            baseObjectPos.x + cameraOffset.x * parallaxFactor.x,
            baseObjectPos.y + cameraOffset.y * parallaxFactor.y
        });
    }
    
}

void Decoration::initDecoration(sf::Vector2f position, sf::Vector2f scale, sf::Vector2f parallaxFactor, int z, sf::Color color,
    std::unordered_multimap<Vector2fPairWithZ, std::unique_ptr<sf::Sprite>, Vector2fPairWithZHash, Vector2fPairWithZEqual>& sprites,
    std::vector<sf::Texture>* textures)
{
        if(textures == nullptr || textures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(textures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);

        sf::Vector2f difference = position - camera->getCameraCenter();
        sf::Vector2f calculatedWithParallaxPos =    // NOTE Работает несовсем правильно. Наверное стоит дописать формулу.
            {
                position.x - parallaxFactor.x,
                position.y - parallaxFactor.y
            };
        all_Z.insert(z);
        sprites.emplace(Vector2fPairWithZ(std::pair(parallaxFactor,calculatedWithParallaxPos),z),std::move(sprite));
}

void Decoration::initDecoration(std::string& name, sf::Vector2f position, sf::Vector2f scale, sf::Vector2f parallaxFactor, int z, sf::Color color, std::unordered_multimap<Vector2fPairWithZ, std::unique_ptr<sf::Sprite>, Vector2fPairWithZHash, Vector2fPairWithZEqual> &sprites, std::map<std::string, sf::Texture> *textures)
{
            auto sprite = std::make_unique<sf::Sprite>(staticTextures->at(name));
            sprite->setOrigin(sprite->getGlobalBounds().getCenter());
            sprite->setPosition(position);
            sprite->setScale(scale);
            sprite->setColor(color);
            all_Z.insert(z);
            staticSprites.emplace(Vector2fPairWithZ(std::pair(parallaxFactor,position),z),std::move(sprite));
}

void Decoration::generateMipmapTextures(std::vector<sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        if(i.generateMipmap())
        {
            std::cout << "Mipmap generated\n"; 
        }
        else
        {
            std::cout << "Error while generating mipmap\n";
        }
    }
    
}

void Decoration::smoothTextures(std::vector<sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        i.setSmooth(true);
    }
    
}

void Decoration::updateTextures()
{

    switchToNextSprite(plant1Sprites,*plant1Textures,plant1);
    switchToNextSprite(plant2Sprites,*plant2Textures,plant2);
    switchToNextSprite(plant3Sprites,*plant3Textures,plant3);



    switchToNextSprite(plant4Sprites,*plant4Textures,plant4);
    switchToNextSprite(plant5Sprites,*plant5Textures,plant5);
    switchToNextSprite(plant6Sprites,*plant6Textures,plant6);
    switchToNextSprite(plant7Sprites,*plant7Textures,plant7);



    switchToNextSprite(cat1Sprites,*cat1Textures,catHelper);

    switchToNextSprite(jumpPlantSprites,*jumpPlantTextures,jumpPlant);

    switchToNextSprite(portalGreenSprites,*portalGreenTextures,portalGreen);

    updateParallax();
}

void Decoration::drawByZOrder(sf::RenderWindow& window)
{
    for (int z : all_Z) {
        // 3. Проходим по всем картам
        for (auto* spriteMap : multimap_pointers) {
            // 4. Проходим по всем элементам в текущей карте
            for (const auto& pair : *spriteMap) {
                // 5. Если Z совпадает - рисуем
                if (pair.first.z == z) {
                    if (pair.second) {  // Проверка на nullptr
                        window.draw(*pair.second);
                    }
                }
            }
        }
    }
    
}

void Decoration::draw(sf::RenderWindow &window)
{
    drawByZOrder(window);
}

void Decoration::clearDecorations()
{
    for (auto &&i : multimap_pointers)
    {
        i->clear();
    }
}
