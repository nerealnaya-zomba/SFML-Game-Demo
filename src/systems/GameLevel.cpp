#include "Defines.h"
#include "Mounting.h"
#include "nlohmann/json_fwd.hpp"
#include <GameLevel.h>
#include <MiniLocationEntrance.h>
#include <WorldPortal.h>
#include <WorldInteractable.h>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <TGUI/AllWidgets.hpp>
#include <TGUI/Backend/Font/SFML-Graphics/BackendFontSFML.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <limits>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <Player.h>
#include <Localization.h>

#include <unordered_map>

namespace
{
constexpr sf::Vector2f kMiniLocationPortalScale{0.22f, 0.33f};
constexpr sf::Vector2f kWorldNameplateScale{0.30f, 0.30f};

std::string lowercaseAscii(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

Type parseBackgroundType(const nlohmann::json& value)
{
    if (value.is_number_integer())
    {
        return value.get<int>() == 1 ? Type::SingleBackground : Type::RepeatedBackgroundXY;
    }

    if (!value.is_string())
    {
        return Type::RepeatedBackgroundXY;
    }

    const std::string typeName = lowercaseAscii(value.get<std::string>());
    if (typeName == "single")
    {
        return Type::SingleBackground;
    }
    if (typeName == "repeatedx" || typeName == "repeated_x")
    {
        return Type::RepeatedBackgroundX;
    }
    if (typeName == "repeatedy" || typeName == "repeated_y")
    {
        return Type::RepeatedBackgroundY;
    }

    return Type::RepeatedBackgroundXY;
}
constexpr std::array<const char*, 20> kWorldNameplateTextures{
    "nameplate_01.png",
    "nameplate_02.png",
    "nameplate_03.png",
    "nameplate_04.png",
    "nameplate_05.png",
    "nameplate_06.png",
    "nameplate_07.png",
    "nameplate_08.png",
    "nameplate_09.png",
    "nameplate_10.png",
    "nameplate_11.png",
    "nameplate_12.png",
    "nameplate_13.png",
    "nameplate_14.png",
    "nameplate_15.png",
    "nameplate_16.png",
    "nameplate_17.png",
    "nameplate_18.png",
    "nameplate_19.png",
    "nameplate_20.png"
};

std::string localizeDataText(const std::string& text)
{
    if (!Localization::isRussian() || text.empty())
    {
        return text;
    }

    static const std::unordered_map<std::string, std::string> translations = {
        {"Veilfall Cascades", "Водопады Вуали"},
        {"Skybound Spires", "Небесные шпили"},
        {"Sunset Bastion", "Закатный бастион"},
        {"Grave Hollow", "Могильная лощина"},
        {"Ruins of the Elder City", "Руины старого города"},
        {"Aurora Nightpeaks", "Ночные пики Авроры"},
        {"Opening climb", "Первый подъем"},
        {"The first ascent is gentle. Use it to settle movement, camera rhythm and your opening shots.", "Первый подъем мягкий. Используйте его, чтобы привыкнуть к движению, ритму камеры и первым выстрелам."},
        {"Hunter's bottleneck", "Узкое место охотника"},
        {"The route narrows here. Secure the upper ledges before committing to the longer crossing.", "Здесь путь сужается. Закрепитесь на верхних уступах, прежде чем идти на длинный переход."},
        {"Enter the hidden route", "Enter - войти на скрытый путь"},
        {"Enter to return", "Enter - вернуться"},
        {"Mini Location 1", "Мини-локация 1"},
        {"Portal 1", "Портал 1"},
        {"Enter portal", "Enter - войти в портал"},
        {"Rain-stiffened markings speak of beasts nesting where the path narrows. Draw them into open ground before you spend too much strength climbing.", "Следы, застывшие под дождем, говорят о тварях, гнездящихся там, где путь сужается. Выманите их на открытое место, прежде чем потратите слишком много сил на подъем."},
        {"Enter to listen", "Enter - слушать"},
        {"Storm Echo", "Эхо бури"},
        {"The clasp snaps and a clutch of grave-gold drops into your palm. Whoever hid it never made the return climb.", "Застежка щелкает, и горсть могильного золота падает в ладонь. Тот, кто спрятал его, так и не вернулся наверх."},
        {"Enter to pry open cache", "Enter - вскрыть тайник"},
        {"Hunter's Purse", "Кошель охотника"},
        {"A wet leather purse slides loose from the stone after your approach.", "После вашего приближения мокрый кожаный кошель выскальзывает из камня."},
        {"Open sky pressure", "Давление открытого неба"},
        {"Flying threats dominate this air lane. Commit to high ground only if you can clear it quickly.", "Летающие угрозы владеют этой воздушной полосой. Забирайтесь наверх только если сможете быстро ее зачистить."},
        {"Storm crossing", "Штормовой переход"},
        {"The storm path rewards short resets. Dash through gaps, then recover on the broad stones.", "Штормовой путь вознаграждает короткие передышки. Рывком проходите разрывы, затем восстанавливайтесь на широких камнях."},
        {"Rookery burst", "Всплеск Гнездовья"},
        {"The rookery entrance erupts fast. Break the first wave immediately or the ledge becomes a trap.", "Вход в Гнездовье взрывается быстро. Сломайте первую волну сразу, иначе уступ станет ловушкой."},
        {"Air pocket", "Воздушный карман"},
        {"Wraiths own the air pocket ahead. Enter ready to shoot upward while repositioning between landings.", "Призраки владеют воздушным карманом впереди. Входите готовыми стрелять вверх и менять позицию между приземлениями."},
        {"Storm descent", "Штормовой спуск"},
        {"Ground hunters dominate the long descent. Take the broad stones and deny them easy flanks.", "Наземные охотники владеют длинным спуском. Держите широкие камни и не давайте им простых флангов."},
        {"Blood-Moon Shrine", "Святилище кровавой луны"},
        {"Enter to bind blood-moon shrine", "Enter - привязать святилище кровавой луны"},
        {"The crimson lamp hums with a low vow. Mark this perch as your return point and draw enough breath to survive the next leap.", "Багровая лампа гудит низкой клятвой. Отметьте этот уступ как точку возврата и вдохните достаточно, чтобы пережить следующий прыжок."},
        {"Prism Shards", "Осколки призмы"},
        {"A crimson shimmer catches under the ledge. There is a cache tucked into the stone.", "Под уступом вспыхивает багровый отблеск. В камне спрятан тайник."},
        {"Enter to gather prism shards", "Enter - собрать осколки призмы"},
        {"A velvet wrap protects a cache of sharp-edged prisms. Even cracked, they buy a surprising amount of ammunition and favor.", "Бархатная обертка хранит острые призмы. Даже треснувшие, они стоят удивительно много боеприпасов и благосклонности."},
        {"Crimson pressure", "Багровое давление"},
        {"This nave pushes you to alternate between low cover and aggressive climbs. Staying in one lane too long gets expensive.", "Этот неф заставляет чередовать низкие укрытия и резкие подъемы. Слишком долго стоять на одной линии дорого обходится."},
        {"High-choir opening", "Вход верхнего хора"},
        {"The upper span opens the level. Clear the bats here and the rest of the route becomes easier to read.", "Верхний пролет открывает уровень. Зачистите летучих тварей здесь, и дальше путь станет понятнее."},
        {"Nave procession", "Процессия нефа"},
        {"The first hall mixes grounded pressure and awkward footing. Win the center before moving on.", "Первый зал смешивает давление с земли и неудобную опору. Заберите центр, прежде чем идти дальше."},
        {"Red seep", "Красная течь"},
        {"Slimes begin to spill through the lower route as soon as you commit to it. Keep momentum and don't overstay.", "Слаймы начинают проливаться через нижний путь, как только вы на него заходите. Держите темп и не задерживайтесь."},
        {"Choir bats", "Летучие твари хора"},
        {"The upper nave becomes dangerous the moment the bats stir. Clear the air before taking the next bridge.", "Верхний неф становится опасным, как только шевелятся летучие твари. Зачистите воздух перед следующим мостом."},
        {"Burial Record", "Погребальная запись"},
        {"Enter to read burial record", "Enter - прочесть погребальную запись"},
        {"These names were crossed out one by one. The last line remains: If the crypt grows too wide, leave a shrine of your own and do not trust the quiet.", "Эти имена вычеркивали одно за другим. Осталась последняя строка: если крипта станет слишком широкой, оставь собственное святилище и не доверяй тишине."},
        {"Crypt-Sanctified Shrine", "Освященное криптой святилище"},
        {"Enter to sanctify this crossing", "Enter - освятить этот переход"},
        {"A patient glow settles over the stone span. The shrine remembers you here, patches your wounds and waits in silence for the next fall.", "Терпеливое сияние ложится на каменный пролет. Святилище запоминает вас здесь, залечивает раны и молча ждет следующего падения."},
        {"Beam Lens Cache", "Тайник линзы луча"},
        {"A burial seam loosens under your steps. There is a hidden case set into the reliquary wall.", "Погребальный шов расходится под вашими шагами. В стену реликвария вмонтирован скрытый футляр."},
        {"Enter to claim beam lens", "Enter - забрать линзу луча"},
        {"A brass case clicks open around a rare lens and a purse of marked coin. Whoever carried it expected to pierce armor, not vanish.", "Латунный футляр щелкает, открывая редкую линзу и кошель меченых монет. Тот, кто нес это, рассчитывал пробивать броню, а не исчезнуть."},
        {"Buried approach", "Погребальный подход"},
        {"This reliquary teaches patience. Let grounded threats come to you before taking the next rise.", "Этот реликварий учит терпению. Пусть наземные угрозы подойдут сами, прежде чем брать следующий подъем."},
        {"Reliquary crossfire", "Перекрестный огонь реликвария"},
        {"The crypt opens overhead here. Expect pressure from both the floor and the rafters.", "Здесь крипта раскрывается над головой. Ждите давления и с пола, и со стропил."},
        {"Burial guard", "Погребальная стража"},
        {"The reliquary opens with a layered patrol. Win the floor before climbing into the central crypt.", "Реликварий открывается многоярусным патрулем. Заберите пол, прежде чем лезть в центральную крипту."},
        {"Floor hunters", "Охотники пола"},
        {"Scorpions own the lower stones and punish indecision. Kite them into broader spaces before finishing them.", "Скорпионы владеют нижними камнями и карают нерешительность. Выманивайте их на широкие места, прежде чем добивать."},
        {"Crypt rafters", "Стропила крипты"},
        {"The upper crypt comes alive once you enter it. Expect pressure from above while you hold the crossing.", "Верхняя крипта оживает, когда вы входите. Ждите давления сверху, пока держите переход."},
        {"Rift Shrine", "Святилище разлома"},
        {"Enter to attune the rift shrine", "Enter - настроить святилище разлома"},
        {"The shrine drinks the twilight crackle and gives it back as calm. It fixes your return point and restores your strength for the next breach.", "Святилище пьет сумеречный треск и возвращает его покоем. Оно фиксирует точку возврата и восстанавливает силы для следующего разлома."},
        {"Night Relic Bundle", "Связка ночных реликвий"},
        {"The trial shifts and a hidden bundle becomes visible near the shrine path.", "Испытание смещается, и возле пути к святилищу становится видна скрытая связка."},
        {"Enter to gather night relics", "Enter - собрать ночные реликвии"},
        {"Moon-silver fittings, old seals and a roll of survivor's coin rest in the bundle. Someone planned to buy safety and ran out of road first.", "В связке лежат лунно-серебряные крепления, старые печати и рулон монет выжившего. Кто-то хотел купить безопасность, но путь кончился раньше."},
        {"Rift trial", "Испытание разлома"},
        {"This is a compact combat room. Expect the shrine, jump timing and reward to chain together quickly.", "Это компактная боевая комната. Святилище, тайминги прыжков и награда быстро складываются в одну цепочку."},
        {"Trial keeper", "Хранитель испытания"},
        {"This compact arena is meant to test control under pressure. Win the first duel, then stabilize at the shrine.", "Эта компактная арена проверяет контроль под давлением. Победите первый поединок, затем закрепитесь у святилища."},
        {"Midnight Echo", "Полуночное эхо"},
        {"Enter to read midnight echo", "Enter - прочесть полуночное эхо"},
        {"The script warns that rain makes every misstep final. Save your strength for the long platforms and spend your gold before the storm spends you.", "Запись предупреждает: дождь делает каждый неверный шаг последним. Берегите силы для длинных платформ и тратьте золото раньше, чем буря потратит вас."},
        {"Abyssal Stash", "Бездонный тайник"},
        {"A rain-dark seam parts in the wall. Something hidden waited here for a careful return.", "Дождевой темный шов расходится в стене. Что-то скрытое ждало здесь осторожного возвращения."},
        {"Enter to recover abyssal stash", "Enter - забрать бездонный тайник"},
        {"Cold-metal trinkets and travel coin survive inside the sealed pouch. It feels like whoever hid it expected never to come back.", "В запечатанном мешочке уцелели холодные металлические безделушки и дорожные монеты. Похоже, спрятавший их не рассчитывал вернуться."},
        {"Veil ascent", "Подъем Вуали"},
        {"The returning veil starts with stacked jumps and narrow recovery windows. Save your momentum for the upper chain.", "Возвращающаяся вуаль начинается с цепочки прыжков и узких окон восстановления. Берегите импульс для верхней цепи."},
        {"Midnight span", "Полуночный пролет"},
        {"This rain-swept span punishes hesitation. Clear the lane, then commit to the crossing in one push.", "Этот залитый дождем пролет карает колебания. Зачистите линию и проходите переход одним решительным рывком."},
        {"Returning watcher", "Возвращающийся смотритель"},
        {"The veil's opening climb is guarded by a patient patrol. Clear it cleanly before the long midnight span.", "Первый подъем Вуали охраняет терпеливый патруль. Зачистите его чисто перед длинным полуночным пролетом."},
        {"Checkpoint attuned", "Точка возврата настроена"},
        {"Your return point for this realm has been refreshed.", "Ваша точка возврата для этой области обновлена."},
        {"Hidden path revealed", "Скрытый путь найден"},
        {"Something concealed answered your approach.", "Что-то скрытое откликнулось на ваше приближение."},
        {"Enter to interact", "Enter - взаимодействовать"},
        {"Forgotten Relic", "Забытая реликвия"},
        {"The dead left a trace here.", "Мертвые оставили здесь след."},
        {"Threat ahead", "Впереди угроза"},
        {"World event", "Событие мира"},
        {"Something shifts in this place.", "Что-то в этом месте меняется."},
        {"A hidden pocket of the realm opens beyond the veil.", "За вуалью открывается скрытый карман области."}
        ,{"Sunken Grotto", "Затонувший грот"},
        {"Enter to descend into grotto", "Enter - спуститься в грот"},
        {"Enter to climb back", "Enter - подняться обратно"},
        {"Enter to claim the flooded cache", "Enter - забрать затопленный тайник"},
        {"Flooded Reliquary", "Затопленный реликварий"},
        {"Coins and moon-bright tokens survived where the roots kept the chamber sealed. Something down here waited to be remembered.", "Монеты и лунно-светлые жетоны уцелели там, где корни держали комнату запечатанной. Что-то внизу ждало, чтобы о нем вспомнили."},
        {"Root Burrow", "Корневая нора"},
        {"Enter the root burrow", "Enter - войти в корневую нору"},
        {"Enter to return outside", "Enter - вернуться наружу"},
        {"Enter to read the root-etched tablet", "Enter - прочесть табличку на корнях"},
        {"Root-Etched Tablet", "Табличка на корнях"},
        {"The tunnel walls whisper of hunters who marked side-paths with buried stone. They hid warnings where only the curious would find them.", "Стены туннеля шепчут об охотниках, отмечавших боковые тропы погребенным камнем. Они прятали предупреждения там, где их нашли бы только любопытные."},
        {"Forgotten Reliquary", "Забытый реликварий"},
        {"Enter the sealed reliquary", "Enter - войти в запечатанный реликварий"},
        {"Enter to leave the reliquary", "Enter - покинуть реликварий"},
        {"Enter to attune reliquary flame", "Enter - настроить пламя реликвария"},
        {"Reliquary Flame", "Пламя реликвария"},
        {"A hidden ember still keeps watch beneath the stone. It can hold your return here and steady the breath you carry deeper underground.", "Скрытый уголек все еще сторожит под камнем. Он может удержать вашу точку возврата здесь и выровнять дыхание, с которым вы идете глубже под землю."}
    };

    const auto it = translations.find(text);
    return it != translations.end() ? it->second : text;
}

struct MiniLocationCandidate
{
    float centerX = 0.f;
    float surfaceY = 0.f;
    float leftX = 0.f;
    float rightX = 0.f;
};

struct MiniLocationTheme
{
    std::string title;
    std::string entranceTexture;
    std::string exitTexture;
    std::string backdropDecoration;
    std::string hillDecoration;
    std::string hangingDecoration;
    std::string ambientDecoration;
    std::string rewardTypeName;
    std::string rewardTexture;
    sf::Vector2f rewardScale{1.f, 1.f};
    sf::Color entranceColor = sf::Color::White;
    sf::Color exitColor = sf::Color::White;
    sf::Color accentColor = sf::Color::White;
    int goldReward = 0;
    bool grantsCheckpoint = false;
    bool restoreVitality = false;
    std::string entrancePrompt;
    std::string exitPrompt;
    std::string rewardPrompt;
    std::string rewardTitle;
    std::string rewardBody;
};

std::uint32_t hashNameplateSeed(const std::string& value)
{
    std::uint32_t hash = 2166136261u;
    for (const unsigned char symbol : value)
    {
        hash ^= symbol;
        hash *= 16777619u;
    }
    return hash;
}

std::string pickWorldNameplateTexture(
    const WorldInteractable::Type type,
    const std::string& title,
    const sf::Vector2f& position
)
{
    const std::string seed =
        std::to_string(static_cast<int>(type))
        + "|"
        + title
        + "|"
        + std::to_string(static_cast<int>(std::round(position.x)))
        + "|"
        + std::to_string(static_cast<int>(std::round(position.y)));

    const std::size_t textureIndex = hashNameplateSeed(seed) % kWorldNameplateTextures.size();
    return kWorldNameplateTextures[textureIndex];
}

void applyWorldNameplateVisual(WorldInteractable::Config& config)
{
    config.textureName = pickWorldNameplateTexture(config.type, config.title, config.position);
    config.scale = kWorldNameplateScale;
    config.color = sf::Color::White;
}

bool shouldUseAutomaticNameplate(const nlohmann::json& interactiveData, const std::string& typeName)
{
    if (interactiveData.contains("AutoNameplate"))
    {
        return interactiveData["AutoNameplate"].get<bool>();
    }

    return typeName != "CustomSign";
}

std::mt19937& miniLocationRng()
{
    static std::mt19937 rng(std::random_device{}());
    return rng;
}

float randomFloat(float minValue, float maxValue)
{
    std::uniform_real_distribution<float> dist(minValue, maxValue);
    return dist(miniLocationRng());
}

template <typename T>
const T& pickRandom(const std::vector<T>& values)
{
    std::uniform_int_distribution<std::size_t> dist(0u, values.size() - 1u);
    return values[dist(miniLocationRng())];
}

std::uint8_t clampToByte(float value)
{
    return static_cast<std::uint8_t>(std::clamp(value, 0.f, 255.f));
}

sf::Color withAlpha(const sf::Color& color, float alpha)
{
    return sf::Color(color.r, color.g, color.b, clampToByte(alpha));
}

sf::Color brighten(const sf::Color& color, float factor)
{
    const float clampedFactor = std::clamp(factor, 0.f, 1.f);
    const auto brightenChannel = [&](std::uint8_t channel) {
        return clampToByte(static_cast<float>(channel) + (255.f - static_cast<float>(channel)) * clampedFactor);
    };

    return sf::Color(
        brightenChannel(color.r),
        brightenChannel(color.g),
        brightenChannel(color.b),
        color.a
    );
}

float fract(float value)
{
    return value - std::floor(value);
}

sf::Vector2f readVector2f(const nlohmann::json& value, const sf::Vector2f fallback = {0.f, 0.f})
{
    if (!value.is_array() || value.size() < 2)
    {
        return fallback;
    }

    return {
        value[0].get<float>(),
        value[1].get<float>()
    };
}

sf::FloatRect readRect(const nlohmann::json& value, const sf::FloatRect fallback = sf::FloatRect({0.f, 0.f}, {0.f, 0.f}))
{
    if (!value.is_array() || value.size() < 4)
    {
        return fallback;
    }

    return sf::FloatRect(
        {value[0].get<float>(), value[1].get<float>()},
        {value[2].get<float>(), value[3].get<float>()}
    );
}

sf::Color readColor(const nlohmann::json& value, const sf::Color fallback = sf::Color::White)
{
    if (!value.is_array() || value.size() < 4)
    {
        return fallback;
    }

    return sf::Color{
        value[0].get<std::uint8_t>(),
        value[1].get<std::uint8_t>(),
        value[2].get<std::uint8_t>(),
        value[3].get<std::uint8_t>()
    };
}

nlohmann::json withOffsetPosition(nlohmann::json object, const sf::Vector2f offset)
{
    if (object.contains("Position"))
    {
        const sf::Vector2f position = readVector2f(object["Position"]);
        object["Position"] = nlohmann::json::array({position.x + offset.x, position.y + offset.y});
    }
    if (object.contains("ActivationArea") && object["ActivationArea"].is_array())
    {
        sf::FloatRect activationArea = readRect(object["ActivationArea"]);
        activationArea.position += offset;
        object["ActivationArea"] = nlohmann::json::array({
            activationArea.position.x,
            activationArea.position.y,
            activationArea.size.x,
            activationArea.size.y
        });
    }
    if (object.contains("Target") && object["Target"].is_object())
    {
        auto& target = object["Target"];
        const std::string targetType = target.value("Type", std::string{"Position"});
        if (targetType != "Level")
        {
            if (target.contains("Position"))
            {
                const sf::Vector2f position = readVector2f(target["Position"]);
                target["Position"] = nlohmann::json::array({position.x + offset.x, position.y + offset.y});
            }
            if (target.contains("SpawnPosition"))
            {
                const sf::Vector2f position = readVector2f(target["SpawnPosition"]);
                target["SpawnPosition"] = nlohmann::json::array({position.x + offset.x, position.y + offset.y});
            }
        }
    }

    return object;
}

sf::Vector2f removeParallaxPreview(const sf::Vector2f displayPosition, const sf::Vector2f parallaxFactor, const sf::Vector2f cameraCenter)
{
    const sf::Vector2f cameraOffset = cameraCenter - BASE_CAMERAPOS;
    return {
        displayPosition.x - cameraOffset.x * parallaxFactor.x,
        displayPosition.y - cameraOffset.y * parallaxFactor.y
    };
}

sf::Vector2f readMiniLocationPoint(const nlohmann::json& object, const char* key, const sf::FloatRect& bounds, const sf::Vector2f fallback)
{
    const sf::Vector2f point = readVector2f(object.value(key, nlohmann::json::array()), fallback);
    const bool looksRelative =
        point.x > -bounds.size.x &&
        point.y > -bounds.size.y &&
        point.x < bounds.size.x * 2.f &&
        point.y < bounds.size.y * 2.f;
    return looksRelative ? bounds.position + point : point;
}

sf::Vector2f miniLocationParallaxReferencePoint(const nlohmann::json& location, const sf::FloatRect& bounds)
{
    (void)location;
    return bounds.getCenter();
}

nlohmann::json withMiniLocationDecorationPosition(nlohmann::json object, const sf::Vector2f origin, const sf::FloatRect& bounds, const sf::Vector2f parallaxReferencePoint)
{
    if (!object.contains("Position"))
    {
        return object;
    }

    const sf::Vector2f relativeDisplayPosition = readVector2f(object["Position"]);
    const sf::Vector2f displayPosition = origin + relativeDisplayPosition;
    object["Position"] = nlohmann::json::array({
        displayPosition.x,
        displayPosition.y
    });
    object["MiniLocationParallaxReference"] = nlohmann::json::array({
        parallaxReferencePoint.x,
        parallaxReferencePoint.y
    });

    return object;
}

sf::FloatRect offsetRect(const sf::FloatRect& rect, const sf::Vector2f offset)
{
    return sf::FloatRect(rect.position + offset, rect.size);
}

void appendMiniLocationDeadAreas(nlohmann::json& target, const nlohmann::json& location, const sf::Vector2f origin)
{
    if (!target.contains("DeadAreas") || !target["DeadAreas"].is_array())
    {
        target["DeadAreas"] = nlohmann::json::array();
    }

    const std::string miniLocationId = location.value("Id", location.value("Title", std::string{}));
    if (location.contains("DeadAreas") && location["DeadAreas"].is_array())
    {
        for (auto deadArea : location["DeadAreas"])
        {
            const sf::FloatRect rect = offsetRect(readRect(deadArea.value("Rect", nlohmann::json::array())), origin);
            deadArea["Rect"] = nlohmann::json::array({rect.position.x, rect.position.y, rect.size.x, rect.size.y});
            if (!miniLocationId.empty())
            {
                deadArea["MiniLocationId"] = miniLocationId;
            }
            target["DeadAreas"].push_back(deadArea);
        }
    }

    if (location.contains("Hazard") && location["Hazard"].is_object())
    {
        auto deadArea = location["Hazard"];
        deadArea["Id"] = deadArea.value("Id", std::string{"legacy_hazard"});
        if (!miniLocationId.empty())
        {
            deadArea["MiniLocationId"] = miniLocationId;
        }
        target["DeadAreas"].push_back(deadArea);
    }
}

void appendNestedMiniLocationContent(nlohmann::json& target, const nlohmann::json& location, const char* key, const sf::Vector2f origin)
{
    if (!location.contains(key) || !location[key].is_array())
    {
        return;
    }

    if (!target.contains(key) || !target[key].is_array())
    {
        target[key] = nlohmann::json::array();
    }

    for (const auto& nestedObject : location[key])
    {
        if (std::string(key) == "Decorations")
        {
            const sf::FloatRect bounds = readRect(location.value("Bounds", nlohmann::json::array()));
            nlohmann::json object = withMiniLocationDecorationPosition(
                nestedObject,
                origin,
                bounds,
                miniLocationParallaxReferencePoint(location, bounds)
            );
            object["MiniLocationId"] = location.value("Id", location.value("Title", std::string{}));
            target[key].push_back(object);
        }
        else
        {
            target[key].push_back(withOffsetPosition(nestedObject, origin));
        }
    }
}

nlohmann::json expandMiniLocationContent(const nlohmann::json& source)
{
    nlohmann::json expanded = source;
    if (!expanded.contains("MiniLocations") || !expanded["MiniLocations"].is_array())
    {
        return expanded;
    }

    for (const auto& location : expanded["MiniLocations"])
    {
        const sf::FloatRect bounds = readRect(location.value("Bounds", nlohmann::json::array()));
        if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
        {
            continue;
        }

        const sf::Vector2f origin = bounds.position;
        appendNestedMiniLocationContent(expanded, location, "Platforms", origin);
        appendNestedMiniLocationContent(expanded, location, "Decorations", origin);
        appendNestedMiniLocationContent(expanded, location, "Interactives", origin);
        appendNestedMiniLocationContent(expanded, location, "Portals", origin);
        appendMiniLocationDeadAreas(expanded, location, origin);
    }

    return expanded;
}

WorldPortal::Target parseWorldPortalTarget(const nlohmann::json& portalData, const std::map<std::string, sf::Vector2f>& miniLocationEntrances)
{
    const nlohmann::json targetData = portalData.value("Target", nlohmann::json::object());
    const std::string targetType = targetData.value("Type", std::string{"Position"});

    WorldPortal::Target target;
    if (targetType == "Level")
    {
        target.type = WorldPortal::TargetType::Level;
        target.levelId = targetData.value("LevelId", std::string{});
        if (targetData.contains("SpawnPosition"))
        {
            target.spawnPosition = readVector2f(targetData["SpawnPosition"]);
        }
        return target;
    }

    if (targetType == "MiniLocation")
    {
        target.type = WorldPortal::TargetType::MiniLocation;
        target.miniLocationId = targetData.value("MiniLocationId", std::string{});
        if (targetData.contains("SpawnPosition"))
        {
            target.spawnPosition = readVector2f(targetData["SpawnPosition"]);
        }
        else if (const auto locationIt = miniLocationEntrances.find(target.miniLocationId); locationIt != miniLocationEntrances.end())
        {
            target.position = locationIt->second;
        }
        else if (!miniLocationEntrances.empty())
        {
            target.miniLocationId = miniLocationEntrances.begin()->first;
            target.position = miniLocationEntrances.begin()->second;
        }
        else
        {
            target.position = readVector2f(targetData.value("Position", portalData.value("Position", nlohmann::json::array())));
        }
        return target;
    }

    target.type = WorldPortal::TargetType::Position;
    target.position = readVector2f(targetData.value("Position", nlohmann::json::array()));
    return target;
}

NotificationTone parseNotificationTone(const std::string& tone)
{
    if (tone == "Success" || tone == "success")
    {
        return NotificationTone::Success;
    }
    if (tone == "Warning" || tone == "warning")
    {
        return NotificationTone::Warning;
    }
    return NotificationTone::Info;
}

std::string humanizeThemeName(const std::string& value)
{
    if (value.empty())
    {
        return {};
    }

    std::string result;
    result.reserve(value.size() + 6u);
    bool previousWasSeparator = true;
    for (const char symbol : value)
    {
        if (symbol == '_' || symbol == '-')
        {
            result.push_back(' ');
            previousWasSeparator = true;
            continue;
        }

        if (std::isupper(static_cast<unsigned char>(symbol)) && !previousWasSeparator)
        {
            result.push_back(' ');
        }

        result.push_back(symbol);
        previousWasSeparator = false;
    }

    if (!result.empty())
    {
        result.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(result.front())));
    }

