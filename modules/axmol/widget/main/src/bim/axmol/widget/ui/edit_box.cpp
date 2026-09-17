// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/edit_box.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/factory/edit_box.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <bim/axmol/input/observer/tap_observer.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>

#include <iscool/signals/implement_signal.hpp>

#include <axmol/ui/UIEditBox/UIEditBox.h>

#define x_widget_scope bim::axmol::widget::edit_box::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::ui::EditBox, input)
#include <bim/axmol/widget/implement_controls_struct.hpp>

class bim::axmol::widget::edit_box::Delegate final
  : public ax::ui::EditBoxDelegate
{
public:
  explicit Delegate(edit_box& e)
    : m_edit_box(e)
  {}

  void editBoxReturn(ax::ui::EditBox*) override
  {}

  void editBoxEditingDidEndWithAction(
      ax::ui::EditBox*,
      ax::ui::EditBoxDelegate::EditBoxEndAction action) override
  {
    if (action == ax::ui::EditBoxDelegate::EditBoxEndAction::RETURN)
      m_edit_box.m_changed();
    else
      m_edit_box.m_cancel();
  }

private:
  edit_box& m_edit_box;
};

IMPLEMENT_SIGNAL(bim::axmol::widget::edit_box, changed, m_changed);
IMPLEMENT_SIGNAL(bim::axmol::widget::edit_box, cancel, m_cancel);

bim_implement_widget(bim::axmol::widget::edit_box);

bim::axmol::widget::edit_box::edit_box(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_tap_observer(*this)
  , m_style_bounds(style.get_declaration_or_empty("bounds"))
  , m_delegate(new Delegate(*this))
  , m_bounds_dirty(true)
{
  m_tap_observer->connect_to_release(
      [this]()
        {
          m_controls->input->openKeyboard();
        });

  m_inputs.attach_to_root(m_tap_observer);
  m_controls->input->setDelegate(m_delegate.get());
}

bim::axmol::widget::edit_box::~edit_box() = default;

bim::axmol::input::node_reference
bim::axmol::widget::edit_box::input_node() const
{
  return m_inputs.root();
}

void bim::axmol::widget::edit_box::enable(bool enabled)
{
  if (m_tap_observer->is_enabled() == enabled)
    return;

  m_tap_observer->enable(enabled);
}

void bim::axmol::widget::edit_box::set_text(const char* text)
{
  m_controls->input->setText(text);
}

std::string_view bim::axmol::widget::edit_box::get_text() const
{
  return m_controls->input->getText();
}

void bim::axmol::widget::edit_box::max_length(int n)
{
  m_controls->input->setMaxLength(n);
}

void bim::axmol::widget::edit_box::onEnter()
{
  ax::Node::onEnter();

  if (m_bounds_dirty)
    update_bounds();
}

void bim::axmol::widget::edit_box::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);

  update_bounds();
}

bool bim::axmol::widget::edit_box::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeOpacityEnabled(true);

  add_group_as_children(*this, m_controls->all_nodes);

  return true;
}

void bim::axmol::widget::edit_box::update_bounds()
{
  if (!isRunning())
    {
      m_bounds_dirty = true;
      return;
    }

  apply_bounds(m_context, m_controls->all_nodes, m_style_bounds);
  m_bounds_dirty = false;
}
