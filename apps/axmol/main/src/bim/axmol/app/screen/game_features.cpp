// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen/game_features.hpp>

#include <bim/axmol/app/part/wallet.hpp>
#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/shop_intent.hpp>
#include <bim/axmol/app/widget/game_feature_button.hpp>

#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/merge_named_node_groups.hpp>
#include <bim/axmol/widget/named_node_group.hpp>
#include <bim/axmol/widget/ui/button.hpp>
#include <bim/axmol/widget/ui/list.hpp>
#include <bim/axmol/widget/ui/passive_node.hpp>

#include <bim/axmol/action/dynamic_factory.hpp>

#include <bim/axmol/style/apply_bounds.hpp>
#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/node.hpp>
#include <bim/axmol/input/observer/axmol_node_touch_observer.hpp>
#include <bim/axmol/input/observer/single_key_observer.hpp>
#include <bim/axmol/input/observer/touch_anywhere.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>

#include <bim/app/analytics/button_clicked.hpp>
#include <bim/app/analytics/coins_transaction.hpp>
#include <bim/app/config.hpp>
#include <bim/app/preference/feature_flags.hpp>
#include <bim/app/preference/wallet.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/bit_map.impl.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/log/log.hpp>
#include <iscool/monitoring/implement_state_monitor.hpp>
#include <iscool/random/rand.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/ActionInterval.h>
#include <axmol/2d/Label.h>

#define x_widget_scope bim::axmol::app::game_features::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button,                                        \
           random_button) x_widget(bim::axmol::widget::button, erase_button)  \
      x_widget(bim::axmol::widget::button, back_button)                       \
          x_widget(bim::axmol::widget::list, list)                            \
              x_widget(ax::Label, feature_description_label)                  \
                  x_widget(bim::axmol::app::game_feature_button, slot_0)      \
                      x_widget(bim::axmol::app::game_feature_button, slot_1)

#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::game_features, back, m_back);
IMPLEMENT_SIGNAL(bim::axmol::app::game_features, shop, m_shop);

ic_implement_state_monitor(bim::axmol::app::game_features, m_monitor, idle,
                           ((idle)((erase_slot)(assign_slot))) //
                           ((erase_slot)((idle)))              //
                           ((assign_slot)((idle)))             //
);

