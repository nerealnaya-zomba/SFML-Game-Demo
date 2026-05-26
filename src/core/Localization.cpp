#include "Localization.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>

namespace
{
Language currentLanguage = Language::English;

std::string normalize(std::string_view value)
{
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

const std::unordered_map<std::string_view, std::string>& russianStrings()
{
    static const std::unordered_map<std::string_view, std::string> strings = {
        {"menu.start", "Начать"},
        {"menu.resume", "Продолжить"},
        {"menu.main_menu", "Главное меню"},
        {"menu.controls", "Управление"},
        {"menu.exit", "Выход"},
        {"menu.settings", "Настройки"},
        {"menu.paused", "ПАУЗА"},
        {"menu.game_paused", "Игра на паузе"},
        {"menu.no_levels", "Игровые уровни не найдены"},
        {"menu.enter_first_gate", "Войти в первые врата"},
        {"menu.current_level", "Текущий уровень"},
        {"menu.starting_level", "Стартовый уровень"},
        {"menu.none", "Нет"},
        {"menu.no_levels_available", "Нет доступных уровней"},
        {"settings.title", "Настройки"},
        {"settings.description", "Настройки меню, языка и видео."},
        {"settings.fullscreen_on", "Полный экран: ВКЛ"},
        {"settings.fullscreen_off", "Полный экран: ВЫКЛ"},
        {"settings.vsync_on", "VSync: ВКЛ"},
        {"settings.vsync_off", "VSync: ВЫКЛ"},
        {"settings.ash_density", "Плотность пепла"},
        {"settings.low", "Низкая"},
        {"settings.medium", "Средняя"},
        {"settings.high", "Высокая"},
        {"settings.language", "Язык"},
        {"settings.reset_progress", "Сбросить прогресс"},
        {"settings.close", "Закрыть"},
        {"settings.saved", "Настройки сохранены"},
        {"settings.error", "Ошибка настроек"},
        {"settings.fullscreen_failed", "Не удалось применить полноэкранный режим."},
        {"settings.fullscreen_enabled", "Полноэкранный режим включен."},
        {"settings.fullscreen_disabled", "Полноэкранный режим выключен."},
        {"settings.vsync_enabled", "VSync включен."},
        {"settings.vsync_disabled", "VSync выключен."},
        {"settings.ash_saved", "Плотность пепла в меню"},
        {"settings.language_saved", "Язык изменен."},
        {"dialog.yes", "Да"},
        {"dialog.no", "Нет"},
        {"dialog.quit", "Выйти из игры?"},
        {"dialog.reset_progress", "Сбросить весь прогресс?\nЗолото, реликвии, оружие и открытые врата будут потеряны."},
        {"controls.title", "Управление"},
        {"controls.body",
            "Движение\n"
            "Left / Right - Движение\n"
            "Z - Прыжок\n"
            "C - Рывок\n\n"
            "Бой\n"
            "X - Выстрел\n"
            "R - Открыть портал\n"
            "E - Меню направлений\n"
            "Q / W - Сменить направление\n"
            "Enter - Подтвердить\n\n"
            "Меню\n"
            "Escape - Пауза\n"
            "F1 - Консоль ритуала"},
        {"death.title", "ВЫ ПОГИБЛИ"},
        {"death.description", "Выберите, что дальше"},
        {"death.restart", "Начать уровень заново"},
        {"death.main_menu", "Вернуться в главное меню"},
        {"death.hint", "Стрелки / W S - выбрать    Enter - подтвердить"},
        {"hud.health", "ЗДОРОВЬЕ"},
        {"hud.energy", "ЭНЕРГИЯ"},
        {"hud.stats_title", "Статы ковенанта"},
        {"hud.stats_hint", "Нажмите P, чтобы открыть статы"},
        {"hud.objective_hint", "Нажмите O, чтобы открыть цель"},
        {"hud.objective_updated", "Цель обновлена"},
        {"hud.inventory", "Инвентарь"},
        {"hud.inventory_hint", "Нажмите I, чтобы открыть инвентарь"},
        {"hud.no_relics", "Реликвий пока нет"},
        {"hud.sanctuary", "Святилище"},
        {"hud.energy_gain", "Реген. энергии"},
        {"hud.damage", "Урон"},
        {"hud.shot_cd", "КД выстрела"},
        {"hud.shot_cost", "Цена выстрела"},
        {"hud.bolt_speed", "Скорость заряда"},
        {"hud.range", "Дальность"},
        {"hud.run_speed", "Скорость бега"},
        {"hud.dash_force", "Сила рывка"},
        {"hud.dash_cd", "КД рывка"},
        {"hud.jump_power", "Сила прыжка"},
        {"hud.air_jumps", "Прыжки в воздухе"},
        {"hud.slow_fall", "Замедл. падения"},
        {"shop.arsenal", "Арсенал ритуалов"},
        {"shop.stock", "Товары торговца"},
        {"shop.gold", "Золото"},
        {"shop.next_route", "Следующий отмеченный путь"},
        {"shop.relics", "Реликвии"},
        {"shop.weapons", "Оружие"},
        {"shop.tabs", "Q/W - Вкладки"},
        {"shop.common_relic", "Обычная реликвия"},
        {"shop.rare_relic", "Редкая реликвия"},
        {"shop.myth_relic", "Мифическая реликвия"},
        {"shop.legendary_relic", "Легендарная реликвия"},
        {"shop.relic", "Реликвия"},
        {"shop.relic_upgrade", "Улучшение-реликвия"},
        {"shop.weapon_rite", "Оружейный ритуал"},
        {"shop.form", "Форма"},
        {"shop.cooldown", "Перезарядка"},
        {"shop.energy_cost", "Цена энергии"},
        {"shop.shot_speed", "Скорость выстрела"},
        {"shop.projectiles", "Снаряды"},
        {"shop.pierce", "Пробитие"},
        {"shop.splash", "Взрыв"},
        {"shop.fan_spread", "Веер"},
        {"shop.health", "Здоровье"},
        {"shop.damage", "Урон"},
        {"shop.bullet_speed", "Скорость заряда"},
        {"shop.bullet_range", "Дальность заряда"},
        {"shop.agility", "Подвижность"},
        {"shop.top_speed", "Макс. скорость"},
        {"shop.shoot_cooldown", "КД выстрела"},
        {"shop.dash_force", "Сила рывка"},
        {"shop.dash_cooldown", "КД рывка"},
        {"shop.extra_jump", "Доп. прыжок"},
        {"shop.jump_height", "Высота прыжка"},
        {"shop.fall_speed", "Скорость падения"},
        {"shop.no_bonus", "Любопытная безделушка без прямого бонуса к статам."},
        {"shop.price", "Цена"},
        {"shop.already_purchased", "Уже куплено"},
        {"shop.enough_gold", "Золота достаточно"},
        {"shop.not_enough_gold", "Недостаточно золота"},
        {"shop.close_hint", "X - Закрыть"},
        {"shop.buy_hint", "Z - Купить    X - Закрыть"},
        {"shop.buy_equip_hint", "Z - Купить и взять    X - Закрыть"},
        {"destination.title", "ВЫБОР НАПРАВЛЕНИЯ"},
        {"destination.subtitle", "Следуйте меткам торговца, открывайте пути и доберитесь до Темных врат."},
        {"destination.default_description", "Выберите, где красные врата ответят на ваш зов."},
        {"destination.none_title", "Направления не открыты"},
        {"destination.none_state", "Вратам некуда ответить."},
        {"destination.none_body", "Выполняйте задания торговца, и здесь появятся новые пути."},
        {"destination.none_legend", "Откройте путь в мире, прежде чем привязать к нему портал."},
        {"destination.custom_route", "Пользовательский путь из редактора уровней."},
        {"destination.custom_focus", "Лучше всего для: проверки лэйаутов, столкновений и карманных локаций."},
        {"destination.sealed", "Запечатанный путь - ковенант пока отвергает эти врата"},
        {"destination.current_bound", "Текущая земля - портал уже привязан"},
        {"destination.current", "Текущая земля"},
        {"destination.bound", "Портал привязан к этому месту"},
        {"destination.unbound", "Непривязанное направление"},
        {"destination.legend", "Пепельная метка: текущая земля   |   Огненная метка: привязка портала   |   Запечатанные карты: закрыто сюжетом"},
        {"destination.cycle", "Листать пути"},
        {"destination.attune", "Привязать портал"},
        {"destination.close", "Закрыть меню"},
        {"destination.level", "Уровень"},
        {"world.entered_realm", "Вход в область"},
        {"world.realm_active", "активна."},
        {"world.weather_title", "Погода области"},
        {"world.weather_body", "задает настроение этой области."},
        {"world.hidden_revealed", "Тайник найден"},
        {"world.hidden_body", "Скрытый объект проявился рядом."},
        {"world.enter_interact", "Enter - взаимодействовать"},
        {"world.panel_close", "Enter или Esc - закрыть"},
        {"world.forgotten_relic", "Забытая реликвия"},
        {"world.dead_trace", "Мертвые оставили здесь след."},
        {"world.enter_portal", "Enter - войти в портал"},
        {"recovery.lost_gold", "Потеряно золота"},
        {"recovery.reclaim", "F - Забрать"},
        {"campaign.unknown_title", "Неназванная земля"},
        {"campaign.unknown_flavor", "Путь, которому ковенант еще не дал имени."},
        {"campaign.unknown_description", "Ни один летописец еще не записал, что там ждет."},
        {"campaign.unknown_focus", "Лучше всего для: неизвестной добычи."},
        {"campaign.unknown_unlock", "Его печать пока не разгадана."}
    };
    return strings;
}
}

void Localization::setLanguage(const Language language)
{
    currentLanguage = language;
}

Language Localization::getLanguage()
{
    return currentLanguage;
}

Language Localization::languageFromString(std::string_view value)
{
    const std::string normalized = normalize(value);
    if (normalized == "ru" || normalized == "rus" || normalized == "russian")
    {
        return Language::Russian;
    }
    return Language::English;
}

std::string Localization::languageToString(const Language language)
{
    return language == Language::Russian ? "ru" : "en";
}

std::string Localization::languageDisplayName(const Language language)
{
    return language == Language::Russian ? "Русский" : "English";
}

Language Localization::nextLanguage(const Language language)
{
    return language == Language::Russian ? Language::English : Language::Russian;
}

bool Localization::isRussian()
{
    return currentLanguage == Language::Russian;
}

std::string Localization::tr(std::string_view key)
{
    if (currentLanguage == Language::Russian)
    {
        const auto& strings = russianStrings();
        const auto it = strings.find(key);
        if (it != strings.end())
        {
            return it->second;
        }
    }

    return std::string(key);
}

std::string Localization::weaponName(std::string_view value)
{
    if (!isRussian())
    {
        return std::string(value);
    }

    const std::string raw(value);
    static const std::unordered_map<std::string_view, std::string> names = {
        {"AshenBolt", "Пепельный заряд"},
        {"Ashen Bolt", "Пепельный заряд"},
        {"Gravepiercer", "Пика Могилобоя"},
        {"Gravepiercer Pike", "Пика Могилобоя"},
        {"PyreOrb", "Скипетр Пиросферы"},
        {"Pyre Orb", "Скипетр Пиросферы"},
        {"Pyre Orb Scepter", "Скипетр Пиросферы"},
        {"StormNeedler", "Штормовой игольник"},
        {"Storm Needler", "Штормовой игольник"},
        {"DreadPrism", "Призма ужаса"},
        {"Dread Prism", "Призма ужаса"},
        {"NightfallBeam", "Луч сумерек"},
        {"Nightfall Beam", "Луч сумерек"},
        {"Voodoo Doll", "Кукла вуду"},
        {"Pyre Book", "Пламенная книга"},
        {"Night Eye", "Ночной глаз"},
        {"Grave Crown", "Могильная корона"},
        {"Storm Cap", "Штормовой колпак"}
    };

    const auto it = names.find(raw);
    return it != names.end() ? it->second : raw;
}

sf::String Localization::toSfString(std::string_view utf8Text)
{
    return sf::String::fromUtf8(utf8Text.begin(), utf8Text.end());
}

void Localization::setText(sf::Text& text, std::string_view utf8Text)
{
    text.setString(toSfString(utf8Text));
}