    return result;
}

int readEditorDrawOrder(const nlohmann::json& object, const int fallback)
{
    if (object.contains("EditorDrawOrder") && object["EditorDrawOrder"].is_number_integer())
    {
        return object["EditorDrawOrder"].get<int>();
    }

    return fallback;
}

int readPresetDrawOrder(const nlohmann::json& data, const char* key, const int fallback)
{
    const nlohmann::json presets = data.value("Presets", nlohmann::json::object());
    if (presets.contains(key) && presets[key].is_number_integer())
    {
        return presets[key].get<int>();
    }

    return fallback;
}

float seededNoise(sf::Vector2f position, int saltA, float saltB)
{
    const float value = std::sin(
        position.x * 0.01373f
        + position.y * 0.00919f
        + static_cast<float>(saltA) * 0.07131f
        + saltB * 17.0f
    ) * 43758.5453f;

    return fract(value);
}

const std::vector<MiniLocationTheme>& getMiniLocationThemes()
{
    static const std::vector<MiniLocationTheme> themes{
        {
            "Sunken Grotto",
            "MossyDecorationHazard_25.png",
            "MossyDecorationHazard_24.png",
            "MossyBackgroundDecoration_09.png",
            "MossyHills_06.png",
            "MossyHangingPlants_06.png",
            "blueFlower1",
            "GoldCache",
            "Item_29.png",
            {3.8f, 3.8f},
            sf::Color(214, 246, 232, 255),
            sf::Color(212, 232, 255, 255),
            sf::Color(102, 202, 178, 255),
            115,
            false,
            false,
            "Enter to descend into grotto",
            "Enter to climb back",
            "Enter to claim the flooded cache",
            "Flooded Reliquary",
            "Coins and moon-bright tokens survived where the roots kept the chamber sealed. Something down here waited to be remembered."
        },
        {
            "Root Burrow",
            "MossyDecorationHazard_23.png",
            "MossyDecorationHazard_25.png",
            "MossyBackgroundDecoration_07.png",
            "MossyHills_05.png",
            "MossyHangingPlants_03.png",
            "windPlant1",
            "EchoTablet",
            "MossyDecorationHazard_11.png",
            {0.28f, 0.28f},
            sf::Color(222, 246, 218, 255),
            sf::Color(236, 250, 224, 255),
            sf::Color(136, 210, 112, 255),
            65,
            false,
            false,
            "Enter the root burrow",
            "Enter to return outside",
            "Enter to read the root-etched tablet",
            "Root-Etched Tablet",
            "The tunnel walls whisper of hunters who marked side-paths with buried stone. They hid warnings where only the curious would find them."
        },
        {
            "Forgotten Reliquary",
            "MossyDecorationHazard_21.png",
            "MossyDecorationHazard_24.png",
            "MossyBackgroundDecoration_08.png",
            "MossyHills_04.png",
            "MossyHangingPlants_07.png",
            "blueFlower2",
            "RestShrine",
            "MossyDecorationHazard_21.png",
            {0.29f, 0.29f},
            sf::Color(248, 236, 220, 255),
            sf::Color(255, 228, 196, 255),
            sf::Color(224, 176, 108, 255),
            0,
            true,
            true,
            "Enter the sealed reliquary",
            "Enter to leave the reliquary",
            "Enter to attune reliquary flame",
            "Reliquary Flame",
            "A hidden ember still keeps watch beneath the stone. It can hold your return here and steady the breath you carry deeper underground."
        }
    };

    return themes;
}