bim::axmol::app::game_features::game_features(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_escape(ax::EventKeyboard::KeyCode::KEY_BACK)
  , m_list_input_stencil(*m_controls->list)
  , m_list_item_inputs(new bim::axmol::input::node(m_list_input_stencil))
  , m_selected_feature{}
  , m_slot_bounds_off(context.get_widget_context().style_cache.get_bounds(
        *style.get_declaration("slot-bounds-off")))
  , m_slot_display_off(context.get_widget_context().style_cache.get_display(
        *style.get_declaration("slot-display-off")))
  , m_item_bounds_off(context.get_widget_context().style_cache.get_bounds(
        *style.get_declaration("item-bounds-off")))
  , m_item_display_off(context.get_widget_context().style_cache.get_display(
        *style.get_declaration("item-display-off")))
  , m_wallet(new wallet(context, *style.get_declaration("wallet")))
  , m_slot{ m_controls->slot_0, m_controls->slot_1 }
  , m_message_popup(
        new message_popup(context, *style.get_declaration("message-popup")))
{
  {
    const iscool::style::declaration& slot_action_style =
        *style.get_declaration("action.slot-selection");

    for (std::size_t i = 0; i != bim::app::g_game_feature_slot_count; ++i)
      m_slot_selection_action[i] = ax::TargetedAction::create(
          m_slot[i],
          context.get_widget_context()
              .action_factory
              .create(context.get_widget_context().colors, slot_action_style)
              .get());
  }

  m_all_nodes = m_controls->all_nodes;
  bim::axmol::widget::merge_named_node_groups(m_all_nodes,
                                              m_wallet->display_nodes());

  m_inputs.push_back(m_wallet->input_node());
  m_inputs.push_back(m_escape);
  m_inputs.push_back(m_controls->back_button->input_node());
  m_inputs.push_back(m_controls->random_button->input_node());
  m_inputs.push_back(m_controls->erase_button->input_node());
  m_inputs.push_back(m_controls->slot_0->input_node());
  m_inputs.push_back(m_controls->slot_1->input_node());
  m_inputs.push_back(m_controls->list->input_node());
  m_inputs.push_back(m_list_item_inputs);
  m_inputs.push_back(m_tap);

  m_wallet->connect_to_clicked(
      [this]()
      {
        open_shop_from_wallet();
      });

  m_escape->connect_to_released(
      [this]()
      {
        cancel_or_quit();
      });

  m_controls->back_button->connect_to_clicked(
      [this]()
      {
        dispatch_back();
      });

  m_message_popup->connect_to_ok(
      [this]()
      {
        open_shop_from_shortage();
      });

  m_controls->random_button->connect_to_clicked(
      [this]()
      {
        select_random_features();
      });

  m_controls->erase_button->connect_to_clicked(
      [this]()
      {
        start_erase_mode();
      });

  m_tap->connect_to_release(
      [this]()
      {
        cancel();
      });

  const iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();
  const bim::app::config& config = *m_context.get_config();

  for (std::size_t i = 0; i != bim::app::g_game_feature_slot_count; ++i)
    {
      m_slot[i]->feature(bim::app::feature_flag_in_slot(preferences, i));
      m_slot[i]->available(bim::app::available_feature_slot(preferences, i));
      m_slot[i]->price(config.game_feature_slot_price[i]);

      m_slot[i]->connect_to_clicked(
          [this, i]()
          {
            select_slot(i);
          });
    }

  const iscool::style::declaration& row_container_style =
      *style.get_declaration("list-row");
  const iscool::style::declaration& button_on_style =
      *style.get_declaration("game-feature-button.on");
  const iscool::style::declaration& button_off_style =
      *style.get_declaration("game-feature-button.off");
  const iscool::style::declaration& row_bounds =
      *style.get_declaration("game-features-row-bounds");

  constexpr std::size_t buttons_per_row = 4;
  constexpr std::size_t button_count = 24;

  bim::axmol::widget::named_node_group row_controls;

  const bim::axmol::widget::context& widget_context =
      context.get_widget_context();

  const std::string node_names[buttons_per_row] = { "feature_0", "feature_1",
                                                    "feature_2", "feature_3" };

  const auto add_list_row = [&]()
  {
    bim::axmol::ref_ptr<bim::axmol::widget::passive_node> item =
        bim::axmol::widget::factory<bim::axmol::widget::passive_node>::create(
            widget_context, row_container_style);

    item->fill(row_controls, row_bounds);
    row_controls.clear();
    m_controls->list->push_back(*item);
  };

  const auto new_button = [&](std::size_t& index,
                              const iscool::style::declaration& style,
                              bool available) -> game_feature_button&
  {
    bim::axmol::ref_ptr<game_feature_button> button(
        game_feature_button::create(widget_context, style));
    button->available(available);
    row_controls[node_names[index % buttons_per_row]] = button;
    ++index;

    if (index % buttons_per_row == 0)
      add_list_row();

    return *button;
  };

  const iscool::style::declaration& item_selection_action_style =
      *style.get_declaration("action.item-selection");

  const bim::game::feature_flags available_features =
      bim::app::available_feature_flags(preferences);

  std::size_t i = 0;
  for (bim::game::feature_flags f : { bim::game::feature_flags::falling_blocks,
                                      bim::game::feature_flags::shield,
                                      bim::game::feature_flags::invisibility,
                                      bim::game::feature_flags::fog_of_war })
    {
      game_feature_button& button =
          new_button(i, button_on_style, !!(available_features & f));
      button.feature(f);
      button.price(config.game_feature_price[f]);

      m_item_selection_action[f] = ax::TargetedAction::create(
          &button, context.get_widget_context()
                       .action_factory
                       .create(context.get_widget_context().colors,
                               item_selection_action_style)
                       .get());

      m_catalog[f] = &button;
      m_list_item_inputs->push_back(button.input_node());

      button.connect_to_clicked(
          [this, f]()
          {
            select_feature(f);
          });
    }

  for (; i < button_count;)
    new_button(i, button_off_style, true);

  if (!row_controls.empty())
    add_list_row();

  const iscool::style::declaration& bottom_filler_style =
      *style.get_declaration("list-bottom-filler-bounds");
  bim::axmol::ref_ptr<ax::Node> filler(ax::Node::create());
  m_controls->list->push_back(*filler, bottom_filler_style);
}

bim::axmol::app::game_features::~game_features() = default;

bim::axmol::input::node_reference
bim::axmol::app::game_features::input_node() const
{
  return m_inputs.root();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::game_features::display_nodes() const
{
  return m_all_nodes;
}

void bim::axmol::app::game_features::attached()
{
  m_wallet->attached();
}

void bim::axmol::app::game_features::displaying()
{
  m_wallet->enter();
  update_affordability();

  show_feature_message(m_selected_feature);
}

void bim::axmol::app::game_features::closing()
{
  cancel();
}

void bim::axmol::app::game_features::start_erase_mode()
{
  if (!m_monitor->is_idle_state())
    {
      cancel();
      return;
    }

  button_clicked(*m_context.get_analytics(), "erase", "game-features");

  assert(m_selected_feature == bim::game::feature_flags{});

  const iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();
  bool slot_is_occupied[bim::app::g_game_feature_slot_count];

  for (std::size_t i = 0; i != bim::app::g_game_feature_slot_count; ++i)
    slot_is_occupied[i] = bim::app::feature_flag_in_slot(preferences, i)
                          != bim::game::feature_flags{};

  bool has_any_occupied_slot = false;

  for (std::size_t i = 0; i != bim::app::g_game_feature_slot_count; ++i)
    has_any_occupied_slot |= slot_is_occupied[i];

  if (!has_any_occupied_slot)
    return;

  for (std::size_t i = 0; i != bim::app::g_game_feature_slot_count; ++i)
    if (slot_is_occupied[i])
      m_slot_animation.run(*m_slot_selection_action[i]);

  m_monitor->set_erase_slot_state();
}

void bim::axmol::app::game_features::select_slot(std::size_t i)
{
  if (m_monitor->is_idle_state())
    {
      purchase_slot(i);
      return;
    }

  if (m_monitor->is_erase_slot_state())
    erase_slot(i);
  else if (m_monitor->is_assign_slot_state())
    assign_slot(i);
}

void bim::axmol::app::game_features::erase_slot(std::size_t i)
{
  assert(m_monitor->is_erase_slot_state());

  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  if (bim::app::available_feature_slot(preferences, i))
    {
      bim::app::feature_flag_in_slot(preferences, i,
                                     bim::game::feature_flags{});
      m_slot[i]->feature(bim::game::feature_flags{});
    }

  cancel_erase_slot();
}

void bim::axmol::app::game_features::assign_slot(std::size_t i)
{
  assert(m_monitor->is_assign_slot_state());

  if (purchase_slot(i))
    {
      iscool::preferences::local_preferences& preferences =
          *m_context.get_local_preferences();

      bim::app::feature_flag_in_slot(preferences, i, m_selected_feature);
      m_slot[i]->feature(m_selected_feature);
    }

  cancel_assign_slot();
}

bool bim::axmol::app::game_features::purchase_slot(std::size_t i)
{
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  if (bim::app::available_feature_slot(preferences, i))
    return true;

  const std::int64_t coins = bim::app::coins_balance(preferences);
  const std::int16_t price =
      m_context.get_config()->game_feature_slot_price[i];

  if (price <= coins)
    {
      bim::app::consume_coins(preferences, price);
      coins_transaction(*m_context.get_analytics(), "feature-slot", -price);
      m_wallet->animate_cash_flow();
      bim::app::available_feature_slot(preferences, i, true);
      m_slot[i]->available(true);
      update_affordability();
      return true;
    }

  m_message_popup->show(
      ic_gettext("You need more coins to purchase this item!"));
  return false;
}

void bim::axmol::app::game_features::select_feature(bim::game::feature_flags f)
{
  if (m_monitor->is_erase_slot_state())
    {
      cancel_erase_slot();
      return;
    }

  if (m_monitor->is_assign_slot_state() && (f == m_selected_feature))
    {
      cancel_assign_slot();
      return;
    }

  button_clicked(*m_context.get_analytics(), "feature", "game-features");

  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();
  bim::game::feature_flags available_features =
      bim::app::available_feature_flags(preferences);
  bool available = !!(available_features & f);

  if (!available)
    {
      const std::int64_t coins = bim::app::coins_balance(preferences);
      const std::int16_t price = m_context.get_config()->game_feature_price[f];

      if (price <= coins)
        {
          bim::app::consume_coins(preferences, price);
          coins_transaction(*m_context.get_analytics(), "feature-item",
                            -price);
          m_wallet->animate_cash_flow();
          available_features |= f;
          bim::app::available_feature_flags(preferences, available_features);
          m_catalog[f]->available(true);
          update_affordability();
          available = true;
        }
      else
        {
          if (m_monitor->is_assign_slot_state())
            cancel_assign_slot();

          m_message_popup->show(
              ic_gettext("You need more coins to purchase this item!"));
          return;
        }
    }

  if (m_monitor->is_assign_slot_state())
    deselect_item();
  else
    {
      assert(m_monitor->is_idle_state());

      for (std::size_t i = 0; i != bim::app::g_game_feature_slot_count; ++i)
        m_slot_animation.run(*m_slot_selection_action[i]);

      m_monitor->set_assign_slot_state();
    }

  m_selected_feature = f;
  activate_item_selection(f);
  show_feature_message(f);
}

void bim::axmol::app::game_features::show_feature_message(
    bim::game::feature_flags f)
{
  if (f == bim::game::feature_flags{})
    {
      m_controls->feature_description_label->setString(
          fmt::format(fmt::runtime(ic_ngettext(
                          "Customize your games with up to {} feature.",
                          "Customize your games with up to {} features.",
                          bim::app::g_game_feature_slot_count)),
                      bim::app::g_game_feature_slot_count));
      return;
    }

  const char* message = "";

  switch (f)
    {
    case bim::game::feature_flags::falling_blocks:
      message =
          ic_gettext("Falling blocks reduce the arena after two minutes!");
      break;
    case bim::game::feature_flags::shield:
      message = ic_gettext("Find this incredibly strong barrel, it will save "
                           "you from one hit!");
      break;
    case bim::game::feature_flags::fog_of_war:
      message = ic_gettext(
          "A thick fog covers the arena. You can't see were you did not go!");
      break;
    case bim::game::feature_flags::invisibility:
      message = ic_gettext("Find the invisibility power up to disappear from "
                           "the screen of all other players!");
      break;
    }

  m_controls->feature_description_label->setString(message);
}

void bim::axmol::app::game_features::update_affordability()
{
  const std::int64_t coins =
      bim::app::coins_balance(*m_context.get_local_preferences());
  const bim::app::config& config = *m_context.get_config();

  for (bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    {
      const std::int16_t price = config.game_feature_price[f];
      m_catalog[f]->affordable(price <= coins);
    }

  for (std::size_t i = 0; i != bim::app::g_game_feature_slot_count; ++i)
    {
      const std::int16_t price = config.game_feature_slot_price[i];
      m_slot[i]->affordable(price <= coins);
    }
}

void bim::axmol::app::game_features::cancel_assign_slot()
{
  stop_slot_animations();
  deselect_item();
  m_monitor->set_idle_state();
}

void bim::axmol::app::game_features::cancel_erase_slot()
{
  stop_slot_animations();
  m_monitor->set_idle_state();
}

void bim::axmol::app::game_features::stop_slot_animations()
{
  m_slot_animation.stop();

  for (std::size_t i = 0; i != bim::app::g_game_feature_slot_count; ++i)
    {
      bim::axmol::style::apply_bounds(
          m_slot_bounds_off, *m_slot[i], *m_slot[i]->getParent(),
          m_context.get_widget_context().device_scale);

      bim::axmol::style::apply_display(m_slot_display_off, *m_slot[i]);
    }
}

void bim::axmol::app::game_features::activate_item_selection(
    bim::game::feature_flags f)
{
  m_selected_feature = f;
  m_item_animation.run(*m_item_selection_action[f]);
}

void bim::axmol::app::game_features::deselect_item()
{
  show_feature_message(bim::game::feature_flags{});

  if (m_selected_feature == bim::game::feature_flags{})
    return;

  m_item_animation.stop();

  bim::axmol::style::apply_bounds(m_item_bounds_off,
                                  *m_catalog[m_selected_feature],
                                  *m_catalog[m_selected_feature]->getParent(),
                                  m_context.get_widget_context().device_scale);

  bim::axmol::style::apply_display(m_item_display_off,
                                   *m_catalog[m_selected_feature]);

  m_selected_feature = bim::game::feature_flags{};
}

void bim::axmol::app::game_features::select_random_features()
{
  button_clicked(*m_context.get_analytics(), "random", "game-features");

  cancel();

  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();
  std::size_t slots[bim::app::g_game_feature_slot_count];
  std::size_t slot_count = 0;

  for (std::size_t i = 0; i != bim::app::g_game_feature_slot_count; ++i)
    if (bim::app::available_feature_slot(preferences, i))
      {
        slots[slot_count] = i;
        ++slot_count;
      }

  const bim::game::feature_flags available_features =
      bim::app::available_feature_flags(preferences);
  bim::game::feature_flags
      features[std::size(bim::game::g_all_game_feature_flags)];
  std::size_t feature_count = 0;

  for (const bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    if (!!(available_features & f))
      {
        features[feature_count] = f;
        ++feature_count;
      }

  std::size_t needed = slot_count;

  for (std::size_t available = feature_count, i = 0, j = 0;
       (needed != 0) && (available != 0); --available, ++j)
    if (iscool::random::rand::get_default().random<std::size_t>(1, available)
        <= needed)
      {
        bim::app::feature_flag_in_slot(preferences, slots[i], features[j]);
        m_slot[slots[i]]->feature(features[j]);
        ++i;
        --needed;
      }

  for (std::size_t i = slot_count - needed; i != slot_count; ++i)
    {
      bim::app::feature_flag_in_slot(preferences, slots[i],
                                     bim::game::feature_flags{});
      m_slot[i]->feature(bim::game::feature_flags{});
    }
}

void bim::axmol::app::game_features::open_shop_from_shortage()
{
  button_clicked(*m_context.get_analytics(), "shortage", "game-features");
  m_shop(shop_intent::program_request);
}

void bim::axmol::app::game_features::open_shop_from_wallet()
{
  button_clicked(*m_context.get_analytics(), "wallet", "game-features");
  m_shop(shop_intent::user_request);
}

void bim::axmol::app::game_features::cancel_or_quit()
{
  if (m_monitor->is_idle_state())
    dispatch_back();
  else
    cancel();
}

void bim::axmol::app::game_features::cancel()
{
  if (m_monitor->is_idle_state())
    return;

  if (m_monitor->is_assign_slot_state())
    cancel_assign_slot();
  else
    {
      assert(m_monitor->is_erase_slot_state());
      cancel_erase_slot();
    }
}

void bim::axmol::app::game_features::dispatch_back()
{
  m_back();
}
