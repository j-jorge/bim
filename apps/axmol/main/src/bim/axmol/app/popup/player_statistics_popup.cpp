// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/player_statistics_popup.hpp>

#include <bim/axmol/app/popup/popup.hpp>

#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/observer/single_key_observer.hpp>
#include <bim/axmol/input/observer/touch_anywhere.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>

#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/factory/progress_timer.hpp>
#include <bim/axmol/widget/implement_widget.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/app/preference/arena_stats.hpp>

#include <axmol/2d/Label.h>
#include <axmol/2d/ProgressTimer.h>
#include <axmol/base/EventKeyboard.h>

#define x_widget_scope bim::axmol::app::player_statistics_popup::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Label, arena_games_total) x_widget(ax::Label, arena_victories) \
      x_widget(ax::Label, arena_defeats) x_widget(ax::Label, arena_draws)     \
          x_widget(ax::Label, success_rate_percents)                          \
              x_widget(ax::ProgressTimer, success_rate_progress)

#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <iscool/i18n/numeric.hpp>
#include <iscool/preferences/local_preferences.hpp>

#include <fmt/format.h>

bim::axmol::app::player_statistics_popup::player_statistics_popup(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_escape(ax::EventKeyboard::KeyCode::KEY_BACK)
  , m_controls(*context.get_widget_context(),
               *style.get_declaration("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_popup(new popup(context, *style.get_declaration("popup")))
{
  m_inputs.push_back(m_escape);

  m_escape->connect_to_released(
      [this]()
      {
        m_popup->hide();
      });

  m_inputs.push_back(m_tap);
  m_tap->connect_to_release(
      [this]()
      {
        m_popup->hide();
      });
}

bim::axmol::app::player_statistics_popup::~player_statistics_popup() = default;

void bim::axmol::app::player_statistics_popup::show()
{
  m_popup->show(m_controls->all_nodes, m_style_bounds, m_inputs.root());

  const iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  const std::int64_t total_games = bim::app::games_in_arena(preferences);
  const std::int64_t games_win = bim::app::victories_in_arena(preferences);
  const std::int64_t games_defeat = bim::app::defeats_in_arena(preferences);
  const std::int64_t games_draw = total_games - games_win - games_defeat;

  m_controls->arena_games_total->setString(
      iscool::i18n::numeric::to_string(total_games));

  m_controls->arena_victories->setString(
      iscool::i18n::numeric::to_string(games_win));
  m_controls->arena_defeats->setString(
      iscool::i18n::numeric::to_string(games_defeat));
  m_controls->arena_draws->setString(
      iscool::i18n::numeric::to_string(games_draw));

  if (total_games == 0)
    {
      m_controls->success_rate_percents->setString("-");
      m_controls->success_rate_progress->setPercentage(100);
    }
  else
    {
      const float percents = float(games_win * 100) / total_games;

      m_controls->success_rate_percents->setString(
          fmt::format("{}%", std::lround(percents)));
      m_controls->success_rate_progress->setPercentage(percents);
    }
}
