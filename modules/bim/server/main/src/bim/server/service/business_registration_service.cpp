// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/business_registration_service.hpp>

#include <bim/server/business/hello.hpp>

#include <bim/server/config.hpp>

#include <bim/net/message/protocol_version.hpp>

#include <bim/business/post.hpp>
#include <bim/business/request_headers.hpp>

#include <bim/version.hpp>

#include <iscool/json/write_to_string.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>

#include <json/value.h>

bim::server::business_registration_service::business_registration_service(
    const config& config, const bim::business::request_headers& headers)
  : m_url(config.business_url + "gs/hello")
  , m_request_headers(headers)
  , m_pulse(config.business_registration_pulse_seconds)
{
  if (config.business_url.empty())
    {
      ic_log(iscool::log::nature::info(), "business_registration_service",
             "Disabled.");
      return;
    }

  ic_log(iscool::log::nature::info(), "business_registration_service",
         "Starting, business URL is '{}'.", config.business_url);

  Json::Value body;
  body["host"] = config.host + ':' + std::to_string(config.port);
  body["version"] = bim::version_major;
  body["protocol_version"] = bim::net::protocol_version;

  if (!iscool::json::write_to_string(m_body, body))
    {
      ic_log(iscool::log::nature::error(), "business_registration_service",
             "Failed to build request body. Service is disabled.");
      return;
    }

  schedule_registration(std::chrono::seconds::zero());
}

bim::server::business_registration_service::~business_registration_service() =
    default;

void bim::server::business_registration_service::schedule_registration(
    const std::chrono::seconds& delay)
{
  m_registration_connection = iscool::schedule::delayed_call(
      [this]()
        {
          send_registration_request();
        },
      delay);
}
void bim::server::business_registration_service::send_registration_request()
{
  m_request_connections = bim::business::post<business::hello_response>(
      m_url, m_request_headers.headers, m_body,
      [this](const business::hello_response& r)
        {
          schedule_registration(r.callback_delay);
        },
      [this]()
        {
          hello_ko();
        });
}

void bim::server::business_registration_service::hello_ko()
{
  ic_log(iscool::log::nature::error(), "business_registration_service",
         "Failed to register. Retrying in {}.", m_pulse);

  schedule_registration(m_pulse);
}
