#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<iostream>
#include<string>
#include<Mounting.h>
#include<GameData.h>
#include<TexturesIterHelper.h>
#include<unordered_map>
#include<bitset>
#include<GameCamera.h>
#include<set>

class GameCamera;
////////////////////////////////////////////////////////////////////////////////
// Кастомные кеш-функторы.
// Нужны для работы unordered_multimap
////////////////////////////////////////////////////////////////////////////////
struct Vector2fHash{
    std::size_t operator()(const sf::Vector2f& v) const{
        std::size_t h1 = std::hash<float>{}(v.x);
        std::size_t h2 = std::hash<float>{}(v.y);
        return h1 ^ (h2 << 1);
    }
};
struct Vector2fEqual {
    bool operator()(const sf::Vector2f& a, const sf::Vector2f& b) const {
        return a.x == b.x && a.y == b.y;
    }
};
struct Vector2fPairHash {
    std::size_t operator()(const std::pair<sf::Vector2f,sf::Vector2f>& pv) const {
        std::size_t h1 = std::hash<float>{}(pv.second.x);
        std::size_t h2 = std::hash<float>{}(pv.second.y);
        std::size_t h3 = std::hash<float>{}(pv.first.x);
        std::size_t h4 = std::hash<float>{}(pv.first.y);
        return ((h1 ^ (h2 << 1)) ^ (h3 << 2)) ^ (h4 << 3);
    }
};
struct Vector2fPairEqual {
    bool operator()(const std::pair<sf::Vector2f,sf::Vector2f>& pva, const std::pair<sf::Vector2f,sf::Vector2f>& pvb) const {
        return  (pva.second.x == pvb.second.x && pva.second.y == pvb.second.y) && 
                (pva.first.x  == pvb.first.x  && pva.first.y  == pvb.first.y );
    }
};
struct Vector2fPairWithZ {
    std::pair<sf::Vector2f, sf::Vector2f> posData;
    int z;

    Vector2fPairWithZ() = default;
    Vector2fPairWithZ(const std::pair<sf::Vector2f,sf::Vector2f>& p, int& zi) 
        : posData(p), z(zi) {};
    Vector2fPairWithZ(std::pair<sf::Vector2f, sf::Vector2f>&& p, int zi) 
        : posData(std::move(p)), z(zi) {};
};
struct Vector2fPairWithZHash {
    std::size_t operator()(const Vector2fPairWithZ& data) const {
        Vector2fPairHash pairHasher;
        std::size_t h1 = pairHasher(data.posData);
        std::size_t h2 = std::hash<int>{}(data.z);
        return h1 ^ (h2 << 1);
    }
};
struct Vector2fPairWithZEqual {
    bool operator()(const Vector2fPairWithZ& a, const Vector2fPairWithZ& b) const {
        Vector2fPairEqual pairEqual;
        return pairEqual(a.posData, b.posData) && (a.z == b.z);
    }
};
////////////////////////////////////////////////////////////
// Основной класс для отображения декораций
////////////////////////////////////////////////////////////
class Decoration 
{
public: 
    // Запрещаем копирование
    Decoration(const Decoration&) = delete;
    Decoration& operator=(const Decoration&) = delete;

    // Разрешаем перемещение
    Decoration(Decoration&&) = default;
    Decoration& operator=(Decoration&&) = default;

    Decoration(GameData& gameTextures, GameCamera& c);
    ~Decoration();

    void addDecoration(std::string name,sf::Vector2f position, sf::Vector2f scale, sf::Vector2f parallaxFactor, int z = 0, sf::Color color = sf::Color::White);
    void updateTextures();
    void drawByZOrder(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    void clearDecorations();

    //Textures and iters pointers
        //Plants
    std::vector<sf::Texture>* plant1Textures;
    texturesIterHelper plant1;

    std::vector<sf::Texture>* plant2Textures;
    texturesIterHelper plant2;

    std::vector<sf::Texture>* plant3Textures;
    texturesIterHelper plant3;

    std::vector<sf::Texture>* plant4Textures;
    texturesIterHelper plant4;

    std::vector<sf::Texture>* plant5Textures;
    texturesIterHelper plant5;

    std::vector<sf::Texture>* plant6Textures;
    texturesIterHelper plant6;

    std::vector<sf::Texture>* plant7Textures;
    texturesIterHelper plant7;
    
    std::vector<sf::Texture>* jumpPlantTextures;
    texturesIterHelper jumpPlant;
        //Cat
    std::vector<sf::Texture>* cat1Textures;
    texturesIterHelper catHelper;
        //Portal
    std::vector<sf::Texture>* portalGreenTextures;
    texturesIterHelper portalGreen;

    
    //Static-textures
    std::map<std::string,sf::Texture>* staticTextures;
        
    private:
    // Pointers to internal data
    GameData* data;
    GameCamera* camera;
    //Sprites
        //Plants
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > plant1Sprites; 
    
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > plant2Sprites;
    
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > plant3Sprites;
    
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > plant4Sprites;
    
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > plant5Sprites;
    
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > plant6Sprites;
    
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > plant7Sprites;

    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > jumpPlantSprites;
        //Cat
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > cat1Sprites;
        //Portal green
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > portalGreenSprites;
        //PortalBlue
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > portal1BlueSprites;
    
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > portal2BlueSprites;

    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > portal3BlueSprites;

    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > portal4BlueSprites;

    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > portal5BlueSprites;

    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > portal6BlueSprites;

    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > portal7BlueSprites;

    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > portal8BlueSprites;
    
    //Static-sprites
    std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    > staticSprites;

    // Указатели на все массивы со спрайтами
    std::vector<std::unordered_multimap<
    Vector2fPairWithZ,
    std::unique_ptr<sf::Sprite>,
    Vector2fPairWithZHash,
    Vector2fPairWithZEqual
    >*> multimap_pointers;
    // Массив всех существующих Z позиций
    std::set<int> all_Z;

    void pushPointersOfUnorderedMultimapsToVector();

    //Switches sprite's texture to next
    void switchToNextSprite(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper);
    void switchToNextSprite(
        std::unordered_multimap<
        Vector2fPairWithZ,
        std::unique_ptr<sf::Sprite>,Vector2fPairWithZHash,Vector2fPairWithZEqual>& spritesArray, 
        std::vector<sf::Texture>& texturesArray, 
        texturesIterHelper& iterHelper
    );

    void updateParallax();
    void applyParalaxes(
        const std::pair<sf::Vector2f, sf::Vector2f>& vectorPair ,  
        const std::unique_ptr<sf::Sprite>& sprite 
    );

    //Utils
    void initDecoration(sf::Vector2f position, sf::Vector2f scale, sf::Vector2f parallaxFactor, int z, sf::Color color,
    std::unordered_multimap<Vector2fPairWithZ, std::unique_ptr<sf::Sprite>, Vector2fPairWithZHash, Vector2fPairWithZEqual>& sprites,
    std::vector<sf::Texture>* textures);
    void initDecoration(std::string& name, sf::Vector2f position, sf::Vector2f scale, sf::Vector2f parallaxFactor, int z, sf::Color color,
    std::unordered_multimap<Vector2fPairWithZ, std::unique_ptr<sf::Sprite>, Vector2fPairWithZHash, Vector2fPairWithZEqual>& sprites,
    std::map<std::string,sf::Texture>* textures);
    void generateMipmapTextures(std::vector<sf::Texture>& texturesArray);
    void smoothTextures(std::vector<sf::Texture>& texturesArray);

};      
