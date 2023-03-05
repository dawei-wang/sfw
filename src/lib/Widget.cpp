#include "sfw/Widget.hpp"
#include "sfw/Layout.hpp"
#include "sfw/GUI-main.hpp"

#include <cassert>
#include <cmath>

#ifdef DEBUG
#include <SFML/Graphics.hpp>
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


bool Widget::isRoot()
{
    return getParent() == nullptr || getParent() == this;
}


bool Widget::isMain()
{
#ifndef NDEBUG
    if (getParent() == this) assert(isRoot());
#endif
    return getParent() == this;
}


Widget* Widget::getRoot()
{
    return !isMain() && getParent() ? getParent()->getRoot() : this;
}


GUI* Widget::getMain()
{
    // Well, it's halfway between "undefined" and "bug" to call this on free-standing widgets...
    assert(       getParent() && getParent() != this ? getParent()->getRoot() : getParent());
    return (GUI*)(getParent() && getParent() != this ? getParent()->getRoot() : getParent());
}


Widget* Widget::find(const std::string& name)
{
    if (GUI* Main = getMain(); Main != nullptr)
    {
        return Main->recall(name);
    }
    return nullptr;
}


void Widget::setPosition(const sf::Vector2f& pos)
{
    m_position = pos;
    m_transform = sf::Transform(
        1, 0, pos.x, // translate x
        0, 1, pos.y, // translate y
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
    for (Widget* parent = m_parent; parent != nullptr; parent = parent->m_parent)
    {
        position.x += parent->m_position.x;
        position.y += parent->m_position.y;

        //! Must also check for isMain() now to avoid infinite looping on parent->parent == parent!
        //! And we can't just add this to the for cond. either, as that would skip the last offset of the Main obj itself... :-/
        if (parent->isMain()) break;
    }
    return position;
}


void Widget::setSize(const sf::Vector2f& size)
{
    m_size = size;
    onResized();
    if (!isRoot())
    {
        getParent()->recomputeGeometry();
    }
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


Widget* Widget::setCallback(std::function<void()> callback)
{
    m_callback = callback;
    return this;
}

Widget* Widget::setCallback(std::function<void(Widget*)> callback)
{
    m_callback = [this, callback]{ return callback(this); };
    return this;
}

void Widget::triggerCallback()
{
    if (m_callback)
    {
        m_callback();
    }
}


void Widget::setParent(Layout* parent)
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


void Widget::centerText(sf::Text& text)
{
    sf::FloatRect r = text.getLocalBounds();
    text.setOrigin({r.left + std::round(r.width / 2.f), r.top + std::round(r.height / 2.f)});
    text.setPosition({m_size.x / 2, m_size.y / 2});
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
void Widget::draw_outline([[maybe_unused]] const gfx::RenderContext& ctx) const
{
	sf::RectangleShape r(sf::Vector2f(getSize().x, getSize().y));
	r.setPosition(getAbsolutePosition());
	r.setFillColor(sf::Color::Transparent);
	r.setOutlineThickness(2);
	r.setOutlineColor(sf::Color::Red);
	ctx.target.draw(r);
}
#endif

} // namespace
