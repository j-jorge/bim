// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/axmol/action/runner.hpp>
#include <bim/axmol/dynamic_node_pool.hpp>
#include <bim/axmol/input/node_reference.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <cstdint>
#include <span>

namespace bim::axmol::style
{
  class bounds_properties;
}

namespace bim::axmol::widget
{
  class context;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace iscool::style
{
  class declaration;
}

namespace ax
{
  class ActionInterval;
  class Label;
  class Node;
  class Vec2;
  struct BezierConfig;
}

namespace bim::axmol::app
{
  class wallet
  {
    DECLARE_VOID_SIGNAL(clicked, m_clicked)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                      //
            ((const bim::axmol::widget::context*)(widget_context)) //
            ((iscool::preferences::local_preferences*)(local_preferences))),
        ic_context_no_properties);

  public:
    wallet(const context& context, const iscool::style::declaration& style);
    ~wallet();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& display_nodes() const;

    void attached();
    void enter();

    void animate_cash_flow();
    void animate_cash_flow(const ax::Vec2& source_world_position);

  private:
    using node_pool = bim::axmol::dynamic_node_pool<ax::Node>;

  private:
    bim::axmol::ref_ptr<ax::Node> new_coin_node() const;

    void spawn_coins(const ax::Vec2& source_position,
                     ax::ActionInterval& update_label_action);
    void prepare_coin_assets(std::span<node_pool::slot> slots,
                             const ax::Vec2& source_position);
    ax::BezierConfig configure_bezier(float fx, float fy,
                                      const ax::Vec2& source_position,
                                      const ax::Vec2& target_position,
                                      bool forward) const;
    void update_balance_label(std::int64_t from_amount, std::int64_t to_amount,
                              float t) const;

  private:
    bim_declare_controls_struct(controls, m_controls, 1);

    bim::axmol::action::runner m_action_runner;
    iscool::signals::scoped_connection m_cash_flow_connection;

    ax::Label* const m_balance_label;
    std::int64_t m_displayed_value;

    node_pool m_coins;
    const iscool::style::declaration& m_coin_style;
    const bim::axmol::style::bounds_properties& m_coin_bounds_style;
  };
}
