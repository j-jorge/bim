// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/debug_popup.hpp>

#include <bim/axmol/app/part/wallet.hpp>
#include <bim/axmol/app/popup/popup.hpp>
#include <bim/axmol/app/preference/arena_stats.hpp>
#include <bim/axmol/app/preference/date_of_next_config_update.hpp>
#include <bim/axmol/app/preference/date_of_next_version_update_message.hpp>
#include <bim/axmol/app/preference/feature_flags.hpp>
#include <bim/axmol/app/preference/wallet.hpp>

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
#include <iscool/time/now.hpp>

#include <axmol/2d/Label.h>
#include <axmol/base/Director.h>

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

  struct button_controls;

#define x_widget_scope
#define x_widget_type_name button_controls
#define x_widget_controls                                                     \
  x_widget(ax::Label, label) x_widget(bim::axmol::widget::button, button)

#include <bim/axmol/widget/implement_controls_struct.hpp>
}

bim::axmol::app::debug_popup::debug_popup(
    const context& context, const iscool::style::declaration& style,
    wallet& wallet)
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
  , m_button_item_controls(*style.get_declaration("button-item-controls"))
  , m_button_item_bounds(*style.get_declaration("button-item-bounds"))
  , m_popup(new popup(context, *style.get_declaration("popup")))
  , m_wallet(wallet)
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
  add_feature_item("Fog of war", bim::game::feature_flags::fog_of_war);
  add_feature_item("Invisibility", bim::game::feature_flags::invisibility);

  add_title("WALLET");
  add_button_item("Get 10 coins.",
                  [this]() -> void
                  {
                    coin_transaction(10);
                  });
  add_button_item("Get 100 coins.",
                  [this]() -> void
                  {
                    coin_transaction(100);
                  });
  add_button_item("Lose 100 coins.",
                  [this]() -> void
                  {
                    coin_transaction(-100);
                  });

  add_feature_item("Fog of war", bim::game::feature_flags::fog_of_war);
  add_feature_item("Invisibility", bim::game::feature_flags::invisibility);

  add_title("PREFERENCES");
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  add_text_item("Game count in arena",
                std::to_string(games_in_arena(preferences)));
  add_text_item("Victories in arena",
                std::to_string(victories_in_arena(preferences)));
  add_text_item("Defeats in arena",
                std::to_string(defeats_in_arena(preferences)));

  const std::chrono::hours now = iscool::time::now<std::chrono::hours>();
  {
    const std::chrono::hours d = date_of_next_config_update(preferences);

    add_button_item("Config update in " + std::to_string((d - now).count())
                        + " h.",
                    [&preferences, now]() -> void
                    {
                      date_of_next_config_update(
                          preferences,
                          std::chrono::duration_cast<std::chrono::hours>(now));
                    });
  }
  {
    const std::chrono::hours d =
        date_of_next_version_update_message(preferences);

    add_button_item("Version check in " + std::to_string((d - now).count())
                        + " h.",
                    [&preferences, now]() -> void
                    {
                      date_of_next_version_update_message(
                          preferences,
                          std::chrono::duration_cast<std::chrono::hours>(now));
                    });
  }

  add_title("SYSTEM");
  add_fps_entry();
  add_text_item("Language", iscool::system::get_language_code());

  m_popup->show(m_controls->all_nodes, m_style_bounds, m_inputs.root());
}

void bim::axmol::app::debug_popup::add_fps_entry()
{
  ax::Director& director = *ax::Director::getInstance();

  add_toggle_item("Show FPS", director.isStatsDisplay(),
                  [&director]() -> bool
                  {
                    director.setStatsDisplay(!director.isStatsDisplay());
                    return director.isStatsDisplay();
                  });
}

void bim::axmol::app::debug_popup::add_feature_item(
    std::string_view label, bim::game::feature_flags flag)
{
  const bool available = !!(bim::axmol::app::available_feature_flags(
                                *m_context.get_local_preferences())
                            & flag);

  auto toggle_flag = [this, flag]() -> bool
  {
    const bim::game::feature_flags new_flags =
        bim::axmol::app::available_feature_flags(
            *m_context.get_local_preferences())
        ^ flag;
    bim::axmol::app::available_feature_flags(
        *m_context.get_local_preferences(), new_flags);
    return !!(new_flags & flag);
  };

  add_toggle_item(label, available, toggle_flag);
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
void bim::axmol::app::debug_popup::add_button_item(
    std::string_view label, std::function<void()> do_action)
{
  button_controls controls(m_context.get_widget_context(),
                           m_button_item_controls);
  controls.label->setString(label);

  bim::axmol::widget::button& b = *controls.button;

  m_inputs.push_back(b.input_node());

  b.connect_to_clicked(do_action);

  add_item(controls.all_nodes, m_button_item_bounds);
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

void bim::axmol::app::debug_popup::coin_transaction(int amount) const
{
  if (amount >= 0)
    add_coins(*m_context.get_local_preferences(), amount);
  else
    consume_coins(*m_context.get_local_preferences(), -amount);

  const ax::Node& n = *m_controls->close_button;

  m_wallet.animate_cash_flow(n.convertToWorldSpace(n.getContentSize() / 2));
}
