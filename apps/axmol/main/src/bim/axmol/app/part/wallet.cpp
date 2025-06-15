// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/part/wallet.hpp>

#include <bim/axmol/app/preference/wallet.hpp>

#include <bim/axmol/style/apply_bounds.hpp>
#include <bim/axmol/style/cache.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/instantiate_widgets.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/axmol/find_child_by_path.hpp>

#include <iscool/i18n/numeric.hpp>
#include <iscool/signals/implement_signal.hpp>

#define x_widget_scope bim::axmol::app::wallet::
#define x_widget_type_name controls
#define x_widget_controls x_widget(bim::axmol::widget::button, wallet_button)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <axmol/2d/ActionEase.h>
#include <axmol/2d/ActionInstant.h>
#include <axmol/2d/ActionInterval.h>
#include <axmol/2d/Label.h>
#include <axmol/base/Director.h>

#include <iscool/random/rand.hpp>

#include <cassert>

IMPLEMENT_SIGNAL(bim::axmol::app::wallet, clicked, m_clicked)

static constexpr std::int64_t g_coins_per_transaction = 10;

bim::axmol::app::wallet::wallet(const context& context,
                                const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_balance_label(dynamic_cast<ax::Label*>(bim::axmol::find_child_by_path(
        *m_controls->wallet_button,
        *style.get_string("label-path-in-button"))))
  , m_coins(g_coins_per_transaction)
  , m_coin_style(*style.get_declaration("coin"))
  , m_coin_bounds_style(context.get_widget_context().style_cache.get_bounds(
        *style.get_declaration("coin-bounds")))
{
  m_controls->wallet_button->connect_to_clicked(
      [this]() -> void
      {
        m_clicked();
      });
}

bim::axmol::app::wallet::~wallet() = default;

bim::axmol::input::node_reference bim::axmol::app::wallet::input_node() const
{
  return m_controls->wallet_button->input_node();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::wallet::display_nodes() const
{
  return m_controls->all_nodes;
}

void bim::axmol::app::wallet::attached()
{
  std::array<std::size_t, g_coins_per_transaction> slots;

  for (int i = 0; i != g_coins_per_transaction; ++i)
    {
      const node_pool::slot slot = m_coins.pick_available();
      slots[i] = slot.id;
      *slot.value = new_coin_node();
    }

  for (std::size_t i : slots)
    m_coins.release(i);
}

void bim::axmol::app::wallet::enter()
{
  m_displayed_value = coins_balance(*m_context.get_local_preferences());
  m_balance_label->setString(
      iscool::i18n::numeric::to_string(m_displayed_value));
}

void bim::axmol::app::wallet::animate_cash_flow()
{
  animate_cash_flow(ax::Director::getInstance()->getWinSizeInPixels() / 2);
}

void bim::axmol::app::wallet::animate_cash_flow(
    const ax::Vec2& source_world_position)
{
  const std::int64_t from_amount = m_displayed_value;
  m_displayed_value = coins_balance(*m_context.get_local_preferences());
  const std::int64_t to_amount = m_displayed_value;

  const float update_label_duration_seconds =
      (to_amount <= from_amount) ? 0.8 : 1;

  ax::ActionInterval* const update_label = ax::ActionFloat::create(
      update_label_duration_seconds, 0, 1,
      [this, from_amount, to_amount](float t) -> void
      {
        update_balance_label(from_amount, to_amount, t);
      });

  if (to_amount <= from_amount)
    {
      m_action_runner.run(*update_label);
      return;
    }

  const ax::Vec2 source_position =
      m_controls->wallet_button->convertToNodeSpace(source_world_position);

  spawn_coins(source_position, *update_label);
}

bim::axmol::ref_ptr<ax::Node> bim::axmol::app::wallet::new_coin_node() const
{
  const bim::axmol::widget::context& widget_context =
      m_context.get_widget_context();

  const bim::axmol::ref_ptr<ax::Node> node =
      bim::axmol::widget::instantiate_widget(widget_context, m_coin_style);

  m_controls->wallet_button->addChild(node.get());
  bim::axmol::style::apply_bounds(m_coin_bounds_style, *node,
                                  *m_controls->wallet_button,
                                  widget_context.device_scale);
  node->removeFromParent();

  return node;
}

void bim::axmol::app::wallet::spawn_coins(
    const ax::Vec2& source_position, ax::ActionInterval& update_label_action)
{
  const ax::Node& wallet = *m_controls->wallet_button;

  node_pool::slot slots[g_coins_per_transaction];
  prepare_coin_assets(slots, source_position);

  const ax::Vec2& half_wallet_size = wallet.getContentSize() / 2;

  ax::Vector<ax::FiniteTimeAction*> move_actions(g_coins_per_transaction);

  for (int i = 0; i != g_coins_per_transaction; ++i)
    {
      const bool forward = i % 2 == 0;
      const float fx = float((i + 1) / 2) / 5;
      const float fy_ref = 0.6;
      const float fy_spread = 0.3;
      const float fy =
          fy_ref
          + iscool::random::rand::get_default().random(-fy_spread, fy_spread);
      const float sign =
          std::copysign(1.f, half_wallet_size.x - source_position.x);
      const ax::Vec2 target_position(
          half_wallet_size.x
              + (forward ? sign : -sign) * fx * half_wallet_size.x,
          half_wallet_size.y + (fy - fy_ref) * half_wallet_size.y);

      const float move_duration_seconds =
          1.4 * iscool::random::rand::get_default().random(0.95f, 1.05f);
      constexpr float scale_duration_seconds = 0.6;

      const ax::BezierConfig bezier =
          configure_bezier(fx, fy, source_position, target_position, forward);

      move_actions.pushBack(ax::TargetedAction::create(
          slots[i].value->get(),
          ax::Sequence::create(
              ax::BezierTo::create(move_duration_seconds, bezier),
              ax::EaseBackIn::create(
                  ax::ScaleTo::create(scale_duration_seconds, 0)),
              nullptr)));
    }

  m_action_runner.run(
      *ax::Sequence::create(ax::Spawn::create(move_actions),
                            ax::CallFunc::create(
                                [this, slots]() -> void
                                {
                                  for (const node_pool::slot& slot : slots)
                                    {
                                      (*slot.value)->removeFromParent();
                                      m_coins.release(slot.id);
                                    }
                                }),
                            &update_label_action, nullptr));
}

void bim::axmol::app::wallet::prepare_coin_assets(
    std::span<node_pool::slot> slots, const ax::Vec2& source_position)
{
  for (node_pool::slot& slot : slots)
    slot = m_coins.pick_available();

  for (node_pool::slot& slot : slots)
    if (!*slot.value)
      *slot.value = new_coin_node();

  ax::Node& wallet = *m_controls->wallet_button;

  for (node_pool::slot& slot : slots)
    {
      ax::Node& s = **slot.value;

      assert(s.getParent() == nullptr);
      wallet.addChild(&s);
      s.setScale(1);
      s.setPosition(source_position);
    }
}

ax::BezierConfig bim::axmol::app::wallet::configure_bezier(
    float fx, float fy, const ax::Vec2& source_position,
    const ax::Vec2& target_position, bool forward) const
{
  const ax::Vec2 d = target_position - source_position;

  ax::BezierConfig bezier;
  bezier.endPosition = target_position;

  if (d.x >= 0)
    {
      if (d.y >= 0)
        {
          if (forward)
            {
              bezier.controlPoint_1.x = source_position.x + fx * d.x;
              bezier.controlPoint_1.y = source_position.y - fy * d.y;
              bezier.controlPoint_2.x = target_position.x - 0.4 * d.x;
              bezier.controlPoint_2.y = source_position.y + 0.4 * d.y;
            }
          else
            {
              bezier.controlPoint_1.x = source_position.x - fx * d.x;
              bezier.controlPoint_1.y = source_position.y - fy * d.y;
              bezier.controlPoint_2.x = source_position.x + 0.4 * d.x;
              bezier.controlPoint_2.y = target_position.y - 0.4 * d.y;
            }
        }
      else
        {
          if (forward)
            {
              bezier.controlPoint_1.x = source_position.x + fx * d.x;
              bezier.controlPoint_1.y = source_position.y - fy * d.y;
              bezier.controlPoint_2.x = target_position.x - 0.4 * d.x;
              bezier.controlPoint_2.y = source_position.y + 0.4 * d.y;
            }
          else
            {
              bezier.controlPoint_1.x = source_position.x - fx * d.x;
              bezier.controlPoint_1.y = source_position.y - fy * d.y;
              bezier.controlPoint_2.x = source_position.x + 0.4 * d.x;
              bezier.controlPoint_2.y = target_position.y - 0.4 * d.y;
            }
        }
    }
  else
    {
      if (d.y >= 0)
        {
          if (forward)
            {
              bezier.controlPoint_1.x = source_position.x + fx * d.x;
              bezier.controlPoint_1.y = source_position.y - fy * d.y;
              bezier.controlPoint_2.x = target_position.x - 0.4 * d.x;
              bezier.controlPoint_2.y = source_position.y + 0.4 * d.y;
            }
          else
            {
              bezier.controlPoint_1.x = source_position.x - fx * d.x;
              bezier.controlPoint_1.y = source_position.y - fy * d.y;
              bezier.controlPoint_2.x = source_position.x + 0.4 * d.x;
              bezier.controlPoint_2.y = target_position.y - 0.4 * d.y;
            }
        }
      else
        {
          if (forward)
            {
              bezier.controlPoint_1.x = source_position.x + fx * d.x;
              bezier.controlPoint_1.y = source_position.y - fy * d.y;
              bezier.controlPoint_2.x = target_position.x - 0.4 * d.x;
              bezier.controlPoint_2.y = source_position.y + 0.4 * d.y;
            }
          else
            {
              bezier.controlPoint_1.x = source_position.x - fx * d.x;
              bezier.controlPoint_1.y = source_position.y - fy * d.y;
              bezier.controlPoint_2.x = source_position.x + 0.4 * d.x;
              bezier.controlPoint_2.y = target_position.y - 0.4 * d.y;
            }
        }
    }

  return bezier;
}

void bim::axmol::app::wallet::update_balance_label(std::int64_t from_amount,
                                                   std::int64_t to_amount,
                                                   float t) const
{
  std::int64_t amount;

  if ((t == 1) && (m_action_runner.running_action_count() == 1))
    amount = m_displayed_value;
  else
    amount =
        from_amount + (std::int64_t)std::lround((to_amount - from_amount) * t);

  m_balance_label->setString(iscool::i18n::numeric::to_string(amount));
}
