// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/app/shop_intent_fwd.hpp>

#include <bim/app/constant/game_feature_slot_count.hpp>

#include <bim/axmol/action/runner.hpp>
#include <bim/axmol/input/node_pointer.hpp>
#include <bim/axmol/input/observer/axmol_node_touch_observer_handle.hpp>
#include <bim/axmol/input/observer/single_key_observer_handle.hpp>
#include <bim/axmol/input/observer/touch_anywhere_handle.hpp>
#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/ref_ptr.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <bim/bit_map.hpp>

#include <iscool/context.hpp>
#include <iscool/monitoring/declare_state_monitor.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <memory>

namespace bim::axmol::style
{
  class bounds_properties;
  class display_properties;
}

namespace bim::axmol::widget
{
  class context;
}

namespace bim::app
{
  struct config;
}

namespace ax
{
  class FiniteTimeAction;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::app
{
  class analytics_service;
}

namespace bim::axmol::app
{
  class application_event_dispatcher;
  class game_feature_button;
  class main_scene;
  class message_popup;
  class wallet;

  class game_features
  {
    DECLARE_VOID_SIGNAL(back, m_back)
    DECLARE_SIGNAL(void(shop_intent), shop, m_shop)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((bim::app::analytics_service*)(analytics))                    //
            ((application_event_dispatcher*)(event_dispatcher))            //
            ((const bim::app::config*)(config))                            //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ),
        ic_context_no_properties);

  public:
    game_features(const context& context,
                  const iscool::style::declaration& style);
    ~game_features();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& display_nodes() const;

    void attached();
    void displaying();
    void closing();

  private:
    using action_ptr = bim::axmol::ref_ptr<ax::FiniteTimeAction>;

  private:
    void start_erase_mode();

    void select_slot(std::size_t i);
    void erase_slot(std::size_t i);
    void assign_slot(std::size_t i);
    bool purchase_slot(std::size_t i);

    void select_feature(bim::game::feature_flags f);
    void show_feature_message(bim::game::feature_flags f);

    void update_affordability();

    void cancel_assign_slot();
    void cancel_erase_slot();

    void stop_slot_animations();

    void activate_item_selection(bim::game::feature_flags f);
    void deselect_item();

    void select_random_features();

    void open_shop_from_shortage();
    void open_shop_from_wallet();

    void cancel_or_quit();
    void cancel();
    void dispatch_back();

  private:
    ic_declare_state_monitor(m_monitor);
    bim_declare_controls_struct(controls, m_controls, 7);

    bim::axmol::input::single_key_observer_handle m_escape;
    bim::axmol::input::touch_anywhere_handle m_tap;
    bim::axmol::input::tree m_inputs;
    bim::axmol::input::axmol_node_touch_observer_handle m_list_input_stencil;
    bim::axmol::input::node_pointer m_list_item_inputs;

    bim::game::feature_flags m_selected_feature;

    bim::axmol::action::runner m_item_animation;
    bim::axmol::action::runner m_slot_animation;
    action_ptr m_slot_selection_action[bim::app::g_game_feature_slot_count];
    bim::bit_map<bim::game::feature_flags, action_ptr> m_item_selection_action;

    const bim::axmol::style::bounds_properties& m_slot_bounds_off;
    const bim::axmol::style::display_properties& m_slot_display_off;
    const bim::axmol::style::bounds_properties& m_item_bounds_off;
    const bim::axmol::style::display_properties& m_item_display_off;

    const std::unique_ptr<wallet> m_wallet;

    bim::axmol::widget::named_node_group m_all_nodes;

    bim::bit_map<bim::game::feature_flags, game_feature_button*> m_catalog;
    game_feature_button* m_slot[bim::app::g_game_feature_slot_count];

    std::unique_ptr<message_popup> m_message_popup;
  };
}