bool usesAbyssPlatformTheme(const std::string& levelName)
{
    return levelName == "level4.json" || levelName == "level6.json";
}

std::string remapPlatformTypeForLevel(const std::string& levelName, const std::string& originalType)
{
    if (!usesAbyssPlatformTheme(levelName))
    {
        return originalType;
    }

    if (originalType == "Thorn-ramp")
    {
        return "Abyss-Thorn-ramp";
    }
    if (originalType == "Runed-ledge")
    {
        return "Abyss-Runed-ledge";
    }
    if (originalType == "Bone-dais")
    {
        return "Abyss-Bone-dais";
    }
    if (originalType == "Obsidian-span")
    {
        return "Abyss-Obsidian-span";
    }
    if (originalType == "Cathedral-span")
    {
        return "Abyss-Cathedral-span";
    }
    if (originalType == "Ritual-bridge")
    {
        return "Abyss-Ritual-bridge";
    }
    if (originalType == "Fallen-arcade")
    {
        return "Abyss-Fallen-arcade";
    }
    if (originalType == "Crypt-pillar")
    {
        return "Abyss-Crypt-pillar";
    }

    return originalType;
}

bool hasEntranceClearance(
    const MiniLocationCandidate& candidate,
    const std::vector<std::shared_ptr<sf::RectangleShape>>& platformRects
)
{
    constexpr float kRequiredHalfWidth = 96.f;
    constexpr float kMaxCeilingGap = 210.f;

    for (const auto& platformRect : platformRects)
    {
        const sf::FloatRect bounds = platformRect->getGlobalBounds();
        const float platformBottom = bounds.position.y + bounds.size.y;
        if (platformBottom >= candidate.surfaceY - 4.f)
        {
            continue;
        }

        const float verticalGap = candidate.surfaceY - platformBottom;
        if (verticalGap > kMaxCeilingGap)
        {
            continue;
        }

        if (candidate.centerX + kRequiredHalfWidth < bounds.position.x
            || candidate.centerX - kRequiredHalfWidth > bounds.position.x + bounds.size.x)
        {
            continue;
        }

        return false;
    }

    return true;
}

void addMiniLocationSeal(Platform& platforms, const float x, const float topY, const float bottomY)
{
    const float startY = std::min(topY, bottomY);
    const float endY = std::max(topY, bottomY);

    for (float currentY = startY; currentY <= endY; currentY += 170.f)
    {
        platforms.addPlatform({x, currentY}, "Invisible-wall");
    }
}

void addBarrierRectCollision(Platform& platforms, const sf::FloatRect& rect)
{
    const float left = rect.position.x;
    const float right = rect.position.x + std::max(rect.size.x, 1.f);
    constexpr float stepX = 28.f;
    for (float x = left; x <= right; x += stepX)
    {
        addMiniLocationSeal(platforms, x, rect.position.y, rect.position.y + rect.size.y);
    }
    addMiniLocationSeal(platforms, right, rect.position.y, rect.position.y + rect.size.y);
}
}

void GameLevelManager::initializeLevels(const std::string& levelsFolder)
{
    levels.clear();
    levelRegistry_.scan(levelsFolder);

    for (const LevelDescriptor& descriptor : levelRegistry_.getLevels())
    {
        levels.emplace(
            descriptor.id,
            std::make_shared<GameLevel>(
                *data,
                *camera,
                *this,
                *window,
                descriptor
            )
        );
    }
}

GameLevelManager::GameLevelManager(GameData& d, GameCamera& c, sf::RenderWindow& w, const std::string& lF)
    : data(&d)
    , camera(&c)
    , window(&w)
    , levelsFolder(lF)
{
    initializeLevels(levelsFolder);

    if (std::optional<LevelDescriptor> firstCampaignLevel = levelRegistry_.resolve("level1.json"); firstCampaignLevel.has_value())
    {
        levelIt = levels.find(firstCampaignLevel->id);
    }
    else
    {
        levelIt = levels.end();
    }

    if (levelIt == levels.end())
    {
        levelIt = levels.begin();
    }
}

GameLevelManager::~GameLevelManager() = default;

void GameLevelManager::setNotificationSink(std::function<void(std::string, std::string, NotificationTone)> sink)
{
    notificationSink_ = sink ? std::move(sink) : [](std::string, std::string, NotificationTone) {};
}

void GameLevelManager::pushNotification(std::string title, std::string body, const NotificationTone tone) const
{
    notificationSink_(std::move(title), std::move(body), tone);
}

void GameLevelManager::setPlayerPositionToBase()
{
    if (!player || levels.empty() || levelIt == levels.end())
    {
        return;
    }

    player->setPosition(levelIt->second->getPlayerSpawnPos());
}

bool GameLevelManager::goToLevel(std::optional<std::string> levelName, const bool ignoreUnlocks)
{
    if (levels.empty() || !levelName.has_value())
    {
        return false;
    }

    const std::optional<LevelDescriptor> resolvedLevel = levelRegistry_.resolve(*levelName);
    if (!resolvedLevel.has_value())
    {
        return false;
    }

    auto nextLevelIt = levels.find(resolvedLevel->id);
    if (nextLevelIt == levels.end())
    {
        return false;
    }

    if (!ignoreUnlocks && player && !player->isLevelUnlocked(resolvedLevel->id))
    {
        return false;
    }

    if (levelIt != levels.end())
    {
        levelIt->second->saveLevelData();
        levelIt->second->exitMiniLocation();
        levelIt->second->clearLevel();
    }

    levelIt = nextLevelIt;
    levelIt->second->loadLevelData(*resolvedLevel);

    if (player)
    {
        const sf::Vector2f spawnPos = levelIt->second->getPlayerSpawnPos();
        player->setPosition(spawnPos);
        player->notifyLevelEntered(levelIt->second->levelName);
        camera->setCenterPosition(spawnPos);
        levelIt->second->onPlayerEnteredLevel();
    }

    return true;
}

bool GameLevelManager::restartCurrentLevel()
{
    if (levels.empty() || levelIt == levels.end())
    {
        return false;
    }

    const LevelDescriptor currentLevel = levelIt->second->getLevelDescriptor();
    levelIt->second->clearLevel();
    levelIt->second->loadLevelData(currentLevel);

    if (player)
    {
        const sf::Vector2f spawnPos = levelIt->second->getPlayerSpawnPos();
        player->respawnAt(spawnPos);
        player->notifyLevelEntered(levelIt->second->levelName);
        camera->setCenterPosition(spawnPos);
        levelIt->second->onPlayerEnteredLevel();
    }

    return true;
}

bool GameLevelManager::respawnPlayerAtCurrentSpawn()
{
    if (levels.empty() || levelIt == levels.end() || !player)
    {
        return false;
    }

    const sf::Vector2f spawnPos = levelIt->second->getPlayerSpawnPos();
    levelIt->second->exitMiniLocation();
    player->respawnAt(spawnPos);
    camera->setCenterPosition(spawnPos);
    return true;
}

void GameLevelManager::update()
{
    if (levels.empty() || levelIt == levels.end())
    {
        return;
    }

    levelIt->second->update();
}

void GameLevelManager::updatePlatforms()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updatePlatforms();
    }
}

void GameLevelManager::updateDecorations()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updateDecorations();
    }
}

void GameLevelManager::updateBackgrounds()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updateBackgrounds();
    }
}

void GameLevelManager::updateGrounds()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updateGrounds();
    }
}

void GameLevelManager::updateEnemyManager()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updateEnemyManager();
    }

    updateDeathRecoveries();
}

void GameLevelManager::updateInteractives()
{
    if (levelIt != levels.end())
    {
        levelIt->second->updateInteractives();
    }
}

void GameLevelManager::draw()
{
    if (levels.empty() || levelIt == levels.end())
    {
        return;
    }

    levelIt->second->draw();
    drawDeathRecoveries();
}

void GameLevelManager::drawPlatforms()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawPlatforms();
    }
}

void GameLevelManager::drawDecorations()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawDecorations();
    }
}

void GameLevelManager::drawBackgrounds()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawBackgrounds();
    }
}

void GameLevelManager::drawGrounds()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawGrounds();
    }
}

void GameLevelManager::drawEnemyManager()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawEnemyManager();
    }

    drawDeathRecoveries();
}

void GameLevelManager::drawInteractives()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawInteractives();
    }
}

void GameLevelManager::drawInteractiveOverlays()
{
    if (levelIt != levels.end())
    {
        levelIt->second->drawInteractiveOverlays();
    }
}

sf::Vector2i GameLevelManager::getCurrentLevelSize() const
{
    return levelIt != levels.end() ? levelIt->second->getLevelSize() : sf::Vector2i{};
}

sf::FloatRect GameLevelManager::getCurrentCameraBoundsForPosition(const sf::Vector2f& position) const
{
    if (levelIt == levels.end() || !levelIt->second)
    {
        return sf::FloatRect({0.f, 0.f}, sf::Vector2f{0.f, 0.f});
    }

    return levelIt->second->getCameraBoundsForPosition(position);
}

bool GameLevelManager::isCurrentMiniLocationActive() const
{
    return levelIt != levels.end() && levelIt->second && levelIt->second->isMiniLocationActive();
}

bool GameLevelManager::enterCurrentMiniLocation(const std::string& id, std::optional<sf::Vector2f> returnSupportPoint)
{
    return levelIt != levels.end() && levelIt->second && levelIt->second->enterMiniLocation(id, returnSupportPoint);
}

void GameLevelManager::exitCurrentMiniLocation()
{
    if (levelIt != levels.end() && levelIt->second)
    {
        levelIt->second->exitMiniLocation();
    }
}

sf::Vector2f GameLevelManager::exitCurrentMiniLocation(const sf::Vector2f& fallbackReturnSupportPoint)
{
    if (levelIt != levels.end() && levelIt->second)
    {
        return levelIt->second->exitMiniLocation(fallbackReturnSupportPoint);
    }

    return fallbackReturnSupportPoint;
}

std::string GameLevelManager::getCurrentLevelName() const
{
    return levelIt != levels.end() ? levelIt->second->levelName : std::string{};
}

std::string GameLevelManager::getCurrentLevelTitle() const
{
    return levelIt != levels.end() ? levelIt->second->levelTitle : std::string{};
}

std::vector<std::string> GameLevelManager::getLevelNames() const
{
    std::vector<std::string> levelNames;
    levelNames.reserve(levelRegistry_.getLevels().size());

    for (const LevelDescriptor& descriptor : levelRegistry_.getLevels())
    {
        if (levels.find(descriptor.id) != levels.end())
        {
            levelNames.push_back(descriptor.id);
        }
    }

    return levelNames;
}

std::map<std::string, std::string> GameLevelManager::getLevelDisplayNames() const
{
    std::map<std::string, std::string> displayNames;
    for (const auto& [levelId, level] : levels)
    {
        displayNames[levelId] = level ? level->levelTitle : levelId;
    }

    return displayNames;
}

std::vector<std::shared_ptr<sf::RectangleShape>>& GameLevelManager::getPlatformRects()
{
    if (levelIt == levels.end() || !levelIt->second)
    {
        throw std::runtime_error("Current level does not exist");
    }

    return levelIt->second->getPlatformRects();
}

Platform& GameLevelManager::getCurrentPlatformSystem()
{
    if (levelIt == levels.end() || !levelIt->second)
    {
        throw std::runtime_error("Current level does not exist");
    }

    return levelIt->second->getPlatformSystem();
}

sf::RectangleShape& GameLevelManager::getGroundRect()
{
    return levelIt->second->getGroundRect();
}

const std::map<std::string, std::shared_ptr<GameLevel>>& GameLevelManager::getLevelsMap() const
{
    return levels;
}

std::map<std::string, std::shared_ptr<GameLevel>>::iterator& GameLevelManager::getIteratorReference()
{
    return levelIt;
}

const LevelRegistry& GameLevelManager::getLevelRegistry() const
{
    return levelRegistry_;
}

std::optional<LevelDescriptor> GameLevelManager::resolveLevelIdentifier(const std::string& levelIdentifier) const
{
    return levelRegistry_.resolve(levelIdentifier);
}

std::string GameLevelManager::getLevelDisplayName(const std::string& levelIdentifier) const
{
    return levelRegistry_.getDisplayName(levelIdentifier);
}

bool GameLevelManager::hasBlockingInteractiveModal() const
{
    return levelIt != levels.end() && levelIt->second && levelIt->second->hasBlockingInteractiveModal();
}

void GameLevelManager::setCurrentLevelSpawn(const sf::Vector2f& pos)
{
    if (levelIt == levels.end() || !levelIt->second)
    {
        return;
    }

    levelIt->second->setPlayerSpawnPos(pos);
}

sf::Vector2f GameLevelManager::getCurrentTraderPosition() const
{
    return levelIt != levels.end() && levelIt->second
        ? levelIt->second->getTraderPosition()
        : sf::Vector2f{800.f, 940.f};
}

bool GameLevelManager::teleportPlayerToCurrentLevelPosition(const sf::Vector2f& pos)
{
    if (!player || levelIt == levels.end() || !levelIt->second)
    {
        return false;
    }

    player->setPosition(pos);
    camera->setCenterPosition(pos);
    return true;
}

bool GameLevelManager::teleportPlayerToCurrentMiniLocationPosition(
    const std::string& miniLocationId,
    const sf::Vector2f& pos,
    std::optional<sf::Vector2f> returnSupportPoint)
{
    if (!enterCurrentMiniLocation(miniLocationId, returnSupportPoint))
    {
        return false;
    }

    if (!player || !camera)
    {
        return false;
    }

    player->teleportToSupportPoint(pos);
    camera->setCenterPosition(pos);
    return true;
}

bool GameLevelManager::teleportPlayerToLevelPosition(const std::string& levelName, const sf::Vector2f& pos)
{
    if (!goToLevel(levelName, true))
    {
        return false;
    }

    exitCurrentMiniLocation();
    return teleportPlayerToCurrentLevelPosition(pos);
}

