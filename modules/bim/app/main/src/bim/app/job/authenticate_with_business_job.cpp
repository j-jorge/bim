// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/authenticate_with_business_job.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/business_url.hpp>

#include <bim/net/message/protocol_version.hpp>

#include <iscool/http/json/send.hpp>
#include <iscool/json/cast.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

IMPLEMENT_SIGNAL(bim::app::authenticate_with_business_job, done, m_done)
IMPLEMENT_SIGNAL(bim::app::authenticate_with_business_job, error, m_error)

bim::app::authenticate_with_business_job::authenticate_with_business_job(
    analytics_service& analytics)
  : m_analytics(analytics)
{}

void bim::app::authenticate_with_business_job::start(
    std::string_view device_id)
{
  ic_log(iscool::log::nature::info(), "authenticate_with_business_job",
         "Connecting to business server at '{}'.", BIM_BUSINESS_SERVER_URL);

  Json::Value body;
  body["device_id"] = std::string(device_id);

  const auto on_result = [this](const Json::Value& r)
    {
      success(r);
    };
  const auto on_error = [this](int status, std::span<const char> body)
    {
      error(status, body);
    };

  m_connection =
      iscool::http::json::post(BIM_BUSINESS_SERVER_URL "/client/authenticate",
                               body, on_result, on_error);
}

void bim::app::authenticate_with_business_job::success(const Json::Value& r)
{
  std::string session_token =
      iscool::json::cast<std::string>(r["session_token"]);

  ic_log(iscool::log::nature::info(), "authenticate_with_business_job",
         "Connected to business server. Session token is '{}'.",
         session_token);

  m_done(std::move(session_token));
}

void bim::app::authenticate_with_business_job::error(
    int status, std::span<const char> body)
{
  ic_log(iscool::log::nature::error(), "authenticate_with_business_job",
         "Failed to connect to the business server ({}): {}.", status,
         std::string_view(body.begin(), body.end()).substr(0, 1024));

  bim::app::error(m_analytics, "business-authentication");

  m_error(status);
}
