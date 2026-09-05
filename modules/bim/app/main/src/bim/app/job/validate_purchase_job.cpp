// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/validate_purchase_job.hpp>

#include <bim/app/analytics/coins_transaction.hpp>
#include <bim/app/analytics/error.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/business/purchase_validation_status.hpp>
#include <bim/app/business/validate_purchase.hpp>
#include <bim/app/business_url.hpp>

#include <bim/business/post.hpp>
#include <bim/business/request_headers.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

IMPLEMENT_SIGNAL(bim::app::validate_purchase_job, done, m_done)
IMPLEMENT_SIGNAL(bim::app::validate_purchase_job, error, m_error)

bim::app::validate_purchase_job::validate_purchase_job(
    analytics_service& analytics, const bim::business::request_headers& r,
    player_profile& p)
  : m_analytics(analytics)
  , m_request_headers(r)
  , m_player_profile(p)
  , m_request_pool(1)
{}

bim::app::validate_purchase_job::~validate_purchase_job() = default;

void bim::app::validate_purchase_job::start(std::string purchase_token)
{
  Json::Value body;
  body["token"] = std::move(purchase_token);

  iscool::http::request_connection_pool::slot slot =
      m_request_pool.pick_available();

  *slot.value = bim::business::post<validate_purchase_response>(
      BIM_BUSINESS_SERVER_URL "/client/billing/validate-purchase",
      m_request_headers.headers, body,
      [this, s = slot.id](const validate_purchase_response& r)
        {
          m_request_pool.release(s);
          success(r);
        },
      [this, s = slot.id]()
        {
          m_request_pool.release(s);
          error();
        });
}

void bim::app::validate_purchase_job::success(
    const validate_purchase_response& r)
{
  if (r.status == purchase_validation_status::ok)
    {
      m_player_profile.coins += r.coins;
      coins_transaction(m_analytics, "purchase-completed", r.coins);
    }

  m_done(r.status);
}

void bim::app::validate_purchase_job::error()
{
  ic_log(iscool::log::nature::error(), "validate_purchase_job",
         "Failed to validate the purchase.");

  bim::app::error(m_analytics, "purchase-validation");
}
