#ifndef GUI_FORMLAYOUT_HPP
#define GUI_FORMLAYOUT_HPP

#include "sfw/Layout.hpp"

namespace sfw
{

/**
 * Horizontally stacked layout with a label before each widget
 */
class FormLayout: public Layout
{
public:
    FormLayout();

    /**
     * Add a new label/widget row in the form
     * @param label: label displayed before the widget
     * @param widget: widget to be added
     */
    Widget* addRow(const sf::String& label, Widget* widget);

protected:
    void recomputeGeometry() override;
    void onThemeChanged() override;

    float m_labelWidth;
};

} // namespace

#endif // GUI_FORMLAYOUT_HPP
