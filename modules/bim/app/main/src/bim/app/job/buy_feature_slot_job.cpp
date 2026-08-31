// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/buy_feature_slot_job.hpp>

#include <bim/app/analytics/coins_transaction.hpp>
#include <bim/app/analytics/error.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/business_url.hpp>
#include <bim/app/config.hpp>

#include <bim/business/post.hpp>
#include <bim/business/request_headers.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

IMPLEMENT_SIGNAL(bim::app::buy_feature_slot_job, done, m_done)

bim::app::buy_feature_slot_job::buy_feature_slot_job(
    analytics_service& analytics, const bim::business::request_headers& r,
    const config& cfg, player_profile& p)
  : m_analytics(analytics)
  , m_request_headers(r)
  , m_config(cfg)
  , m_player_profile(p)
{}

bim::app::buy_feature_slot_job::~buy_feature_slot_job() = default;

void bim::app::buy_feature_slot_job::start(std::size_t i)
{
  // There's not many slots, and in the current state only one is
  // purchaseable. There's no reason to manage simultaneous purchases here.
  if (m_connection.connected())
    return;

  Json::Value body;
  body["slot_index"] = i;

  m_connection = bim::business::post(
      BIM_BUSINESS_SERVER_URL "/client/game-feature/buy-slot",
      m_request_headers.headers, body,
      [this, i]()
        {
          success(i);
        },
      [this]()
        {
          error();
        });
}

void bim::app::buy_feature_slot_job::success(std::size_t i)
{
  const std::int16_t price = m_config.game_feature_slot_price[i];

  m_player_profile.coins -= price;
  m_player_profile.slot_availability[i] = true;

  coins_transaction(m_analytics, "feature-slot", -price);

  m_done(i);
}

void bim::app::buy_feature_slot_job::error()
{
  ic_log(iscool::log::nature::error(), "buy_feature_slot_job",
         "Failed to buy the slot.");

  bim::app::error(m_analytics, "buy-slot");
}
