// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/push_legacy_state_job.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/business/legacy_state.hpp>
#include <bim/app/business_url.hpp>
#include <bim/app/preference/legacy.hpp>

#include <bim/business/post.hpp>
#include <bim/business/request_headers.hpp>

#include <bim/game/feature_flags_string.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/preferences/local_preferences.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

IMPLEMENT_SIGNAL(bim::app::push_legacy_state_job, done, m_done)

bim::app::push_legacy_state_job::push_legacy_state_job(
    analytics_service& analytics, const bim::business::request_headers& r,
    iscool::preferences::local_preferences& preferences)
  : m_analytics(analytics)
  , m_request_headers(r)
  , m_preferences(preferences)
{}

bim::app::push_legacy_state_job::~push_legacy_state_job() = default;

void bim::app::push_legacy_state_job::start()
{
  if (legacy_inventory_pushed(m_preferences))
    {
      m_schedule_connection = iscool::schedule::delayed_call(
          [this]()
            {
              m_done();
            });
      return;
    }

  ic_log(iscool::log::nature::info(), "push_legacy_state_job",
         "Sending the legacy state to the backend.");

  Json::Value body;
  body["coins"] = m_preferences.get_value("coins", (std::int64_t)0);

  {
    Json::Value& slots = body["slots"];

    if (m_preferences.get_value("feature_flags.slot_0.available", false))
      slots.append(0);

    if (m_preferences.get_value("feature_flags.slot_1.available", false))
      slots.append(1);
  }

  {
    Json::Value& game_features = body["game_features"];
    const bim::game::feature_flags owned_features =
        (bim::game::feature_flags)m_preferences.get_value(
            "feature_flags.available",
            std::int64_t(bim::game::feature_flags{}));

    for (const bim::game::feature_flags f :
         bim::game::g_all_game_feature_flags)
      if (!!(owned_features & f))
        game_features.append(std::string(bim::game::to_simple_string(f)));
  }

  {
    Json::Value& game_feature_selection = body["game_feature_selection"];

    {
      const bim::game::feature_flags f =
          (bim::game::feature_flags)m_preferences.get_value(
              "feature_flags.slot_0",
              std::int64_t(bim::game::feature_flags{}));
      Json::Value& selection =
          game_feature_selection[game_feature_selection.size()];

      selection["slot_index"] = 0;
      selection["feature"] = std::string(bim::game::to_simple_string(f));
    }

    {
      const bim::game::feature_flags f =
          (bim::game::feature_flags)m_preferences.get_value(
              "feature_flags.slot_1",
              std::int64_t(bim::game::feature_flags{}));
      Json::Value& selection =
          game_feature_selection[game_feature_selection.size()];

      selection["slot_index"] = 1;
      selection["feature"] = std::string(bim::game::to_simple_string(f));
    }
  }

  {
    Json::Value& arena_stats = body["arena_stats"];

    arena_stats["game_count"] =
        m_preferences.get_value("arena_stats.game_count", (std::int64_t)0);
    arena_stats["victory_count"] =
        m_preferences.get_value("arena_stats.victory_count", (std::int64_t)0);
    arena_stats["defeat_count"] =
        m_preferences.get_value("arena_stats.defeat_count", (std::int64_t)0);
  }

  m_http_connection = bim::business::post<legacy_state_transfer_response>(
      BIM_BUSINESS_SERVER_URL "/client/transfer-legacy-inventory",
      m_request_headers.headers, body,
      [this](const legacy_state_transfer_response& response)
        {
          success(response);
        },
      [this]()
        {
          error();
        });
}

void bim::app::push_legacy_state_job::success(
    const legacy_state_transfer_response& response)
{
  ic_log(iscool::log::nature::info(), "push_legacy_state_job",
         "Legacy state sent: {}.", (std::int64_t)response.transfer_state);

  if ((response.transfer_state == transfer_result::done)
      || (response.transfer_state == transfer_result::already_done))
    legacy_inventory_pushed(m_preferences, true);

  m_done();
}

void bim::app::push_legacy_state_job::error()
{
  ic_log(iscool::log::nature::error(), "push_legacy_state_job",
         "Failed to send the legacy state to the backend.");

  bim::app::error(m_analytics, "push-legacy");

  m_done();
}
