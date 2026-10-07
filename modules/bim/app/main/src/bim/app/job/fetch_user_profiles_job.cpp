// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/fetch_user_profiles_job.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/business/user_profile.hpp>
#include <bim/app/business/user_profiles.hpp>

#include <bim/business/business_url.hpp>
#include <bim/business/post.hpp>
#include <bim/business/request_headers.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

IMPLEMENT_SIGNAL(bim::app::fetch_user_profiles_job, done, m_done)

bim::app::fetch_user_profiles_job::fetch_user_profiles_job(
    analytics_service& analytics, const bim::business::request_headers& r)
  : m_analytics(analytics)
  , m_request_headers(r)
  , m_request_pool(4)
{}

bim::app::fetch_user_profiles_job::~fetch_user_profiles_job() = default;

void bim::app::fetch_user_profiles_job::start(
    std::span<const bim::net::user_id> users)
{
  Json::Value body;
  Json::Value& user_ids = body["user_ids"];

  for (const bim::net::user_id id : users)
    user_ids[user_ids.size()] = id;

  const iscool::http::request_connection_pool::slot slot =
      m_request_pool.pick_available();

  *slot.value = bim::business::post<user_profiles_response>(
      BIM_BUSINESS_SERVER_URL "/client/users/profiles",
      m_request_headers.headers, body,
      [this, s = slot.id](user_profiles_response r)
        {
          m_request_pool.release(s);
          m_done(r.profiles);
        },
      [this, s = slot.id]()
        {
          m_request_pool.release(s);
          error();
        });
}

void bim::app::fetch_user_profiles_job::error()
{
  ic_log(iscool::log::nature::error(), "fetch_user_profiles_job",
         "Failed to fetch the profiles.");

  bim::app::error(m_analytics, "user-profiles");
}
