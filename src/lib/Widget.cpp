#include "sfw/Widget.hpp"
#include "sfw/WidgetContainer.hpp"
#include "sfw/GUI-main.hpp"
#include "sfw/util/diagnostics.hpp"

#include <cassert>
#include <cmath>

#ifdef DEBUG
#
#  include <SFML/Graphics.hpp>
#
#endif

namespace sfw
{

Widget::Widget():
    m_parent(nullptr),
    m_previous(nullptr),
    m_next(nullptr),
    m_state(WidgetState::Default),
    m_selectable(true)
{
}


bool Widget::isRoot() const
{
    return getParent() == nullptr || getParent() == this;
}


bool Widget::isMain() const
{
#ifndef NDEBUG
    if (getParent() == this) assert(isRoot()); // not there in my MSVC yet: [[ assert: isRoot() ]]
#endif
    return getParent() == this;
}


Widget* Widget::getRoot() const
{
    return !isMain() && getParent() ? getParent()->getRoot() : const_cast<Widget*>(this); // Not const when returning itself...
}


GUI* Widget::getMain() const
{
    //!! Well, it's halfway between "undefined" and "bug" to call this on free-standing widgets...
    //!! But forbidding it would prevent adding children to them, criplling usability too much!
    //!!   assert(getParent() && getParent() != this ? getParent()->getRoot() : getParent());
    return (GUI*)(getParent() && getParent() != this ? getParent()->getRoot() : getParent());
}


void Widget::setName(const std::string& name)
{
    if (GUI* Main = getMain(); Main != nullptr)
    {
        Main->remember(this, name);
    }
}

std::string Widget::getName(Widget* widget) const
{
    if (GUI* Main = getMain(); Main != nullptr)
    {
        return Main->recall(widget ? widget : this);
    }
    return "";
}


Widget* Widget::getWidget(const std::string& name) const
{
    if (GUI* Main = getMain(); Main)
    {
        return Main->recall(name);
    }
    return nullptr;
}


void Widget::setPosition(const sf::Vector2f& pos)
{
    m_position = {roundf(pos.x), roundf(pos.y)};
    m_transform = sf::Transform(
        1, 0, m_position.x, // translate x
        0, 1, m_position.y, // translate y
        0, 0, 1
    );
}

void Widget::setPosition(float x, float y)
{
    setPosition(sf::Vector2f(x, y));
}


const sf::Vector2f& Widget::getPosition() const
{
    return m_position;
}


sf::Vector2f Widget::getAbsolutePosition() const
{
    sf::Vector2f position = m_position;

    for (const Widget* base = this; !base->isRoot();)
    {
	base = base->getParent();
        position.x += base->m_position.x;
        position.y += base->m_position.y;
    }

    return position;
}


void Widget::setSize(const sf::Vector2f& size)
{
    m_size = sf::Vector2f(roundf(size.x), roundf(size.y));

    onResized();
    if (!isRoot()) getParent()->recomputeGeometry();
}


void Widget::setSize(float width, float height)
{
    setSize(sf::Vector2f(width, height));
}


const sf::Vector2f& Widget::getSize() const
{
    return m_size;
}


bool Widget::containsPoint(const sf::Vector2f& point) const
{
    return point.x > 0.f && point.x < m_size.x && point.y > 0.f && point.y < m_size.y;
}


bool Widget::isSelectable() const
{
    return m_selectable;
}


bool Widget::isFocused() const
{
    return m_state == WidgetState::Focused || m_state == WidgetState::Pressed;
}


void Widget::setSelectable(bool selectable)
{
    m_selectable = selectable;
}


Widget* Widget::setCallback(Callback callback)
{
    m_callback = callback;
    return this;
}

void Widget::triggerCallback()
{
    assert( std::holds_alternative<Callback_w>(m_callback) || std::holds_alternative<Callback_void>(m_callback) );

    if (std::holds_alternative<Callback_w>(m_callback) && std::get<Callback_w>(m_callback))
    {
        return (std::get<Callback_w>(m_callback).value()) (this);
    }
    else if (std::holds_alternative<Callback_void>(m_callback) && std::get<Callback_void>(m_callback))
    {
        return (std::get<Callback_void>(m_callback).value()) ();
    }
}


void Widget::setParent(WidgetContainer* parent)
{
    m_parent = parent;
}


void Widget::setState(WidgetState state)
{
    m_state = state;
    onStateChanged(state);
}


WidgetState Widget::getState() const
{
    return m_state;
}


const sf::Transform& Widget::getTransform() const
{
    return m_transform;
}


// callbacks -----------------------------------------------------------------

void Widget::onStateChanged(WidgetState) { }
void Widget::onMouseEnter() { }
void Widget::onMouseLeave() { }
void Widget::onMouseMoved(float, float) { }
void Widget::onMousePressed(float, float) { }
void Widget::onMouseReleased(float, float) { }
void Widget::onMouseWheelMoved(int) { }
void Widget::onKeyPressed(const sf::Event::KeyEvent&) { }
void Widget::onKeyReleased(const sf::Event::KeyEvent&) { }
void Widget::onTextEntered(uint32_t) { }
void Widget::onThemeChanged() { }
void Widget::onResized() { }


// diagnostics ---------------------------------------------------------------

#ifdef DEBUG
void Widget::draw_outline([[maybe_unused]] const gfx::RenderContext& ctx, sf::Color color) const
{
	sf::RectangleShape r(sf::Vector2f(getSize().x, getSize().y));
	r.setPosition(getAbsolutePosition());
	r.setFillColor(sf::Color::Transparent);
	r.setOutlineThickness(2);
	r.setOutlineColor(color);
	ctx.target.draw(r);
}
#endif

} // namespace
