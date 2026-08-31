// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/app/constant/game_feature_slot_count.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <iscool/http/request_connection.hpp>
#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <array>
#include <optional>

namespace bim::business
{
  class request_headers;
}

namespace bim::app
{
  class analytics_service;
  class player_profile;

  class assign_feature_slots_job
  {
    DECLARE_SIGNAL(void(), done, m_done)

  public:
    using slot_assignment = std::array<std::optional<bim::game::feature_flags>,
                                       g_game_feature_slot_count>;

  public:
    assign_feature_slots_job(analytics_service& analytics,
                             const bim::business::request_headers& r,
                             player_profile& p);
    ~assign_feature_slots_job();

    void assign_slots(const slot_assignment& assignment);
    void clear_slot(std::size_t i);

  private:
    enum class action_kind : std::int8_t;

    struct task
    {
      action_kind action;
      bim::game::feature_flags feature;
    };

  private:
    void start_task();

    void send_clear_request(std::size_t i);
    void send_assign_request();

    void clear_success();
    void clear_error();
    void assign_success();
    void assign_error();

    void done();

  private:
    analytics_service& m_analytics;
    const bim::business::request_headers& m_request_headers;
    player_profile& m_player_profile;

    std::array<task, g_game_feature_slot_count> m_task;

    iscool::http::request_connection m_connection;
  };
}
