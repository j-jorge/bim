// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/assign_feature_slots_job.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/business_url.hpp>

#include <bim/business/post.hpp>
#include <bim/business/request_headers.hpp>

#include <bim/game/feature_flags_string.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

enum class bim::app::assign_feature_slots_job::action_kind : std::int8_t
{
  skip,
  clear,
  assign
};

IMPLEMENT_SIGNAL(bim::app::assign_feature_slots_job, done, m_done)

bim::app::assign_feature_slots_job::assign_feature_slots_job(
    analytics_service& analytics, const bim::business::request_headers& r,
    player_profile& p)
  : m_analytics(analytics)
  , m_request_headers(r)
  , m_player_profile(p)
{
  m_task.fill(task{ .action = action_kind::skip,
                    .feature = bim::game::feature_flags{} });
}

bim::app::assign_feature_slots_job::~assign_feature_slots_job() = default;

void bim::app::assign_feature_slots_job::assign_slots(
    const slot_assignment& assignment)
{
  for (std::size_t i = 0; i != g_game_feature_slot_count; ++i)
    if (assignment[i])
      {
        m_task[i].action = action_kind::assign;
        m_task[i].feature = *assignment[i];
      }

  start_task();
}

void bim::app::assign_feature_slots_job::clear_slot(std::size_t i)
{
  m_task[i].action = action_kind::clear;

  start_task();
}

void bim::app::assign_feature_slots_job::start_task()
{
  // We don't want to flood the server when the user repeatedly taps the
  // randomize button, so we keep the requests in sequence.
  if (m_connection.connected())
    return;

  bool has_assignment = false;

  for (std::size_t i = 0; i != g_game_feature_slot_count; ++i)
    if (m_task[i].action == action_kind::clear)
      {
        m_task[i].action = action_kind::skip;
        send_clear_request(i);
        return;
      }
    else
      has_assignment |= (m_task[i].action == action_kind::assign);

  if (has_assignment)
    send_assign_request();
}

void bim::app::assign_feature_slots_job::send_clear_request(std::size_t i)
{
  assert(!m_connection.connected());

  Json::Value body;
  body["slot_index"] = i;

  m_connection = bim::business::post(
      BIM_BUSINESS_SERVER_URL "/client/game-feature/clear-slot",
      m_request_headers.headers, body,
      [this, i]()
        {
          m_player_profile.slot_feature[i] = bim::game::feature_flags{};
          clear_success();
        },
      [this]()
        {
          clear_error();
        });
}

void bim::app::assign_feature_slots_job::send_assign_request()
{
  assert(!m_connection.connected());

  Json::Value body;
  Json::Value& selection = body["selection"];

  slot_assignment assignment_wish;

  for (std::size_t i = 0; i != g_game_feature_slot_count; ++i)
    if (m_task[i].action == action_kind::assign)
      {
        m_task[i].action = action_kind::skip;

        Json::Value& entry = selection[selection.size()];
        entry["slot_index"] = i;
        entry["feature"] =
            std::string(bim::game::to_simple_string(m_task[i].feature));
        assignment_wish[i] = m_task[i].feature;
      }

  m_connection = bim::business::post(
      BIM_BUSINESS_SERVER_URL "/client/game-feature/assign-slots",
      m_request_headers.headers, body,
      [this, sent = assignment_wish]()
        {
          for (std::size_t i = 0, n = sent.size(); i != n; ++i)
            if (sent[i])
              m_player_profile.slot_feature[i] = *sent[i];

          assign_success();
        },
      [this]()
        {
          assign_error();
        });
}

void bim::app::assign_feature_slots_job::clear_success()
{
  ic_log(iscool::log::nature::info(), "assign_feature_slots_job",
         "Slot cleared.");

  done();
}

void bim::app::assign_feature_slots_job::assign_success()
{
  ic_log(iscool::log::nature::info(), "assign_feature_slots_job",
         "Slots assigned.");

  done();
}

void bim::app::assign_feature_slots_job::clear_error()
{
  ic_log(iscool::log::nature::error(), "assign_feature_slots_job",
         "Failed to clear the slot.");

  bim::app::error(m_analytics, "clear-slot");
  done();
}

void bim::app::assign_feature_slots_job::assign_error()
{
  ic_log(iscool::log::nature::error(), "assign_feature_slots_job",
         "Failed to assign the slots.");

  bim::app::error(m_analytics, "assign-slot");
  done();
}

void bim::app::assign_feature_slots_job::done()
{
  m_connection.disconnect();
  start_task();
  m_done();
}