bool GameLevelManager::teleportPlayerToLevelSpawn(const std::string& levelName)
{
    return goToLevel(levelName, true);
}

void GameLevelManager::attachPlayer(Player& p)
{
    player = &p;

    for (auto& [_, level] : levels)
    {
        level->attachPlayer(p);
    }

    if (levelIt != levels.end())
    {
        player->notifyLevelEntered(levelIt->second->levelName);
        levelIt->second->onPlayerEnteredLevel();
    }
}

void GameLevelManager::clearDeathRecoveries()
{
    deathRecoveries.clear();
}

void GameLevelManager::registerDeathRecovery(const sf::Vector2f& position, int goldAmount)
{
    if (!data || goldAmount <= 0 || levelIt == levels.end())
    {
        return;
    }

    deathRecoveries.clear();
    deathRecoveries.push_back(std::make_unique<DeathRecovery>(
        *data,
        levelIt->second->levelName,
        findDeathRecoveryAnchor(position),
        goldAmount
    ));
}

bool GameLevelManager::handleEvent(const sf::Event& event)
{
    if (!player || levelIt == levels.end())
    {
        return false;
    }

    for (auto& recovery : deathRecoveries)
    {
        if (recovery->belongsToLevel(levelIt->second->levelName) && recovery->handleEvent(event, *player))
        {
            return true;
        }
    }

    if (levelIt->second->handleEvent(event))
    {
        return true;
    }

    deathRecoveries.erase(
        std::remove_if(deathRecoveries.begin(), deathRecoveries.end(),
            [](const std::unique_ptr<DeathRecovery>& recovery) {
                return recovery->isRecovered();
            }),
        deathRecoveries.end()
    );

    return false;
}

void GameLevelManager::updateDeathRecoveries()
{
    if (!player || levelIt == levels.end())
    {
        return;
    }

    for (auto& recovery : deathRecoveries)
    {
        if (recovery->belongsToLevel(levelIt->second->levelName))
        {
            recovery->update(*player);
        }
    }

    deathRecoveries.erase(
        std::remove_if(deathRecoveries.begin(), deathRecoveries.end(),
            [](const std::unique_ptr<DeathRecovery>& recovery) {
                return recovery->isRecovered();
            }),
        deathRecoveries.end()
    );
}

void GameLevelManager::drawDeathRecoveries()
{
    if (levelIt == levels.end())
    {
        return;
    }

    for (auto& recovery : deathRecoveries)
    {
        if (recovery->belongsToLevel(levelIt->second->levelName))
        {
            recovery->draw(*window);
        }
    }
}

sf::Vector2f GameLevelManager::findDeathRecoveryAnchor(const sf::Vector2f& position)
{
    const float levelWidth = static_cast<float>(getCurrentLevelSize().x);
    const float clampedX = std::clamp(position.x, 18.f, std::max(18.f, levelWidth - 18.f));

    float supportY = getGroundRect().getPosition().y;
    for (const auto& platformRect : getPlatformRects())
    {
        const sf::FloatRect bounds = platformRect->getGlobalBounds();
        if (clampedX < bounds.position.x - 6.f || clampedX > bounds.position.x + bounds.size.x + 6.f)
        {
            continue;
        }

        if (bounds.position.y + 8.f < position.y)
        {
            continue;
        }

        supportY = std::min(supportY, bounds.position.y);
    }

    return {clampedX, supportY - 6.f};
}

GameLevel::GameLevel(GameData& d, GameCamera& c, GameLevelManager& m, sf::RenderWindow& w, const LevelDescriptor& descriptor)
    : data(&d)
    , camera(&c)
    , levelManager(&m)
    , window(&w)
{
    loadLevelData(descriptor);
}

GameLevel::~GameLevel() = default;

void GameLevel::updatePlatforms()
{
    if (platforms)
    {
        platforms->update();
    }
}

void GameLevel::updateDecorations()
{
    if (decorations)
    {
        decorations->setActiveMiniLocation(activeMiniLocationId_);
        decorations->updateTextures();
    }
}

void GameLevel::updateBackgrounds()
{
    for (auto&& i : background)
    {
        i->update();
    }
}

void GameLevel::updateGrounds()
{
}

void GameLevel::updateEnemyManager()
{
    if (!enemyManager)
    {
        return;
    }

    enemyManager->updateAI_all();
    enemyManager->updateControls_all();
    enemyManager->updatePhysics_all();
    enemyManager->updateSpawners_all();
    enemyManager->updateTextures_all();
}

void GameLevel::updateInteractives()
{
    for (auto& interactive : interactives)
    {
        interactive->update();
    }
}

void GameLevel::update()
{
    updateDecorations();
    updateBackgrounds();
    updateGrounds();
    updateInteractives();
    updateMiniLocationHazards();
    updateLevelEvents();
}

void GameLevel::drawPlatforms()
{
    if (platforms)
    {
        platforms->draw(*window);
    }

    drawMiniLocationBarriers();
}

void GameLevel::drawDecorations()
{
    if (decorations)
    {
        decorations->draw(*window);
    }
}

void GameLevel::drawBackgrounds()
{
    for (auto&& i : background)
    {
        i->draw(*window);
    }
}

void GameLevel::drawGrounds()
{
    if (ground)
    {
        ground->draw(*window);
    }

    drawMiniLocationHazards();
}

void GameLevel::draw()
{
    if (useSharedWorldDrawOrder_)
    {
        for (const SharedDrawEntry& entry : sharedWorldDrawOrder_)
        {
            drawSharedWorldEntry(entry);
        }
        if (platforms)
        {
            platforms->drawAtmosphere(*window);
            drawMiniLocationBarriers();
        }
        return;
    }

    drawBackgrounds();
    drawDecorations();
    drawGrounds();
    drawActors();
    drawInteractives();
    drawPlatforms();
}

void GameLevel::drawActors()
{
    drawEnemyManager();

    if (player)
    {
        player->draw(*window);
        player->drawBullets(*window);
    }
}

void GameLevel::drawSharedWorldEntry(const SharedDrawEntry& entry)
{
    switch (entry.kind)
    {
        case SharedDrawEntry::Kind::Background:
            if (entry.index < background.size() && background[entry.index])
            {
                background[entry.index]->draw(*window);
            }
            break;

        case SharedDrawEntry::Kind::Decoration:
            if (decorations)
            {
                decorations->drawInstance(*window, entry.index);
            }
            break;

        case SharedDrawEntry::Kind::Ground:
            drawGrounds();
            break;

        case SharedDrawEntry::Kind::Actor:
            drawActors();
            break;

        case SharedDrawEntry::Kind::Interactive:
            if (entry.index < interactives.size() && interactives[entry.index])
            {
                interactives[entry.index]->draw(*window);
            }
            break;

        case SharedDrawEntry::Kind::Platform:
            if (platforms)
            {
                platforms->drawInstance(*window, entry.index);
            }
            break;
    }
}

void GameLevel::drawEnemyManager()
{
    if (enemyManager)
    {
        enemyManager->draw_all();
    }
}

void GameLevel::drawInteractives()
{
    for (auto& interactive : interactives)
    {
        interactive->draw(*window);
    }
}

void GameLevel::drawInteractiveOverlays()
{
    for (auto& interactive : interactives)
    {
        interactive->drawOverlay(*window);
    }
}

const GameLevel::GeneratedMiniLocation* GameLevel::activeMiniLocation() const
{
    if (!activeMiniLocationId_.has_value())
    {
        return nullptr;
    }

    for (const auto& generatedLocation : generatedMiniLocations)
    {
        if (generatedLocation.id == *activeMiniLocationId_ || generatedLocation.title == *activeMiniLocationId_)
        {
            return &generatedLocation;
        }
    }

    return nullptr;
}

void GameLevel::updateMiniLocationHazards()
{
    const GeneratedMiniLocation* location = activeMiniLocation();

    const auto hazardApplies = [location](const GeneratedMiniHazard& hazard) {
        if (!hazard.miniLocationId.empty())
        {
            return location != nullptr &&
                (hazard.miniLocationId == location->id || hazard.miniLocationId == location->title);
        }
        return location == nullptr;
    };

    if (enemyManager)
    {
        for (const auto& hazard : generatedMiniHazards)
        {
            if (!hazardApplies(hazard))
            {
                continue;
            }

            enemyManager->killEnemiesInRect({
                {hazard.leftX, hazard.topY},
                {std::max(0.f, hazard.rightX - hazard.leftX), std::max(0.f, hazard.bottomY - hazard.topY)}
            });
        }
    }

    if (!player || !player->isAlive || player->isPlayingDieAnimation)
    {
        return;
    }

    const sf::Vector2f feet = player->getFeetPosition();

    for (const auto& hazard : generatedMiniHazards)
    {
        if (!hazardApplies(hazard))
        {
            continue;
        }

        if (feet.x >= hazard.leftX && feet.x <= hazard.rightX &&
            feet.y >= hazard.topY && feet.y <= hazard.bottomY)
        {
            player->forceKill();
            return;
        }
    }

    if (location != nullptr &&
        feet.x >= location->activeLeftX && feet.x <= location->activeRightX &&
        feet.y >= location->deathY)
    {
        player->forceKill();
        return;
    }
}

void GameLevel::drawMiniLocationBarriers()
{
    if (!window)
    {
        return;
    }

    const float time = miniLocationEffectsClock.getElapsedTime().asSeconds();

    for (const auto& barrier : generatedMiniBarriers)
    {
        const float height = barrier.bottomY - barrier.topY;
        if (height <= 0.f)
        {
            continue;
        }

        const float pulse = 0.5f + 0.5f * std::sin(time * 3.1f + barrier.phase);

        sf::RectangleShape outerGlow({barrier.width * 4.8f, height});
        outerGlow.setOrigin({outerGlow.getSize().x / 2.f, 0.f});
        outerGlow.setPosition({barrier.x, barrier.topY});
        outerGlow.setFillColor(withAlpha(barrier.glowColor, 34.f + 34.f * pulse));
        window->draw(outerGlow);

        sf::RectangleShape innerGlow({barrier.width * 2.4f, height});
        innerGlow.setOrigin({innerGlow.getSize().x / 2.f, 0.f});
        innerGlow.setPosition({barrier.x, barrier.topY});
        innerGlow.setFillColor(withAlpha(brighten(barrier.coreColor, 0.25f), 82.f + 48.f * pulse));
        window->draw(innerGlow);

        sf::RectangleShape core({barrier.width, height});
        core.setOrigin({core.getSize().x / 2.f, 0.f});
        core.setPosition({barrier.x, barrier.topY});
        core.setFillColor(withAlpha(brighten(barrier.coreColor, 0.42f), 176.f + 48.f * pulse));
        window->draw(core);

        for (int segmentIndex = 0; segmentIndex < 10; ++segmentIndex)
        {
            const float ratio = static_cast<float>(segmentIndex) / 9.f;
            const float y = barrier.topY
                + ratio * height
                + std::sin(time * 4.6f + barrier.phase + ratio * 9.f) * 9.f;
            const float segmentWidth = barrier.width * (
                1.18f + 0.32f * std::sin(time * 6.4f + barrier.phase * 0.7f + static_cast<float>(segmentIndex))
            );

            sf::RectangleShape segment({segmentWidth, 4.f});
            segment.setOrigin({segmentWidth / 2.f, 2.f});
            segment.setPosition({
                barrier.x + std::sin(time * 2.7f + barrier.phase + ratio * 11.f) * 7.f,
                y
            });
            segment.setFillColor(withAlpha(brighten(barrier.coreColor, 0.55f), 210.f));
            window->draw(segment);
        }

        for (const float side : std::array<float, 2>{-1.f, 1.f})
        {
            for (int sparkIndex = 0; sparkIndex < 6; ++sparkIndex)
            {
                const float ratio = static_cast<float>(sparkIndex) / 5.f;
                const float y = barrier.topY
                    + ratio * height
                    + std::sin(time * 2.2f + barrier.phase + side * 0.7f + ratio * 10.f) * 13.f;
                const float radius = 2.6f + 1.7f * (0.5f + 0.5f * std::sin(time * 5.8f + barrier.phase + sparkIndex));

                sf::CircleShape spark(radius);
                spark.setOrigin({radius, radius});
                spark.setPosition({
                    barrier.x + side * (barrier.width * 0.9f + 8.f + 5.f * std::sin(time * 3.4f + ratio * 9.f)),
                    y
                });
                spark.setFillColor(withAlpha(brighten(barrier.glowColor, 0.38f), 180.f));
                window->draw(spark);
            }
        }
    }
}

void GameLevel::drawMiniLocationHazards()
{
    if (!window)
    {
        return;
    }

    const float time = miniLocationEffectsClock.getElapsedTime().asSeconds();

    for (const auto& hazard : generatedMiniHazards)
    {
        const float width = hazard.rightX - hazard.leftX;
        const float height = hazard.bottomY - hazard.topY;
        if (width <= 0.f || height <= 0.f)
        {
            continue;
        }

        const float pulse = 0.5f + 0.5f * std::sin(time * 2.6f + hazard.phase);

        sf::RectangleShape glow({width + 96.f, height + 40.f});
        glow.setPosition({hazard.leftX - 48.f, hazard.topY - 14.f});
        glow.setFillColor(withAlpha(hazard.glowColor, 30.f + 24.f * pulse));
        window->draw(glow);

        sf::RectangleShape abyss({width, height});
        abyss.setPosition({hazard.leftX, hazard.topY});
        abyss.setFillColor(sf::Color(34, 7, 5, 220));
        window->draw(abyss);

        sf::RectangleShape moltenBand({width, std::min(44.f, height * 0.32f)});
        moltenBand.setPosition({hazard.leftX, hazard.topY});
        moltenBand.setFillColor(withAlpha(hazard.coreColor, 132.f + 38.f * pulse));
        window->draw(moltenBand);

        for (int bubbleIndex = 0; bubbleIndex < 16; ++bubbleIndex)
        {
            const float ratio = static_cast<float>(bubbleIndex) / 15.f;
            const float x = hazard.leftX
                + ratio * width
                + std::sin(time * 2.0f + hazard.phase + ratio * 8.4f) * 13.f;
            const float y = hazard.topY + 6.f + std::sin(time * 5.1f + hazard.phase + ratio * 10.2f) * 5.f;
            const float radius = 8.f + 5.f * (0.5f + 0.5f * std::sin(time * 6.0f + hazard.phase + bubbleIndex));

            sf::CircleShape bubble(radius);
            bubble.setOrigin({radius, radius});
            bubble.setPosition({x, y});
            bubble.setFillColor(withAlpha(brighten(hazard.coreColor, 0.35f), 112.f + 62.f * pulse));
            window->draw(bubble);
        }

        for (int emberIndex = 0; emberIndex < 12; ++emberIndex)
        {
            const float laneNoise = seededNoise(
                {hazard.leftX + static_cast<float>(emberIndex) * 17.f, hazard.topY},
                emberIndex,
                hazard.phase + 0.31f
            );
            const float horizontalDrift = std::sin(time * (1.3f + emberIndex * 0.08f) + hazard.phase + emberIndex) * 18.f;
            const float riseDistance = std::fmod(
                time * (76.f + emberIndex * 7.f) + hazard.phase * 48.f + static_cast<float>(emberIndex * 31),
                height + 88.f
            );
            const float x = hazard.leftX + laneNoise * width + horizontalDrift;
            const float y = hazard.bottomY - riseDistance;
            const float radius = 2.2f + 3.6f * seededNoise(
                {hazard.rightX, hazard.bottomY + static_cast<float>(emberIndex) * 9.f},
                emberIndex + 7,
                hazard.phase + 0.63f
            );

            sf::CircleShape ember(radius);
            ember.setOrigin({radius, radius});
            ember.setPosition({x, y});
            ember.setFillColor(withAlpha(hazard.emberColor, 96.f + 84.f * pulse));
            window->draw(ember);
        }
    }
}

