// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/fetch_player_profile_job.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/business/player_profile_json.hpp>
#include <bim/app/business_url.hpp>

#include <bim/business/post.hpp>
#include <bim/business/request_headers.hpp>

#include <bim/bit_map.impl.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

IMPLEMENT_SIGNAL(bim::app::fetch_player_profile_job, done, m_done)

bim::app::fetch_player_profile_job::fetch_player_profile_job(
    analytics_service& analytics, const bim::business::request_headers& r)
  : m_analytics(analytics)
  , m_request_headers(r)
{}

bim::app::fetch_player_profile_job::~fetch_player_profile_job() = default;

void bim::app::fetch_player_profile_job::start(player_profile& profile)
{
  ic_log(iscool::log::nature::info(), "fetch_player_profile_job",
         "Fetching the player's profile.");

  m_player_profile = &profile;

  m_connection = bim::business::post(
      profile, BIM_BUSINESS_SERVER_URL "/client/me", m_request_headers.headers,
      [this]()
        {
          success();
        },
      [this]()
        {
          error();
        });
}

void bim::app::fetch_player_profile_job::success()
{
  ic_log(iscool::log::nature::info(), "fetch_player_profile_job",
         "Player profile received.");

  m_player_profile = nullptr;

  m_done();
}

void bim::app::fetch_player_profile_job::error()
{
  ic_log(iscool::log::nature::error(), "fetch_player_profile_job",
         "Failed to fetch the player's profile.");

  *m_player_profile = {};
  m_player_profile = nullptr;

  bim::app::error(m_analytics, "fetch-player-profile");

  m_done();
}
