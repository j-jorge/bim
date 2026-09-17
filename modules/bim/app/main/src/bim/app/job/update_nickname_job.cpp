// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/update_nickname_job.hpp>

#include <bim/app/analytics/coins_transaction.hpp>
#include <bim/app/analytics/error.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/config.hpp>

#include <bim/business/business_url.hpp>
#include <bim/business/post.hpp>
#include <bim/business/request_headers.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

IMPLEMENT_SIGNAL(bim::app::update_nickname_job, done, m_done)
IMPLEMENT_SIGNAL(bim::app::update_nickname_job, error, m_error)

bim::app::update_nickname_job::update_nickname_job(
    analytics_service& analytics, const bim::business::request_headers& r,
    player_profile& p)
  : m_analytics(analytics)
  , m_request_headers(r)
  , m_player_profile(p)
{}

bim::app::update_nickname_job::~update_nickname_job() = default;

void bim::app::update_nickname_job::start(std::string nickname)
{
  Json::Value body;
  body["nickname"] = nickname;

  m_connection = bim::business::post<update_nickname_response>(
      BIM_BUSINESS_SERVER_URL "/client/account/update-nickname",
      m_request_headers.headers, body,
      [this,
       n = std::move(nickname)](const update_nickname_response& r) mutable
        {
          success(std::move(n), r);
        },
      [this]()
        {
          error();
        });
}

void bim::app::update_nickname_job::success(std::string nickname,
                                            const update_nickname_response& r)
{
  m_player_profile.nickname = std::move(nickname);
  m_player_profile.nickname_change_allowed_date =
      r.nickname_change_allowed_date;

  m_done();
}

void bim::app::update_nickname_job::error()
{
  ic_log(iscool::log::nature::error(), "update_nickname_job",
         "Failed to change the nickname.");

  bim::app::error(m_analytics, "update-nickname");

  m_error();
}