void GameLevel::initializePlatforms(const nlohmann::json& data)
{
    platforms = std::make_shared<Platform>();

    for (const auto& platform : data["Platforms"])
    {
        const sf::Vector2f position = {
            platform["Position"][0],
            platform["Position"][1]
        };

        const std::string type = platform["Type"];
        Platform::InstanceOverrides overrides;
        if (platform.contains("HitboxSize"))
        {
            const sf::Vector2f hitboxSize = readVector2f(platform["HitboxSize"]);
            overrides.hitboxSize = {
                std::max(hitboxSize.x, 8.f),
                std::max(hitboxSize.y, 8.f)
            };
        }
        if (platform.contains("HitboxOffset"))
        {
            overrides.hitboxOffset = readVector2f(platform["HitboxOffset"]);
        }
        if (platform.contains("Scale"))
        {
            const sf::Vector2f scale = readVector2f(platform["Scale"], {1.f, 1.f});
            overrides.spriteScale = {
                std::max(scale.x, 0.02f),
                std::max(scale.y, 0.02f)
            };
        }
        if (platform.contains("AtmosphereStyle"))
        {
            if (const auto atmosphereStyle = parsePlatformAtmosphereStyle(platform["AtmosphereStyle"].get<std::string>()); atmosphereStyle.has_value())
            {
                overrides.atmosphereStyle = *atmosphereStyle;
            }
        }
        if (platform.contains("AtmosphereColor"))
        {
            const auto& color = platform["AtmosphereColor"];
            overrides.atmosphereColor = sf::Color{
                color[0].get<std::uint8_t>(),
                color[1].get<std::uint8_t>(),
                color[2].get<std::uint8_t>(),
                color[3].get<std::uint8_t>()
            };
        }
        if (platform.contains("AtmosphereDensity"))
        {
            overrides.atmosphereDensity = std::max(platform["AtmosphereDensity"].get<float>(), 0.f);
        }
        if (platform.contains("BounceEnabled"))
        {
            overrides.bounceEnabled = platform.value("BounceEnabled", true);
        }

        platforms->addPlatform(position, remapPlatformTypeForLevel(levelName, type), overrides);
    }
}

void GameLevel::initializeDecorations(const nlohmann::json& data)
{
    decorations = std::make_shared<Decoration>(*this->data, *this->camera);

    for (const auto& decoration : data["Decorations"])
    {
        const std::string name = decoration["Name"];
        const sf::Vector2f position = {decoration["Position"][0], decoration["Position"][1]};
        const sf::Vector2f scale = {decoration["Scale"][0], decoration["Scale"][1]};
        const sf::Color color = sf::Color{
            decoration["Color"][0],
            decoration["Color"][1],
            decoration["Color"][2],
            decoration["Color"][3]
        };
        const sf::Vector2f parallaxFactor = {decoration["ParallaxFactor"][0], decoration["ParallaxFactor"][1]};
        const int zDepth = decoration.value("Z", 0);
        const float rotation = decoration.value("Rotation", 0.f);
        std::optional<DecorationMiniLocationContext> miniLocationContext;
        const std::string miniLocationId = decoration.value("MiniLocationId", std::string{});
        if (!miniLocationId.empty())
        {
            miniLocationContext = DecorationMiniLocationContext{
                miniLocationId,
                position,
                readVector2f(
                    decoration.value("MiniLocationParallaxReference", nlohmann::json::array()),
                    position
                )
            };
        }

        decorations->addDecoration(name, position, scale, parallaxFactor, zDepth, color, rotation, miniLocationContext);
    }
}

void GameLevel::initializeBackground(const nlohmann::json& data)
{
    try
    {
        if (!data.contains("Background") || !data["Background"].is_array())
        {
            background.clear();
            return;
        }

        const std::string backgroundTheme = data.contains("Presets")
            ? data["Presets"].value("BackgroundTheme", std::string{})
            : std::string{};
        const float backgroundTileOffsetY = data.contains("Presets")
            ? data["Presets"].value("BackgroundTileOffsetY", 0.f)
            : 0.f;
        const auto& backgroundLayers = data["Background"];
        const std::size_t layerCount = backgroundLayers.size();
        std::vector<std::pair<std::string, sf::Vector2i>> occupiedSingleBackgroundTiles;
        auto tileIndexForPosition = [](const sf::Vector2f position) {
            return sf::Vector2i{
                std::max(0, static_cast<int>(std::floor(position.x / static_cast<float>(WINDOW_WIDTH)))),
                std::max(0, static_cast<int>(std::floor(position.y / static_cast<float>(WINDOW_HEIGHT))))
            };
        };

        std::size_t layerIndex = 0;
        for (const auto& backgroundData : backgroundLayers)
        {
            const sf::Vector2f position = {backgroundData["Position"][0], backgroundData["Position"][1]};
            const sf::Vector2f parallaxFactor = {
                backgroundData["ParallaxFactor"][0],
                backgroundData["ParallaxFactor"][1]
            };
            const std::string name = backgroundData["BgName"];
            if (this->data->backgroundTextures.find(name) == this->data->backgroundTextures.end())
            {
                std::cerr << "Level " << levelName << " references missing background texture: " << name << '\n';
                ++layerIndex;
                continue;
            }
            const Type type = parseBackgroundType(backgroundData.value("Type", nlohmann::json{}));
            const sf::Vector2i tileIndex = tileIndexForPosition(position);
            if (type == Type::SingleBackground)
            {
                const auto duplicateIt = std::find_if(
                    occupiedSingleBackgroundTiles.begin(),
                    occupiedSingleBackgroundTiles.end(),
                    [&](const auto& entry) {
                        return entry.first == name && entry.second == tileIndex;
                    }
                );
                if (duplicateIt != occupiedSingleBackgroundTiles.end())
                {
                    ++layerIndex;
                    continue;
                }
                occupiedSingleBackgroundTiles.emplace_back(name, tileIndex);
            }

            BackgroundSceneConfig sceneConfig;
            sceneConfig.themeName = backgroundData.value("Theme", backgroundTheme);
            sceneConfig.layerIndex = layerIndex;
            sceneConfig.layerCount = layerCount;
            sceneConfig.tileOffsetY = backgroundTileOffsetY;

            background.push_back(
                std::make_shared<Background>(
                    *this->data,
                    *this->camera,
                    *this,
                    position,
                    name,
                    parallaxFactor,
                    type,
                    sceneConfig
                )
            );

            ++layerIndex;
        }
    }
    catch (const std::exception&)
    {
        const std::string errorMsg = "Error loading level data for: " + levelName;
        std::cout << std::endl << errorMsg << std::endl;
        runErrorScreen(errorMsg);
    }
}

void GameLevel::initializeGround(const nlohmann::json& data)
{
    const auto makeFallbackGround = [&]() {
        const unsigned int levelWidth = static_cast<unsigned int>(std::max(size.x, 0));
        const unsigned int fallbackY = size.y > 40 ? static_cast<unsigned int>(size.y - 40) : 0u;

        std::cerr << "Level " << levelName << " has no ground strips. Using fallback ground.\n";
        ground = std::make_shared<Ground>(
            *this->data,
            *this,
            "TileSetGreen_02.png",
            0u,
            levelWidth,
            fallbackY,
            BASE_GROUND_OFFSET
        );
        ground->setVisualDepthRows(3u);
        ground->setStyle("VerdantKeep");
    };

    if (!data.contains("Ground") || !data["Ground"].is_array() || data["Ground"].empty())
    {
        makeFallbackGround();
        return;
    }

    for (const auto& groundData : data["Ground"])
    {
        const std::string groundName = groundData["GroundName"];
        const int rawPointBegin = groundData["Points"][0].get<int>();
        const int rawPointEnd = groundData["Points"][1].get<int>();
        const sf::Vector2u position = {
            static_cast<unsigned int>(std::clamp(rawPointBegin, 0, std::max(size.x, 0))),
            static_cast<unsigned int>(std::clamp(rawPointEnd, 0, std::max(size.x, 0)))
        };
        const unsigned int yPos = groundData["YPos"];
        const float offset = groundData.contains("Offset")
            ? groundData["Offset"].get<float>()
            : BASE_GROUND_OFFSET;
        const unsigned int depthRows = groundData.contains("DepthRows")
            ? groundData["DepthRows"].get<unsigned int>()
            : 0u;
        const std::string groundStyle = groundData.value("GroundStyle", std::string{});

        ground = std::make_shared<Ground>(
            *this->data,
            *this,
            groundName,
            position.x,
            position.y,
            yPos,
            offset
        );

        ground->setVisualDepthRows(depthRows);
        if (!groundStyle.empty())
        {
            ground->setStyle(groundStyle);
        }
    }

    if (!ground)
    {
        makeFallbackGround();
    }
}

void GameLevel::initializeEnemyManager(const nlohmann::json& data)
{
    if (!player || !platforms || !ground)
    {
        return;
    }

    enemyManager = std::make_unique<EnemyManager>(
        data,
        *this->data,
        *this,
        *platforms,
        *ground,
        *player,
        *window
    );
}

void GameLevel::initializeInteractives(const nlohmann::json& data)
{
    interactives.clear();

    if (!player)
    {
        return;
    }

    const auto parseColor = [](const nlohmann::json& object, const char* key, const sf::Color& fallback) {
        if (!object.contains(key))
        {
            return fallback;
        }

        const auto& color = object[key];
        return sf::Color{
            color[0].get<std::uint8_t>(),
            color[1].get<std::uint8_t>(),
            color[2].get<std::uint8_t>(),
            color[3].get<std::uint8_t>()
        };
    };

    if (data.contains("Interactives"))
    {
        for (const auto& interactiveData : data["Interactives"])
        {
            WorldInteractable::Config config;

            const std::string typeName = interactiveData.value("Type", "EchoTablet");
            if (typeName == "RestShrine")
            {
                config.type = WorldInteractable::Type::RestShrine;
            }
            else if (typeName == "GoldCache")
            {
                config.type = WorldInteractable::Type::GoldCache;
            }

            config.textureName = interactiveData.value("Texture", std::string{});
            config.position = {
                interactiveData["Position"][0].get<float>(),
                interactiveData["Position"][1].get<float>()
            };
            config.scale = interactiveData.contains("Scale")
                ? sf::Vector2f{
                    interactiveData["Scale"][0].get<float>(),
                    interactiveData["Scale"][1].get<float>()
                }
                : sf::Vector2f{1.f, 1.f};
            config.color = parseColor(interactiveData, "Color", sf::Color::White);
            config.accentColor = parseColor(interactiveData, "AccentColor", sf::Color(220, 184, 122, 255));
            config.interactRadius = interactiveData.value("InteractRadius", 120.f);
            config.goldReward = interactiveData.value("RewardGold", 0);
            config.singleUse = interactiveData.value("SingleUse", true);
            config.grantsCheckpoint = interactiveData.value("GrantsCheckpoint", config.type == WorldInteractable::Type::RestShrine);
            config.restoreVitality = interactiveData.value("RestoreVitality", config.type == WorldInteractable::Type::RestShrine);
            config.hiddenUntilNearby = interactiveData.value("HiddenUntilNearby", false);
            config.revealRadius = interactiveData.value("RevealRadius", 170.f);
            config.revealTitle = localizeDataText(interactiveData.value("RevealTitle", std::string{"Hidden path revealed"}));
            config.revealBody = localizeDataText(interactiveData.value("RevealBody", std::string{"Something concealed answered your approach."}));
            config.prompt = localizeDataText(interactiveData.value("Prompt", std::string{"Enter to interact"}));
            config.title = localizeDataText(interactiveData.value("Title", std::string{"Forgotten Relic"}));
            config.body = localizeDataText(interactiveData.value("Body", std::string{"The dead left a trace here."}));

            if (shouldUseAutomaticNameplate(interactiveData, typeName) || config.textureName.empty())
            {
                applyWorldNameplateVisual(config);
            }

            if (interactiveData.contains("SpawnOffset"))
            {
                config.hasCustomSpawnOffset = true;
                config.spawnOffset = {
                    interactiveData["SpawnOffset"][0].get<float>(),
                    interactiveData["SpawnOffset"][1].get<float>()
                };
            }

            interactives.push_back(std::make_unique<WorldInteractable>(
                *this->data,
                *this->camera,
                *this->levelManager,
                *player,
                config
            ));
        }
    }

    for (const auto& generatedLocation : generatedMiniLocations)
    {
        MiniLocationEntrance::Config entranceConfig;
        entranceConfig.textureName = generatedLocation.entranceTexture;
        entranceConfig.position = generatedLocation.entrancePosition;
        entranceConfig.scale = generatedLocation.entranceScale;
        entranceConfig.destinationSupportPoint = generatedLocation.entranceDestinationSupport;
        entranceConfig.color = generatedLocation.entranceColor;
        entranceConfig.accentColor = generatedLocation.accentColor;
        entranceConfig.interactRadius = generatedLocation.interactRadius;
        entranceConfig.prompt = generatedLocation.entrancePrompt;
        entranceConfig.subtitle = generatedLocation.title;
        entranceConfig.exitsMiniLocation = false;
        entranceConfig.miniLocationId = generatedLocation.id;

        interactives.push_back(std::make_unique<MiniLocationEntrance>(
            *this->data,
            *this->camera,
            *this->levelManager,
            *player,
            entranceConfig
        ));

        MiniLocationEntrance::Config exitConfig;
        exitConfig.textureName = generatedLocation.exitTexture;
        exitConfig.position = generatedLocation.exitPosition;
        exitConfig.scale = generatedLocation.exitScale;
        exitConfig.destinationSupportPoint = generatedLocation.exitDestinationSupport;
        exitConfig.color = generatedLocation.exitColor;
        exitConfig.accentColor = generatedLocation.accentColor;
        exitConfig.interactRadius = generatedLocation.interactRadius;
        exitConfig.prompt = generatedLocation.exitPrompt;
        exitConfig.subtitle = "Return to the trail";
        exitConfig.exitsMiniLocation = true;
        exitConfig.miniLocationId = generatedLocation.id;

        interactives.push_back(std::make_unique<MiniLocationEntrance>(
            *this->data,
            *this->camera,
            *this->levelManager,
            *player,
            exitConfig
        ));
    }

    for (const auto& generatedReward : generatedMiniRewards)
    {
        WorldInteractable::Config config;

        if (generatedReward.typeName == "RestShrine")
        {
            config.type = WorldInteractable::Type::RestShrine;
        }
        else if (generatedReward.typeName == "GoldCache")
        {
            config.type = WorldInteractable::Type::GoldCache;
        }
        else
        {
            config.type = WorldInteractable::Type::EchoTablet;
        }

        config.textureName = generatedReward.textureName;
        config.position = generatedReward.position;
        config.scale = generatedReward.scale;
        config.color = generatedReward.color;
        config.accentColor = generatedReward.accentColor;
        config.interactRadius = generatedReward.interactRadius;
        config.goldReward = generatedReward.goldReward;
        config.singleUse = generatedReward.singleUse;
        config.grantsCheckpoint = generatedReward.grantsCheckpoint;
        config.restoreVitality = generatedReward.restoreVitality;
        config.hasCustomSpawnOffset = generatedReward.hasCustomSpawnOffset;
        config.spawnOffset = generatedReward.spawnOffset;
        config.hiddenUntilNearby = true;
        config.revealRadius = 210.f;
        config.revealTitle = generatedReward.title;
        config.revealBody = "A hidden reward stirs nearby. Search the pocket space carefully.";
        config.prompt = generatedReward.prompt;
        config.title = generatedReward.title;
        config.body = generatedReward.body;
        applyWorldNameplateVisual(config);

        interactives.push_back(std::make_unique<WorldInteractable>(
            *this->data,
            *this->camera,
            *this->levelManager,
            *player,
            config
        ));
    }
}

