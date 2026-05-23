#include <AskDialogue.h>
#include <Localization.h>

#include <utility>

AskDialogue::AskDialogue(sf::Vector2f pos, sf::Vector2f size, std::string text, sf::RenderWindow& window)
    : onYesClick([]() {})
    , onNoClick([]() {})
    , desiredSize_(size)
{
    window_m = &window;
    gui.setWindow(window);

    yesButton = tgui::Button::create();
    yesButton->onClick(onYesClick);
    yesButton->setSize(size.x / 5, size.y / 3);
    yesButton->setOrigin(0.5, 0.5);
    yesButton->setText(Localization::isRussian() ? Localization::tr("dialog.yes") : "Yes");
    yesButton->setTextSize(characterSize);
    yesButton->setWidgetName("yesButton");
    yesButton->getRenderer()->setBorders({2.f});
    yesButton->getRenderer()->setRoundedBorderRadius(8.f);
    yesButton->getRenderer()->setBackgroundColor(BASE_YES_IDLE_COLOR);
    yesButton->getRenderer()->setBackgroundColorHover(BASE_YES_HOVER_COLOR);
    yesButton->getRenderer()->setBackgroundColorDown(BASE_YES_CLICK_COLOR);
    yesButton->getRenderer()->setBorderColor(sf::Color(179, 132, 84));
    yesButton->getRenderer()->setBorderColorHover(sf::Color(219, 176, 120));
    yesButton->getRenderer()->setBorderColorDown(sf::Color(219, 176, 120));
    yesButton->getRenderer()->setTextColor(BASE_YES_TEXT_IDLE_COLOR);
    yesButton->getRenderer()->setTextColorDown(BASE_YES_TEXT_CLICK_COLOR);
    yesButton->getRenderer()->setTextColorHover(BASE_YES_TEXT_HOVER_COLOR);

    noButton = tgui::Button::create();
    noButton->onClick(onNoClick);
    noButton->setSize(size.x / 5, size.y / 3);
    noButton->setOrigin(0.5, 0.5);
    noButton->setText(Localization::isRussian() ? Localization::tr("dialog.no") : "No");
    noButton->setTextSize(characterSize);
    noButton->setWidgetName("noButton");
    noButton->getRenderer()->setBorders({2.f});
    noButton->getRenderer()->setRoundedBorderRadius(8.f);
    noButton->getRenderer()->setBackgroundColor(BASE_NO_IDLE_COLOR);
    noButton->getRenderer()->setBackgroundColorHover(BASE_NO_HOVER_COLOR);
    noButton->getRenderer()->setBackgroundColorDown(BASE_NO_CLICK_COLOR);
    noButton->getRenderer()->setBorderColor(sf::Color(176, 74, 73));
    noButton->getRenderer()->setBorderColorHover(sf::Color(219, 112, 108));
    noButton->getRenderer()->setBorderColorDown(sf::Color(219, 112, 108));
    noButton->getRenderer()->setTextColor(BASE_NO_TEXT_IDLE_COLOR);
    noButton->getRenderer()->setTextColorDown(BASE_NO_TEXT_CLICK_COLOR);
    noButton->getRenderer()->setTextColorHover(BASE_NO_TEXT_HOVER_COLOR);

    label = tgui::Label::create();
    label->setTextSize(characterSize);
    label->setText(text);
    label->setOrigin(0.5, 0.5);
    label->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
    label->getRenderer()->setTextColor(BASE_LABEL_TEXT_COLOR);
    label->getRenderer()->setTextOutlineColor(sf::Color(0, 0, 0, 180));
    label->getRenderer()->setTextOutlineThickness(1.f);

    gui.add(yesButton);
    gui.add(noButton);
    gui.add(label);

    mainRect_m.setSize(size);
    setRectangleOriginToMiddle(mainRect_m);
    mainRect_m.setFillColor(BASE_ASKDIALOGUE_BACKGROUND_COLOR);
    mainRect_m.setOutlineThickness(3.f);
    mainRect_m.setOutlineColor(sf::Color(146, 109, 80, 230));

    refreshLayout();
}

AskDialogue::~AskDialogue() = default;

void AskDialogue::draw(sf::RenderWindow& window)
{
    if (!isCalled)
    {
        return;
    }

    refreshLayout();
    window.draw(mainRect_m);
    gui.draw();
}

void AskDialogue::handleEvent(const sf::Event& ev)
{
    if (!isCalled)
    {
        return;
    }

    gui.handleEvent(ev);
}

void AskDialogue::open()
{
    isCalled = true;
}

void AskDialogue::close()
{
    isCalled = false;
}

bool AskDialogue::isOpen() const
{
    return isCalled;
}

void AskDialogue::attachWindow(sf::RenderWindow& window)
{
    window_m = &window;
    gui.setWindow(window);
    refreshLayout();
}

void AskDialogue::setOnYesClick(std::function<void()> fnc)
{
    onYesClick = std::move(fnc);
    yesButton->onClick(onYesClick);
}

void AskDialogue::setOnNoClick(std::function<void()> fnc)
{
    onNoClick = std::move(fnc);
    noButton->onClick(onNoClick);
}

void AskDialogue::connectTGUIFont(tgui::Font& font)
{
    gui.setFont(font);
}

void AskDialogue::refreshLayout()
{
    if (!window_m)
    {
        return;
    }

    const sf::View& defaultView = window_m->getDefaultView();
    const sf::Vector2f center = defaultView.getCenter();

    mainRect_m.setSize(desiredSize_);
    setRectangleOriginToMiddle(mainRect_m);
    mainRect_m.setPosition(center);

    const sf::Vector2f buttonSize = {desiredSize_.x / 5.f, desiredSize_.y / 3.f};
    yesButton->setSize(buttonSize.x, buttonSize.y);
    noButton->setSize(buttonSize.x, buttonSize.y);
    yesButton->setPosition(center.x - desiredSize_.x * 0.24f, center.y + desiredSize_.y * 0.22f);
    noButton->setPosition(center.x + desiredSize_.x * 0.24f, center.y + desiredSize_.y * 0.22f);

    label->setPosition(center.x, center.y - desiredSize_.y * 0.16f);
}
