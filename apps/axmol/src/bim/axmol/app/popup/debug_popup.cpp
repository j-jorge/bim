// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/debug_popup.hpp>

#include <bim/axmol/app/popup/popup.hpp>
#include <bim/axmol/app/preference/feature_flags.hpp>

#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/implement_widget.hpp>
#include <bim/axmol/widget/ui/button.hpp>
#include <bim/axmol/widget/ui/list.hpp>
#include <bim/axmol/widget/ui/passive_node.hpp>
#include <bim/axmol/widget/ui/toggle.hpp>

#define x_widget_scope bim::axmol::app::debug_popup::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, close_button)                          \
      x_widget(bim::axmol::widget::list, list)

#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <bim/game/feature_flags.hpp>

#include <iscool/preferences/local_preferences.hpp>
#include <iscool/system/language_code.hpp>

#include <axmol/2d/Label.h>

namespace
{
  struct label_controls;

#define x_widget_scope
#define x_widget_type_name label_controls
#define x_widget_controls x_widget(ax::Label, label)

#include <bim/axmol/widget/implement_controls_struct.hpp>

  struct text_controls;

#define x_widget_scope
#define x_widget_type_name text_controls
#define x_widget_controls x_widget(ax::Label, label) x_widget(ax::Label, value)

#include <bim/axmol/widget/implement_controls_struct.hpp>

  struct toggle_controls;

#define x_widget_scope
#define x_widget_type_name toggle_controls
#define x_widget_controls                                                     \
  x_widget(ax::Label, label) x_widget(bim::axmol::widget::toggle, toggle)

#include <bim/axmol/widget/implement_controls_struct.hpp>
}

bim::axmol::app::debug_popup::debug_popup(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_list_item_container_style(*style.get_declaration("list-item"))
  , m_title_item_controls(*style.get_declaration("title-item-controls"))
  , m_title_item_bounds(*style.get_declaration("title-item-bounds"))
  , m_text_item_controls(*style.get_declaration("text-item-controls"))
  , m_text_item_bounds(*style.get_declaration("text-item-bounds"))
  , m_toggle_item_controls(*style.get_declaration("toggle-item-controls"))
  , m_toggle_item_bounds(*style.get_declaration("toggle-item-bounds"))
  , m_popup(new popup(context, *style.get_declaration("popup")))
{
  m_controls->close_button->connect_to_clicked(
      [this]()
      {
        m_popup->hide();
      });
}

bim::axmol::app::debug_popup::~debug_popup() = default;

void bim::axmol::app::debug_popup::show()
{
  m_controls->list->clear();

  m_inputs.clear();
  m_inputs.push_back(m_controls->close_button->input_node());
  m_inputs.push_back(m_controls->list->input_node());

  add_title("FEATURES");
  add_feature_item("Falling blocks", bim::game::feature_flags::falling_blocks);

  add_title("SYSTEM");
  add_text_item("Language", iscool::system::get_language_code());

  m_popup->show(m_controls->all_nodes, m_style_bounds, m_inputs.root());
}

void bim::axmol::app::debug_popup::add_feature_item(
    std::string_view label, bim::game::feature_flags flag)
{
  const bool enabled = !!(bim::axmol::app::enabled_feature_flags(
                              *m_context.get_local_preferences())
                          & flag);

  auto toggle_flag = [this, flag]() -> bool
  {
    const bim::game::feature_flags new_flags =
        bim::axmol::app::enabled_feature_flags(
            *m_context.get_local_preferences())
        ^ flag;
    bim::axmol::app::enabled_feature_flags(*m_context.get_local_preferences(),
                                           new_flags);
    return !!(new_flags & flag);
  };

  add_toggle_item(label, enabled, toggle_flag);
}

void bim::axmol::app::debug_popup::add_title(std::string_view label)
{
  label_controls controls(m_context.get_widget_context(),
                          m_title_item_controls);
  controls.label->setString(label);

  add_item(controls.all_nodes, m_title_item_bounds);
}

void bim::axmol::app::debug_popup::add_text_item(std::string_view label,
                                                 std::string_view value)
{
  text_controls controls(m_context.get_widget_context(), m_text_item_controls);
  controls.label->setString(label);
  controls.value->setString(value);

  add_item(controls.all_nodes, m_text_item_bounds);
}

void bim::axmol::app::debug_popup::add_toggle_item(
    std::string_view label, bool state, std::function<bool()> do_toggle)
{
  toggle_controls controls(m_context.get_widget_context(),
                           m_toggle_item_controls);
  controls.label->setString(label);

  bim::axmol::widget::toggle& t = *controls.toggle;

  m_inputs.push_back(t.input_node());

  t.set_state(state);
  t.connect_to_clicked(
      [&t, do_toggle = std::move(do_toggle)]() -> void
      {
        t.set_state(do_toggle());
      });

  add_item(controls.all_nodes, m_toggle_item_bounds);
}

void bim::axmol::app::debug_popup::add_item(
    const bim::axmol::widget::named_node_group& nodes,
    const iscool::style::declaration& bounds)
{
  bim::axmol::ref_ptr<bim::axmol::widget::passive_node> item =
      bim::axmol::widget::factory<bim::axmol::widget::passive_node>::create(
          m_context.get_widget_context(), m_list_item_container_style);

  item->fill(nodes, bounds);
  m_controls->list->push_back(*item);
}