void GameLevel::initializeExplicitMiniLocations(const nlohmann::json& data)
{
    generatedMiniLocations.clear();
    generatedMiniRewards.clear();
    generatedMiniBarriers.clear();
    generatedMiniHazards.clear();

    if (!platforms || !data.contains("MiniLocations") || !data["MiniLocations"].is_array())
    {
        return;
    }

    float farthestRightX = static_cast<float>(size.x);

    for (const auto& locationData : data["MiniLocations"])
    {
        const sf::FloatRect bounds = readRect(locationData.value("Bounds", nlohmann::json::array()));
        if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
        {
            continue;
        }

        const nlohmann::json entryData = locationData.value("Entry", nlohmann::json::object());
        const nlohmann::json exitData = locationData.value("Exit", nlohmann::json::object());
        const sf::Color defaultAccentColor = sf::Color(130, 214, 184, 255);
        const sf::Color accentColor = locationData.contains("AccentColor")
            ? readColor(locationData["AccentColor"], defaultAccentColor)
            : (entryData.contains("AccentColor")
                ? readColor(entryData["AccentColor"], defaultAccentColor)
                : (exitData.contains("AccentColor")
                    ? readColor(exitData["AccentColor"], defaultAccentColor)
                    : defaultAccentColor));

        GeneratedMiniLocation location;
        location.id = locationData.value("Id", locationData.value("Title", std::string{"mini_location"}));
        location.title = locationData.value("Title", locationData.value("Id", std::string{"Mini Location"}));
        location.cameraBounds = bounds;
        location.entranceTexture = entryData.value("Texture", std::string{"MossyDecorationHazard_25.png"});
        location.exitTexture = exitData.value("Texture", std::string{"MossyDecorationHazard_24.png"});
        location.entrancePosition = readMiniLocationPoint(
            entryData,
            "Position",
            bounds,
            {bounds.position.x + 64.f, bounds.position.y + bounds.size.y - 20.f}
        );
        const sf::Vector2f portalSpawnPosition = bounds.position + readVector2f(
            locationData.value("SpawnPosition", nlohmann::json::array()),
            {bounds.size.x * 0.5f, bounds.size.y - 42.f}
        );
        location.exitPosition = portalSpawnPosition;
        location.entranceDestinationSupport = readMiniLocationPoint(
            entryData,
            "DestinationSupport",
            bounds,
            portalSpawnPosition
        );
        location.exitDestinationSupport = readMiniLocationPoint(
            exitData,
            "DestinationSupport",
            bounds,
            {location.entrancePosition.x, location.entrancePosition.y - 18.f}
        );
        location.entranceScale = readVector2f(
            entryData.value("Scale", nlohmann::json::array()),
            kMiniLocationPortalScale
        );
        location.exitScale = readVector2f(
            exitData.value("Scale", nlohmann::json::array()),
            kMiniLocationPortalScale
        );
        location.entranceColor = readColor(
            entryData.value("Color", nlohmann::json::array()),
            sf::Color(214, 246, 232, 255)
        );
        location.exitColor = readColor(
            exitData.value("Color", nlohmann::json::array()),
            sf::Color(212, 232, 255, 255)
        );
        location.accentColor = accentColor;
        location.interactRadius = entryData.value(
            "InteractRadius",
            exitData.value("InteractRadius", 126.f)
        );
        location.entrancePrompt = localizeDataText(entryData.value("Prompt", std::string{"Enter the hidden route"}));
        location.exitPrompt = localizeDataText(exitData.value("Prompt", std::string{"Enter to return"}));
        location.roomLeftX = bounds.position.x;
        location.roomRightX = bounds.position.x + bounds.size.x;
        location.activeLeftX = location.roomLeftX;
        location.activeRightX = location.roomRightX;
        location.roomCeilingY = bounds.position.y;
        location.roomFloorY = bounds.position.y + bounds.size.y;
        location.deathY = locationData.value("DeathY", std::numeric_limits<float>::max());

        const sf::Color barrierColor = locationData.contains("BarrierColor")
            ? readColor(locationData["BarrierColor"], accentColor)
            : accentColor;
        const sf::Color barrierGlowColor = locationData.contains("BarrierGlowColor")
            ? readColor(locationData["BarrierGlowColor"], brighten(accentColor, 0.2f))
            : brighten(accentColor, 0.2f);

        const auto addBarrier = [&](const nlohmann::json& barrierData, const bool relativeRect) {
            if (!barrierData.value("Enabled", true))
            {
                return;
            }
            sf::FloatRect barrierRect = readRect(barrierData.value("Rect", nlohmann::json::array()));
            if (relativeRect)
            {
                barrierRect.position += bounds.position;
            }
            if (barrierRect.size.x <= 0.f || barrierRect.size.y <= 0.f)
            {
                return;
            }
            if (barrierData.value("BlocksPlayer", true))
            {
                addBarrierRectCollision(*platforms, barrierRect);
            }
            generatedMiniBarriers.push_back({
                barrierRect.position.x + barrierRect.size.x * 0.5f,
                barrierRect.position.y,
                barrierRect.position.y + barrierRect.size.y,
                barrierRect.size.x,
                randomFloat(0.f, 6.28318f),
                barrierData.contains("CoreColor") ? readColor(barrierData["CoreColor"], barrierColor) : barrierColor,
                barrierData.contains("GlowColor") ? readColor(barrierData["GlowColor"], barrierGlowColor) : barrierGlowColor
            });
        };

        if (locationData.contains("Barriers") && locationData["Barriers"].is_array())
        {
            for (const auto& barrierData : locationData["Barriers"])
            {
                addBarrier(barrierData, true);
            }
        }
        else if (locationData.value("BarrierEnabled", true))
        {
            const float barrierWidth = locationData.value("BarrierWidth", 22.f);
            const float barrierTop = 0.f;
            const float barrierHeight = static_cast<float>(size.y) + 180.f;
            addBarrier({{"Rect", {bounds.position.x - 18.f, barrierTop, barrierWidth, barrierHeight}}, {"BlocksPlayer", true}}, false);
            addBarrier({{"Rect", {bounds.position.x + bounds.size.x + 18.f, barrierTop, barrierWidth, barrierHeight}}, {"BlocksPlayer", true}}, false);
        }

        const auto addDeadArea = [&](const nlohmann::json& hazardData, const bool rectIsRelative) {
            if (!hazardData.value("Enabled", true))
            {
                return;
            }

            sf::FloatRect hazardRect = readRect(
                hazardData.value("Rect", nlohmann::json::array()),
                sf::FloatRect(
                    {bounds.position.x, bounds.position.y + bounds.size.y + 64.f},
                    {bounds.size.x, 64.f}
                )
            );
            if (rectIsRelative)
            {
                hazardRect.position += bounds.position;
            }

            generatedMiniHazards.push_back({
                hazardRect.position.x,
                hazardRect.position.x + hazardRect.size.x,
                hazardRect.position.y,
                hazardRect.position.y + hazardRect.size.y,
                randomFloat(0.f, 6.28318f),
                hazardData.contains("CoreColor")
                    ? readColor(hazardData["CoreColor"], sf::Color(242, 104, 56, 255))
                    : sf::Color(242, 104, 56, 255),
                hazardData.contains("GlowColor")
                    ? readColor(hazardData["GlowColor"], sf::Color(255, 182, 96, 255))
                    : sf::Color(255, 182, 96, 255),
                hazardData.contains("EmberColor")
                    ? readColor(hazardData["EmberColor"], sf::Color(255, 236, 188, 255))
                    : sf::Color(255, 236, 188, 255),
                location.id
            });

            if (!locationData.contains("DeathY"))
            {
                location.deathY = std::min(location.deathY, hazardRect.position.y);
            }
        };

        if (locationData.contains("DeadAreas") && locationData["DeadAreas"].is_array())
        {
            for (const auto& deadAreaData : locationData["DeadAreas"])
            {
                addDeadArea(deadAreaData, true);
            }
        }
        else if (locationData.contains("Hazard"))
        {
            addDeadArea(locationData["Hazard"], false);
        }

        generatedMiniLocations.push_back(location);
        farthestRightX = std::max(farthestRightX, bounds.position.x + bounds.size.x + 160.f);
    }

    size.x = std::max(size.x, static_cast<int>(std::ceil(farthestRightX)));
}

void GameLevel::initializeWorldHazards(const nlohmann::json& data)
{
    if (data.contains("DeadAreas") && data["DeadAreas"].is_array())
    {
        for (const auto& deadAreaData : data["DeadAreas"])
        {
            if (!deadAreaData.value("Enabled", true) ||
                !deadAreaData.value("MiniLocationId", std::string{}).empty())
            {
                continue;
            }

            const sf::FloatRect rect = readRect(deadAreaData.value("Rect", nlohmann::json::array()));
            if (rect.size.x <= 0.f || rect.size.y <= 0.f)
            {
                continue;
            }

            generatedMiniHazards.push_back({
                rect.position.x,
                rect.position.x + rect.size.x,
                rect.position.y,
                rect.position.y + rect.size.y,
                randomFloat(0.f, 6.28318f),
                deadAreaData.contains("CoreColor") ? readColor(deadAreaData["CoreColor"], sf::Color(242, 104, 56, 255)) : sf::Color(242, 104, 56, 255),
                deadAreaData.contains("GlowColor") ? readColor(deadAreaData["GlowColor"], sf::Color(255, 182, 96, 255)) : sf::Color(255, 182, 96, 255),
                deadAreaData.contains("EmberColor") ? readColor(deadAreaData["EmberColor"], sf::Color(255, 236, 188, 255)) : sf::Color(255, 236, 188, 255),
                std::string{}
            });
        }
    }

    if (platforms && data.contains("Barriers") && data["Barriers"].is_array())
    {
        for (const auto& barrierData : data["Barriers"])
        {
            if (!barrierData.value("Enabled", true) ||
                !barrierData.value("MiniLocationId", std::string{}).empty())
            {
                continue;
            }

            const sf::FloatRect rect = readRect(barrierData.value("Rect", nlohmann::json::array()));
            if (rect.size.x <= 0.f || rect.size.y <= 0.f)
            {
                continue;
            }

            if (barrierData.value("BlocksPlayer", true))
            {
                addBarrierRectCollision(*platforms, rect);
            }
            generatedMiniBarriers.push_back({
                rect.position.x + rect.size.x * 0.5f,
                rect.position.y,
                rect.position.y + rect.size.y,
                rect.size.x,
                randomFloat(0.f, 6.28318f),
                barrierData.contains("CoreColor") ? readColor(barrierData["CoreColor"], sf::Color(130, 214, 184, 255)) : sf::Color(130, 214, 184, 255),
                barrierData.contains("GlowColor") ? readColor(barrierData["GlowColor"], sf::Color(156, 238, 208, 255)) : sf::Color(156, 238, 208, 255)
            });
        }
    }
}

