// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/buy_feature_job.hpp>

#include <bim/app/analytics/coins_transaction.hpp>
#include <bim/app/analytics/error.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/business_url.hpp>
#include <bim/app/config.hpp>

#include <bim/business/post.hpp>
#include <bim/business/request_headers.hpp>

#include <bim/game/feature_flags_string.hpp>

#include <bim/bit_map.impl.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

IMPLEMENT_SIGNAL(bim::app::buy_feature_job, done, m_done)

bim::app::buy_feature_job::buy_feature_job(
    analytics_service& analytics, const bim::business::request_headers& r,
    const config& cfg, player_profile& p)
  : m_analytics(analytics)
  , m_request_headers(r)
  , m_config(cfg)
  , m_player_profile(p)
  , m_request_pool(4)
{}

bim::app::buy_feature_job::~buy_feature_job() = default;

void bim::app::buy_feature_job::start(bim::game::feature_flags f)
{
  Json::Value body;
  body["feature_name"] = std::string(bim::game::to_simple_string(f));

  iscool::http::request_connection_pool::slot slot =
      m_request_pool.pick_available();

  *slot.value = bim::business::post(
      BIM_BUSINESS_SERVER_URL "/client/game-feature/buy-feature",
      m_request_headers.headers, body,
      [this, f, s = slot.id]()
        {
          m_request_pool.release(s);
          success(f);
        },
      [this, s = slot.id]()
        {
          m_request_pool.release(s);
          error();
        });
}

void bim::app::buy_feature_job::success(bim::game::feature_flags f)
{
  const std::int16_t price = m_config.game_feature_price[f];

  m_player_profile.coins -= price;
  m_player_profile.available_features |= f;

  coins_transaction(m_analytics, "feature-item", -price);

  m_done(f);
}

void bim::app::buy_feature_job::error()
{
  ic_log(iscool::log::nature::error(), "buy_feature_job",
         "Failed to buy the feature.");

  bim::app::error(m_analytics, "buy-feature");
}
