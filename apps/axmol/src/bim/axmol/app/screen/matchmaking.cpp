#include <bim/axmol/app/screen/matchmaking.hpp>

#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/net/exchange/new_game_exchange.hpp>
#include <bim/net/session_handler.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Label.h>

#include <fmt/format.h>

#define x_widget_scope bim::axmol::app::matchmaking::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Label, status_label)                                           \
      x_widget(bim::axmol::widget::button, ready_button)
#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::matchmaking, start_game, m_start_game);

bim::axmol::app::matchmaking::matchmaking(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_new_game(new bim::net::new_game_exchange(
        m_context.get_session_handler()->message_stream()))
  , m_style_displaying(*style.get_declaration("display.displaying"))
  , m_style_new_game(*style.get_declaration("display.new-game"))
  , m_style_wait(*style.get_declaration("display.wait"))
{
  m_inputs.push_back(m_controls->ready_button->input_node());

  m_controls->ready_button->connect_to_clicked(
      [this]()
      {
        accept_game();
      });
}

bim::axmol::app::matchmaking::~matchmaking() = default;

bim::axmol::input::node_reference
bim::axmol::app::matchmaking::input_node() const
{
  return m_inputs.root();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::matchmaking::nodes() const
{
  return m_controls->all_nodes;
}

void bim::axmol::app::matchmaking::displayed()
{
  update_display_waiting();

  m_game_proposal_connection = m_new_game->connect_to_game_proposal(
      [this](unsigned player_count)
      {
        update_display_with_game_proposal(player_count);
      });
  m_launch_connection = m_new_game->connect_to_launch_game(
      [this](iscool::net::channel_id channel, unsigned player_count,
             unsigned player_index)
      {
        launch_game(channel, player_count, player_index);
      });

  assert(m_context.get_session_handler()->connected());
  m_new_game->start(m_context.get_session_handler()->session_id(), {});
}

void bim::axmol::app::matchmaking::closing()
{
  m_game_proposal_connection.disconnect();
  m_new_game->stop();
}

void bim::axmol::app::matchmaking::update_display_with_game_proposal(
    unsigned player_count)
{
  if (player_count <= 1)
    {
      // TODO: state monitor
      update_display_waiting();
      return;
    }

  bim::axmol::widget::apply_display(m_context.get_widget_context().style_cache,
                                    m_controls->all_nodes, m_style_new_game);

  m_controls->status_label->setString(
      fmt::format("Got {} player!", player_count));
  //      ic_ngettext("Got {} player!", "Got {} players!", player_count))));
}

void bim::axmol::app::matchmaking::update_display_waiting()
{
  bim::axmol::widget::apply_display(m_context.get_widget_context().style_cache,
                                    m_controls->all_nodes, m_style_wait);

  m_controls->status_label->setString(ic_gettext("Setting up a new game…"));
}

void bim::axmol::app::matchmaking::accept_game()
{
  bim::axmol::widget::apply_display(m_context.get_widget_context().style_cache,
                                    m_controls->all_nodes, m_style_wait);

  m_controls->status_label->setString(
      ic_gettext("Waiting for your opponents…"));

  m_game_proposal_connection.disconnect();
  m_new_game->accept();
}

void bim::axmol::app::matchmaking::launch_game(iscool::net::channel_id channel,
                                               unsigned player_count,
                                               unsigned player_index)
{
  m_launch_connection.disconnect();
  m_start_game();
}