void GameLevel::generateMiniLocations()
{
    generatedMiniLocations.clear();
    generatedMiniRewards.clear();
    generatedMiniBarriers.clear();
    generatedMiniHazards.clear();

    if (!platforms || !decorations || !ground || loadedLevelData.is_null())
    {
        return;
    }

    const float baseWidth = static_cast<float>(size.x);
    const float groundY = ground->getRect().getPosition().y;
    const sf::Vector2f spawnPos = {
        loadedLevelData["Presets"]["PlayerSpawn"][0].get<float>(),
        loadedLevelData["Presets"]["PlayerSpawn"][1].get<float>()
    };

    std::vector<sf::Vector2f> reservedPositions;
    reservedPositions.push_back(spawnPos);

    if (loadedLevelData.contains("Interactives"))
    {
        for (const auto& interactiveData : loadedLevelData["Interactives"])
        {
            reservedPositions.push_back({
                interactiveData["Position"][0].get<float>(),
                interactiveData["Position"][1].get<float>()
            });
        }
    }

    std::vector<MiniLocationCandidate> candidates;
    const auto& platformRects = platforms->getRects();

    const auto tryAddCandidate = [&](MiniLocationCandidate candidate) {
        candidate.centerX = std::clamp(candidate.centerX, 220.f, baseWidth - 220.f);
        candidate.leftX = std::clamp(candidate.leftX, 0.f, baseWidth);
        candidate.rightX = std::clamp(candidate.rightX, 0.f, baseWidth);

        if (candidate.rightX - candidate.leftX < 120.f)
        {
            return;
        }

        if (std::abs(candidate.centerX - spawnPos.x) < 440.f)
        {
            return;
        }

        for (const sf::Vector2f& reserved : reservedPositions)
        {
            if (std::abs(candidate.centerX - reserved.x) < 340.f
                && std::abs(candidate.surfaceY - reserved.y) < 240.f)
            {
                return;
            }
        }

        if (!hasEntranceClearance(candidate, platformRects))
        {
            return;
        }

        candidates.push_back(candidate);
    };

    for (const auto& platformRect : platformRects)
    {
        const sf::FloatRect bounds = platformRect->getGlobalBounds();
        if (bounds.size.x < 145.f || bounds.size.y > 95.f || bounds.size.x < bounds.size.y)
        {
            continue;
        }

        tryAddCandidate({
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y,
            bounds.position.x + 20.f,
            bounds.position.x + bounds.size.x - 20.f
        });
    }

    constexpr std::array<float, 5> kGroundFractions{0.18f, 0.36f, 0.54f, 0.72f, 0.84f};
    for (const float fraction : kGroundFractions)
    {
        const float center = std::clamp(baseWidth * fraction + randomFloat(-170.f, 170.f), 220.f, baseWidth - 220.f);
        tryAddCandidate({center, groundY, center - 128.f, center + 128.f});
    }

    std::shuffle(candidates.begin(), candidates.end(), miniLocationRng());

    const int desiredCount = std::clamp(static_cast<int>(std::round(baseWidth / 3600.f)), 1, 3);
    const float minSpacing = std::max(860.f, baseWidth * 0.13f);

    std::vector<MiniLocationCandidate> selectedCandidates;
    for (const auto& candidate : candidates)
    {
        const bool overlapsSelected = std::any_of(selectedCandidates.begin(), selectedCandidates.end(),
            [&](const MiniLocationCandidate& selected) {
                return std::abs(candidate.centerX - selected.centerX) < minSpacing;
            });
        if (overlapsSelected)
        {
            continue;
        }

        selectedCandidates.push_back(candidate);
        if (static_cast<int>(selectedCandidates.size()) >= desiredCount)
        {
            break;
        }
    }

    if (selectedCandidates.empty())
    {
        const float fallbackCenter = std::clamp(baseWidth * 0.62f, 240.f, baseWidth - 240.f);
        selectedCandidates.push_back({fallbackCenter, groundY, fallbackCenter - 128.f, fallbackCenter + 128.f});
    }

    std::sort(selectedCandidates.begin(), selectedCandidates.end(), [](const MiniLocationCandidate& lhs, const MiniLocationCandidate& rhs) {
        return lhs.centerX < rhs.centerX;
    });

    constexpr float kPocketWorldGap = 1860.f;
    constexpr float kPocketRoomSpacing = 1040.f;
    constexpr float kPocketSealOverflow = 180.f;
    constexpr float kPocketWorldSealInset = 92.f;
    const float worldSealX = std::max(0.f, baseWidth - kPocketWorldSealInset);
    const float worldSealTop = 0.f;
    const float worldSealBottom = static_cast<float>(size.y) + kPocketSealOverflow;

    addMiniLocationSeal(
        *platforms,
        worldSealX,
        worldSealTop,
        worldSealBottom
    );
    generatedMiniBarriers.push_back({
        worldSealX + 36.f,
        worldSealTop,
        worldSealBottom,
        24.f,
        randomFloat(0.f, 6.28318f),
        sf::Color(136, 232, 220, 255),
        sf::Color(102, 198, 192, 255)
    });

    float extensionCursor = baseWidth + kPocketWorldGap;
    const auto& themes = getMiniLocationThemes();

    for (std::size_t index = 0; index < selectedCandidates.size(); ++index)
    {
        const MiniLocationCandidate& candidate = selectedCandidates[index];
        const MiniLocationTheme& theme = pickRandom(themes);

        const float roomWidth = randomFloat(960.f, 1180.f);
        const float roomFloorY = std::clamp(groundY - randomFloat(86.f, 124.f), 840.f, 942.f);
        const float roomHeight = randomFloat(250.f, 320.f);
        const float roomX = extensionCursor;
        const float roomCenterX = roomX + roomWidth * 0.5f;
        const float roomCeilingY = roomFloorY - roomHeight;
        const float shelfY = roomFloorY - randomFloat(150.f, 174.f);
        const float roomLeftSealX = roomX - 102.f;
        const float roomRightSealX = roomX + roomWidth + 32.f;
        const float roomSealTop = 0.f;
        const float roomSealBottom = static_cast<float>(size.y) + kPocketSealOverflow;
        const float deathY = std::min(static_cast<float>(size.y) - 24.f, roomFloorY + randomFloat(152.f, 184.f));
        const float hazardTopY = std::max(roomFloorY + 88.f, deathY - 20.f);
        const float hazardBottomY = static_cast<float>(size.y) + 56.f;
        const std::string miniLocationId = "procedural_mini_location_" + std::to_string(index + 1u);

        extensionCursor += roomWidth + kPocketRoomSpacing + randomFloat(80.f, 180.f);
        size.x = std::max(size.x, static_cast<int>(std::ceil(roomX + roomWidth + 420.f)));

        addMiniLocationSeal(
            *platforms,
            roomLeftSealX,
            roomSealTop,
            roomSealBottom
        );
        addMiniLocationSeal(
            *platforms,
            roomRightSealX,
            roomSealTop,
            roomSealBottom
        );
        generatedMiniBarriers.push_back({
            roomLeftSealX + 36.f,
            roomSealTop,
            roomSealBottom,
            22.f,
            randomFloat(0.f, 6.28318f),
            theme.accentColor,
            brighten(theme.entranceColor, 0.18f)
        });
        generatedMiniBarriers.push_back({
            roomRightSealX + 36.f,
            roomSealTop,
            roomSealBottom,
            22.f,
            randomFloat(0.f, 6.28318f),
            theme.exitColor,
            brighten(theme.accentColor, 0.24f)
        });
        generatedMiniHazards.push_back({
            roomX - 8.f,
            roomX + roomWidth + 56.f,
            hazardTopY,
            hazardBottomY,
            randomFloat(0.f, 6.28318f),
            sf::Color(242, 104, 56, 255),
            sf::Color(255, 182, 96, 255),
            sf::Color(255, 236, 188, 255),
            miniLocationId
        });

        for (float floorX = roomX + 28.f; floorX <= roomX + roomWidth - 330.f; floorX += 320.f)
        {
            platforms->addPlatform({floorX, roomFloorY}, "Quadruple");
        }
        platforms->addPlatform({roomX + roomWidth - 308.f, roomFloorY}, "Quadruple");
        platforms->addPlatform({roomX + 18.f, roomFloorY - 142.f}, "Crypt-pillar");
        platforms->addPlatform({roomX + roomWidth - 78.f, roomFloorY - 142.f}, "Crypt-pillar");
        platforms->addPlatform({roomCenterX - 166.f, roomCeilingY + 34.f}, "Fallen-arcade");
        platforms->addPlatform({roomCenterX - 135.f, shelfY}, "Triple");
        if (index % 2 == 0)
        {
            platforms->addPlatform({roomX + 136.f, roomFloorY - 96.f}, "Double-horizontal-1");
        }
        else
        {
            platforms->addPlatform({roomX + roomWidth - 252.f, roomFloorY - 98.f}, "Double-horizontal-2");
        }

        decorations->addDecoration(
            theme.backdropDecoration,
            {roomCenterX, roomFloorY - 118.f},
            {1.04f, 1.04f},
            {0.12f, 0.10f},
            -4,
            sf::Color(255, 255, 255, 96)
        );
        decorations->addDecoration(
            theme.hillDecoration,
            {roomCenterX, roomFloorY + 34.f},
            {0.84f, 0.84f},
            {0.18f, 0.14f},
            -3,
            sf::Color(176, 198, 204, 162)
        );
        decorations->addDecoration(
            theme.hangingDecoration,
            {roomCenterX, roomCeilingY + 18.f},
            {0.58f, 0.58f},
            {0.32f, 0.28f},
            -1,
            sf::Color(170, 214, 188, 204)
        );
        decorations->addDecoration(
            theme.ambientDecoration,
            {roomCenterX - 132.f, roomFloorY + 62.f},
            {0.34f, 0.34f},
            {0.68f, 0.68f},
            2,
            sf::Color(236, 255, 244, 236)
        );
        decorations->addDecoration(
            theme.ambientDecoration,
            {roomCenterX + 146.f, roomFloorY + 60.f},
            {0.28f, 0.28f},
            {0.70f, 0.70f},
            2,
            sf::Color(236, 255, 244, 220)
        );
        decorations->addDecoration(
            "MossyDecorationHazard_03.png",
            {roomX + 86.f, roomFloorY - 10.f},
            {0.22f, 0.22f},
            {0.54f, 0.50f},
            1,
            sf::Color(255, 255, 255, 220)
        );
        decorations->addDecoration(
            "MossyDecorationHazard_19.png",
            {roomX + roomWidth - 80.f, roomFloorY - 6.f},
            {0.20f, 0.20f},
            {0.54f, 0.50f},
            1,
            sf::Color(255, 255, 255, 216)
        );

        const float returnDirection = candidate.centerX < baseWidth * 0.72f ? 1.f : -1.f;
        const float returnSupportX = std::clamp(
            candidate.centerX + returnDirection * 88.f,
            candidate.leftX + 28.f,
            candidate.rightX - 28.f
        );

        GeneratedMiniLocation generatedLocation;
        generatedLocation.title = localizeDataText(theme.title);
        generatedLocation.id = miniLocationId;
        generatedLocation.cameraBounds = {{roomLeftSealX - 28.f, 0.f}, {roomRightSealX - roomLeftSealX + 56.f, static_cast<float>(size.y)}};
        generatedLocation.entranceTexture = theme.entranceTexture;
        generatedLocation.exitTexture = theme.exitTexture;
        generatedLocation.entrancePosition = {candidate.centerX, candidate.surfaceY + 18.f};
        generatedLocation.entranceDestinationSupport = {roomX + 198.f, roomFloorY};
        generatedLocation.entranceScale = kMiniLocationPortalScale;
        generatedLocation.exitPosition = {roomX + 110.f, roomFloorY + 18.f};
        generatedLocation.exitDestinationSupport = {returnSupportX, candidate.surfaceY};
        generatedLocation.exitScale = kMiniLocationPortalScale;
        generatedLocation.entranceColor = theme.entranceColor;
        generatedLocation.exitColor = theme.exitColor;
        generatedLocation.accentColor = theme.accentColor;
        generatedLocation.interactRadius = 126.f;
        generatedLocation.entrancePrompt = localizeDataText(theme.entrancePrompt);
        generatedLocation.exitPrompt = localizeDataText(theme.exitPrompt);
        generatedLocation.roomLeftX = roomX - 8.f;
        generatedLocation.roomRightX = roomX + roomWidth + 56.f;
        generatedLocation.activeLeftX = roomLeftSealX - 28.f;
        generatedLocation.activeRightX = roomRightSealX + 28.f;
        generatedLocation.roomCeilingY = roomCeilingY;
        generatedLocation.roomFloorY = roomFloorY;
        generatedLocation.deathY = deathY;
        generatedMiniLocations.push_back(generatedLocation);

        GeneratedMiniReward reward;
        reward.typeName = theme.rewardTypeName;
        reward.textureName = theme.rewardTexture;
        reward.position = {roomCenterX + randomFloat(-12.f, 12.f), shelfY - 8.f};
        reward.scale = theme.rewardScale;
        reward.color = sf::Color::White;
        reward.accentColor = theme.accentColor;
        reward.interactRadius = 118.f;
        reward.goldReward = theme.goldReward;
        reward.singleUse = true;
        reward.grantsCheckpoint = theme.grantsCheckpoint;
        reward.restoreVitality = theme.restoreVitality;
        reward.prompt = localizeDataText(theme.rewardPrompt);
        reward.title = localizeDataText(theme.rewardTitle);
        reward.body = localizeDataText(theme.rewardBody);
        if (theme.grantsCheckpoint)
        {
            reward.hasCustomSpawnOffset = true;
            reward.spawnOffset = {72.f, -126.f};
        }
        generatedMiniRewards.push_back(reward);
    }
}

void GameLevel::tryInitializeEnemyManager()
{
    if (player && !loadedLevelData.is_null())
    {
        initializeEnemyManager(loadedLevelData);
    }
}

void GameLevel::tryInitializeInteractives()
{
    if (player && !loadedLevelData.is_null())
    {
        initializeInteractives(loadedLevelData);
    }
}

void GameLevel::queueNotification(std::string title, std::string body, const NotificationTone tone) const
{
    if (levelManager)
    {
        levelManager->pushNotification(std::move(title), std::move(body), tone);
    }
}

void GameLevel::initializePortals(const nlohmann::json& data)
{
    if (!player || !levelManager || !camera || !data.contains("Portals") || !data["Portals"].is_array())
    {
        return;
    }

    std::map<std::string, sf::Vector2f> miniLocationEntrances;
    for (const auto& location : generatedMiniLocations)
    {
        miniLocationEntrances[location.title] = location.entranceDestinationSupport;
    }

    if (data.contains("MiniLocations") && data["MiniLocations"].is_array())
    {
        for (const auto& locationData : data["MiniLocations"])
        {
            const std::string id = locationData.value("Id", locationData.value("Title", std::string{}));
            if (id.empty())
            {
                continue;
            }

            const sf::FloatRect bounds = readRect(locationData.value("Bounds", nlohmann::json::array()));
            const sf::Vector2f defaultSpawn{bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y - 42.f};
            if (locationData.contains("SpawnPosition"))
            {
                miniLocationEntrances[id] = bounds.position + readVector2f(locationData["SpawnPosition"], {bounds.size.x * 0.5f, bounds.size.y - 42.f});
            }
            else
            {
                const nlohmann::json entryData = locationData.value("Entry", nlohmann::json::object());
                miniLocationEntrances[id] = readVector2f(
                    entryData.value("DestinationSupport", nlohmann::json::array()),
                    defaultSpawn
                );
            }
        }
    }

    for (const auto& portalData : data["Portals"])
    {
        WorldPortal::Config config;
        config.id = portalData.value("Id", std::string{});
        config.position = readVector2f(portalData.value("Position", nlohmann::json::array()));
        config.scale = readVector2f(portalData.value("Scale", nlohmann::json::array()), {0.36f, 0.36f});
        config.color = readColor(portalData.value("Color", nlohmann::json::array()), sf::Color(212, 236, 255, 245));
        config.accentColor = readColor(portalData.value("AccentColor", nlohmann::json::array()), sf::Color(112, 208, 255, 255));
        config.interactRadius = portalData.value("InteractRadius", 130.f);
        config.hasActivationArea = portalData.contains("ActivationArea") && portalData["ActivationArea"].is_array();
        config.activationArea = readRect(portalData.value("ActivationArea", nlohmann::json::array()));
        config.portalTexture = portalData.value("PortalTexture", portalData.value("Texture", std::string{"portalGreen"}));
        config.prompt = portalData.value("Prompt", std::string{"Enter portal"});
        config.title = portalData.value("Title", std::string{"World Portal"});
        config.target = parseWorldPortalTarget(portalData, miniLocationEntrances);

        interactives.push_back(std::make_unique<WorldPortal>(
            *this->data,
            *this->camera,
            *this->levelManager,
            *player,
            config
        ));
    }
}

void GameLevel::initializeSharedDrawOrder(const nlohmann::json& data)
{
    sharedWorldDrawOrder_.clear();
    useSharedWorldDrawOrder_ = false;

    int fallbackOrder = 0;

    if (data.contains("Background") && data["Background"].is_array())
    {
        for (std::size_t index = 0; index < data["Background"].size(); ++index)
        {
            sharedWorldDrawOrder_.push_back({
                SharedDrawEntry::Kind::Background,
                index,
                readEditorDrawOrder(data["Background"][index], fallbackOrder++)
            });
        }
    }

    if (data.contains("Decorations") && data["Decorations"].is_array())
    {
        for (std::size_t index = 0; index < data["Decorations"].size(); ++index)
        {
            sharedWorldDrawOrder_.push_back({
                SharedDrawEntry::Kind::Decoration,
                index,
                readEditorDrawOrder(data["Decorations"][index], fallbackOrder++)
            });
        }
    }

    if (ground)
    {
        const nlohmann::json groundData = data.contains("Ground") && data["Ground"].is_array() && !data["Ground"].empty()
            ? data["Ground"].front()
            : nlohmann::json::object();
        sharedWorldDrawOrder_.push_back({
            SharedDrawEntry::Kind::Ground,
            0u,
            readEditorDrawOrder(groundData, fallbackOrder++)
        });
    }

    sharedWorldDrawOrder_.push_back({
        SharedDrawEntry::Kind::Actor,
        0u,
        readPresetDrawOrder(data, "ActorDrawOrder", fallbackOrder++)
    });

    const std::size_t explicitInteractiveCount =
        data.contains("Interactives") && data["Interactives"].is_array()
            ? data["Interactives"].size()
            : 0u;
    const std::size_t explicitPortalCount =
        data.contains("Portals") && data["Portals"].is_array()
            ? data["Portals"].size()
            : 0u;
    const std::size_t portalStartIndex = interactives.size() >= explicitPortalCount
        ? interactives.size() - explicitPortalCount
        : interactives.size();

    for (std::size_t index = 0; index < interactives.size(); ++index)
    {
        int order = fallbackOrder++;
        if (index < explicitInteractiveCount)
        {
            order = readEditorDrawOrder(data["Interactives"][index], order);
        }
        else if (index >= portalStartIndex && data.contains("Portals") && data["Portals"].is_array())
        {
            const std::size_t portalIndex = index - portalStartIndex;
            if (portalIndex < data["Portals"].size())
            {
                order = readEditorDrawOrder(data["Portals"][portalIndex], order);
            }
        }
        sharedWorldDrawOrder_.push_back({
            SharedDrawEntry::Kind::Interactive,
            index,
            order
        });
    }

    if (data.contains("Platforms") && data["Platforms"].is_array())
    {
        for (std::size_t index = 0; index < data["Platforms"].size(); ++index)
        {
            sharedWorldDrawOrder_.push_back({
                SharedDrawEntry::Kind::Platform,
                index,
                readEditorDrawOrder(data["Platforms"][index], fallbackOrder++)
            });
        }
    }

    if (sharedWorldDrawOrder_.empty())
    {
        return;
    }

    const auto kindRank = [](const SharedDrawEntry::Kind kind) {
        switch (kind)
        {
            case SharedDrawEntry::Kind::Background:
                return 0;
            case SharedDrawEntry::Kind::Decoration:
                return 1;
            case SharedDrawEntry::Kind::Ground:
                return 2;
            case SharedDrawEntry::Kind::Actor:
                return 3;
            case SharedDrawEntry::Kind::Interactive:
                return 4;
            case SharedDrawEntry::Kind::Platform:
                return 5;
        }
        return 6;
    };

    std::sort(sharedWorldDrawOrder_.begin(), sharedWorldDrawOrder_.end(), [&](const SharedDrawEntry& lhs, const SharedDrawEntry& rhs) {
        if (lhs.order != rhs.order)
        {
            return lhs.order < rhs.order;
        }
        if (lhs.kind != rhs.kind)
        {
            return kindRank(lhs.kind) < kindRank(rhs.kind);
        }
        return lhs.index < rhs.index;
    });

    const std::size_t platformCount = platforms ? platforms->getInstanceCount() : 0u;
    const std::size_t decorationCount = decorations ? decorations->getInstanceCount() : 0u;
    sharedWorldDrawOrder_.erase(
        std::remove_if(sharedWorldDrawOrder_.begin(), sharedWorldDrawOrder_.end(),
            [&](const SharedDrawEntry& entry) {
                switch (entry.kind)
                {
                    case SharedDrawEntry::Kind::Background:
                        return entry.index >= background.size();
                    case SharedDrawEntry::Kind::Decoration:
                        return !decorations || entry.index >= decorationCount;
                    case SharedDrawEntry::Kind::Ground:
                        return !ground;
                    case SharedDrawEntry::Kind::Actor:
                        return false;
                    case SharedDrawEntry::Kind::Interactive:
                        return entry.index >= interactives.size();
                    case SharedDrawEntry::Kind::Platform:
                        return !platforms || entry.index >= platformCount;
                }
                return true;
            }),
        sharedWorldDrawOrder_.end()
    );

    useSharedWorldDrawOrder_ = !sharedWorldDrawOrder_.empty();
}

void GameLevel::initializeEventZones(const nlohmann::json& data)
{
    levelEventZones.clear();

    if (playerSpawnPos != sf::Vector2f{0.f, 0.f})
    {
        levelEventZones.push_back({
            localizeDataText("Checkpoint attuned"),
            localizeDataText("Your return point for this realm has been refreshed."),
            sf::FloatRect(playerSpawnPos - sf::Vector2f{100.f, 180.f}, {200.f, 260.f}),
            NotificationTone::Success,
            true,
            true,
            false
        });
    }

    if (data.contains("Spawners") && data["Spawners"].is_array())
    {
        for (const auto& spawnerData : data["Spawners"])
        {
            if (spawnerData.value("EnemyAmount", 0) <= 0)
            {
                continue;
            }

            const sf::FloatRect spawnArea = readRect(spawnerData.value("SpawnArea", nlohmann::json::array()));
            if (spawnArea.size.x <= 0.f || spawnArea.size.y <= 0.f)
            {
                continue;
            }

            const std::string enemyName = spawnerData.value("EnemyName", std::string{"Enemies"});
            levelEventZones.push_back({
                localizeDataText(spawnerData.value("EncounterTitle", std::string{"Threat ahead"})),
                localizeDataText(spawnerData.value("EncounterBody", enemyName + " gather in this stretch. Enter prepared.")),
                spawnArea,
                parseNotificationTone(spawnerData.value("EncounterTone", std::string{"Warning"})),
                spawnerData.value("EncounterFireOnce", true),
                true,
                false
            });
        }
    }

    if (data.contains("Events") && data["Events"].is_array())
    {
        for (const auto& eventData : data["Events"])
        {
            const sf::FloatRect bounds = readRect(eventData.value("Bounds", nlohmann::json::array()));
            if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
            {
                continue;
            }

            levelEventZones.push_back({
                localizeDataText(eventData.value("Title", std::string{"World event"})),
                localizeDataText(eventData.value("Body", std::string{"Something shifts in this place."})),
                bounds,
                parseNotificationTone(eventData.value("Tone", std::string{"Info"})),
                eventData.value("FireOnce", true),
                eventData.value("RequireAlive", true),
                false
            });
        }
    }

    for (const auto& generatedLocation : generatedMiniLocations)
    {
        levelEventZones.push_back({
            localizeDataText(generatedLocation.title),
            localizeDataText("A hidden pocket of the realm opens beyond the veil."),
            sf::FloatRect(
                {generatedLocation.activeLeftX, generatedLocation.roomCeilingY},
                {generatedLocation.activeRightX - generatedLocation.activeLeftX, generatedLocation.roomFloorY - generatedLocation.roomCeilingY + 96.f}
            ),
            NotificationTone::Info,
            true,
            true,
            false
        });
    }
}

void GameLevel::updateLevelEvents()
{
    if (!player)
    {
        return;
    }

    if (introNotificationPending_)
    {
        introNotificationPending_ = false;
        const std::string worldName = levelTitle.empty() ? levelName : levelTitle;
        queueNotification(
            Localization::isRussian() ? Localization::tr("world.entered_realm") : "Entered realm",
            worldName + " " + (Localization::isRussian() ? Localization::tr("world.realm_active") : "is now active."),
            NotificationTone::Info
        );

        if (!weatherThemeTitle_.empty())
        {
            queueNotification(
                Localization::isRussian() ? Localization::tr("world.weather_title") : "Weather omen",
                weatherThemeTitle_ + " " + (Localization::isRussian() ? Localization::tr("world.weather_body") : "shapes the mood of this realm."),
                NotificationTone::Info
            );
        }
    }

    const sf::Vector2f feet = player->getFeetPosition();
    for (auto& zone : levelEventZones)
    {
        if (zone.fireOnce && zone.triggered)
        {
            continue;
        }

        if (zone.requireAlive && (!player->isAlive || player->isPlayingDieAnimation))
        {
            continue;
        }

        if (!zone.bounds.contains(feet))
        {
            continue;
        }

        queueNotification(zone.title, zone.body, zone.tone);
        zone.triggered = true;
    }
}

void GameLevel::onPlayerEnteredLevel()
{
    introNotificationPending_ = true;
    for (auto& zone : levelEventZones)
    {
        zone.triggered = false;
    }
}

void GameLevel::loadLevelData(const LevelDescriptor& descriptor)
{
    std::ifstream dataFile(descriptor.filePath);
    if (!dataFile.good())
    {
        std::cerr << "Error reading level's json data:\n\t" << descriptor.filePath.string() << " not found!\n";
        std::exit(EXIT_FAILURE);
    }

    loadedLevelData = expandMiniLocationContent(nlohmann::json::parse(dataFile));

    levelName = descriptor.id;
    levelTitle = localizeDataText(descriptor.title);
    sourceFileName = descriptor.fileName;
    sourceFilePath = descriptor.filePath;
    playerSpawnPos = {
        loadedLevelData["Presets"]["PlayerSpawn"][0],
        loadedLevelData["Presets"]["PlayerSpawn"][1]
    };
    weatherThemeId_ = loadedLevelData.value("Presets", nlohmann::json::object()).value("WeatherTheme", std::string{});
    weatherThemeTitle_ = humanizeThemeName(weatherThemeId_);

    size = sf::Vector2i(loadedLevelData["Presets"]["Size"][0], loadedLevelData["Presets"]["Size"][1]);
    primaryWorldWidth = loadedLevelData["Presets"].value("MainWorldWidth", size.x);
    primaryWorldWidth = std::clamp(primaryWorldWidth, 0, size.x);
    primaryWorldCameraLeftEdge = 0.f;
    primaryWorldCameraRightEdge = static_cast<float>(primaryWorldWidth);

    initializePlatforms(loadedLevelData);
    initializeDecorations(loadedLevelData);
    initializeBackground(loadedLevelData);
    initializeGround(loadedLevelData);
    if (ground)
    {
        primaryWorldCameraLeftEdge = std::max(0.f, ground->getSurfaceBounds().position.x);
        primaryWorldCameraRightEdge = std::min(
            ground->getCameraClampRight(),
            static_cast<float>(primaryWorldWidth > 0 ? primaryWorldWidth : size.x)
        );
    }

    const bool hasExplicitMiniLocations =
        loadedLevelData.contains("MiniLocations") && loadedLevelData["MiniLocations"].is_array();
    const bool shouldGenerateProceduralMiniLocations = loadedLevelData["Presets"].value(
        "GenerateMiniLocations",
        !hasExplicitMiniLocations
    );

    if (hasExplicitMiniLocations)
    {
        initializeExplicitMiniLocations(loadedLevelData);
    }
    else if (shouldGenerateProceduralMiniLocations)
    {
        generateMiniLocations();
    }
    else
    {
        generatedMiniLocations.clear();
        generatedMiniRewards.clear();
        generatedMiniBarriers.clear();
        generatedMiniHazards.clear();
    }
    initializeWorldHazards(loadedLevelData);

    enemyManager.reset();
    tryInitializeEnemyManager();
    tryInitializeInteractives();
    initializePortals(loadedLevelData);
    initializeEventZones(loadedLevelData);
    initializeSharedDrawOrder(loadedLevelData);
}

void GameLevel::clearLevel()
{
    if (platforms)
    {
        platforms->clearPlatforms();
    }
    if (decorations)
    {
        decorations->clearDecorations();
    }
    if (enemyManager)
    {
        enemyManager->clearEnemies();
        enemyManager.reset();
    }
    interactives.clear();
    generatedMiniLocations.clear();
    generatedMiniRewards.clear();
    generatedMiniBarriers.clear();
    generatedMiniHazards.clear();
    activeMiniLocationId_.reset();
    activeMiniLocationReturnSupport_.reset();
    levelEventZones.clear();
    introNotificationPending_ = false;
    weatherThemeId_.clear();
    weatherThemeTitle_.clear();
    sharedWorldDrawOrder_.clear();
    useSharedWorldDrawOrder_ = false;

    background.clear();
    ground.reset();
    platforms.reset();
    decorations.reset();
}

void GameLevel::saveLevelData()
{
}

void GameLevel::resetTobase()
{
}

void GameLevel::runErrorScreen(std::string errorString)
{
    sf::Text text(*data->gameFont);
    text.setCharacterSize(50u);
    Localization::setText(text, errorString + (Localization::isRussian() ? "\n Нажмите 'Q' для выхода" : "\n Press 'Q' to exit"));
    setTextOriginToMiddle(text);
    text.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2});

    while (true)
    {
        while (const auto ev = this->window->pollEvent())
        {
            if (const auto* keyPressed = ev->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Q)
                {
                    std::exit(0);
                }
            }
        }

        window->clear(sf::Color::Red);
        window->draw(text);
        window->display();
    }
}

sf::Vector2i GameLevel::getLevelSize() const
{
    return size;
}

sf::FloatRect GameLevel::getCameraBoundsForPosition(const sf::Vector2f& position) const
{
    if (const GeneratedMiniLocation* generatedLocation = activeMiniLocation())
    {
        return generatedLocation->cameraBounds;
    }

    (void)position;

    const float mainWorldRight = std::max(
        primaryWorldCameraRightEdge > 0.f ? primaryWorldCameraRightEdge : static_cast<float>(primaryWorldWidth),
        0.f
    );
    const float levelHeight = static_cast<float>(std::max(size.y, 1));
    const float levelTop = std::min(0.f, static_cast<float>(WINDOW_HEIGHT) - levelHeight);
    const float mainWorldLeft = std::clamp(primaryWorldCameraLeftEdge, 0.f, mainWorldRight);
    return sf::FloatRect({mainWorldLeft, levelTop}, {std::max(1.f, mainWorldRight - mainWorldLeft), levelHeight});
}

sf::FloatRect GameLevel::getWorldObjectCameraBoundsForPosition(const sf::Vector2f& position) const
{
    if (const GeneratedMiniLocation* generatedLocation = activeMiniLocation())
    {
        const float margin = 96.f;
        const sf::FloatRect activeBounds{
            {generatedLocation->cameraBounds.position.x - margin, generatedLocation->cameraBounds.position.y - margin},
            {generatedLocation->cameraBounds.size.x + margin * 2.f, generatedLocation->cameraBounds.size.y + margin * 2.f}
        };
        if (activeBounds.contains(position))
        {
            return generatedLocation->cameraBounds;
        }
    }

    const float mainWorldRight = std::max(
        primaryWorldCameraRightEdge > 0.f ? primaryWorldCameraRightEdge : static_cast<float>(primaryWorldWidth),
        0.f
    );
    const float levelHeight = static_cast<float>(std::max(size.y, 1));
    const float levelTop = std::min(0.f, static_cast<float>(WINDOW_HEIGHT) - levelHeight);
    const float mainWorldLeft = std::clamp(primaryWorldCameraLeftEdge, 0.f, mainWorldRight);
    return sf::FloatRect({mainWorldLeft, levelTop}, {std::max(1.f, mainWorldRight - mainWorldLeft), levelHeight});
}

bool GameLevel::isMiniLocationActive() const
{
    return activeMiniLocationId_.has_value();
}

bool GameLevel::enterMiniLocation(const std::string& id, std::optional<sf::Vector2f> returnSupportPoint)
{
    for (const auto& generatedLocation : generatedMiniLocations)
    {
        if (generatedLocation.id == id || generatedLocation.title == id)
        {
            activeMiniLocationId_ = generatedLocation.id.empty() ? generatedLocation.title : generatedLocation.id;
            if (returnSupportPoint.has_value())
            {
                activeMiniLocationReturnSupport_ = *returnSupportPoint;
            }
            return true;
        }
    }
    return false;
}

void GameLevel::exitMiniLocation()
{
    activeMiniLocationId_.reset();
    activeMiniLocationReturnSupport_.reset();
}

sf::Vector2f GameLevel::exitMiniLocation(const sf::Vector2f& fallbackReturnSupportPoint)
{
    const sf::Vector2f returnSupportPoint = activeMiniLocationReturnSupport_.value_or(fallbackReturnSupportPoint);
    exitMiniLocation();
    return returnSupportPoint;
}

std::vector<std::shared_ptr<sf::RectangleShape>>& GameLevel::getPlatformRects()
{
    return platforms->getRects();
}

Platform& GameLevel::getPlatformSystem()
{
    return *platforms;
}

sf::RectangleShape& GameLevel::getGroundRect()
{
    return ground->getRect();
}

sf::Vector2f GameLevel::getPlayerSpawnPos()
{
    return playerSpawnPos;
}

sf::Vector2f GameLevel::getTraderPosition() const
{
    const nlohmann::json presets = loadedLevelData.value("Presets", nlohmann::json::object());
    return readVector2f(
        presets.value("TraderPosition", nlohmann::json::array()),
        {800.f, 940.f}
    );
}

sf::Sprite& GameLevel::getLevelBackgroundSprite()
{
    return background.begin()->get()->getSprite();
}

bool GameLevel::handleEvent(const sf::Event& event)
{
    for (auto& interactive : interactives)
    {
        if (interactive->blocksPlayerInput())
        {
            interactive->handleEvent(event);
            return true;
        }
    }

    for (auto& interactive : interactives)
    {
        if (interactive->handleEvent(event))
        {
            return true;
        }
    }

    return false;
}

bool GameLevel::hasBlockingInteractiveModal() const
{
    return std::any_of(interactives.begin(), interactives.end(), [](const std::unique_ptr<InteractiveObject>& interactive) {
        return interactive && interactive->blocksPlayerInput();
    });
}

void GameLevel::setPlayerSpawnPos(const sf::Vector2f& pos)
{
    playerSpawnPos = pos;
}

LevelDescriptor GameLevel::getLevelDescriptor() const
{
    return LevelDescriptor{
        levelName,
        levelTitle,
        sourceFileName,
        sourceFilePath,
        loadedLevelData.value("Presets", nlohmann::json::object()).value("MenuOrder", 1000),
        loadedLevelData.value("Presets", nlohmann::json::object()).value("isAvaiable", true)
    };
}

void GameLevel::attachPlayer(Player& p)
{
    player = &p;

    if (enemyManager)
    {
        enemyManager->attachPlayer(p);
    }
    else
    {
        tryInitializeEnemyManager();
    }

    tryInitializeInteractives();
    if (player && !loadedLevelData.is_null())
    {
        initializePortals(loadedLevelData);
        initializeSharedDrawOrder(loadedLevelData);
    }
}
