// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/business_registration_service.hpp>

#include <bim/server/config.hpp>

#include <bim/net/message/protocol_version.hpp>

#include <bim/assume.hpp>
#include <bim/version.hpp>

#include <iscool/http/json/headers.hpp>
#include <iscool/http/send.hpp>
#include <iscool/json/cast_uint.hpp>
#include <iscool/json/is_of_type_uint.hpp>
#include <iscool/json/parse_string.hpp>
#include <iscool/json/write_to_string.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>

#include <json/value.h>

bim::server::business_registration_service::business_registration_service(
    const config& config)
  : m_url(config.business_url + "gs/hello")
  , m_retry_delay(0)
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

  m_headers.reserve(2);
  m_headers.emplace_back("Authorization: " + config.business_token);
  m_headers.emplace_back(iscool::http::json::headers::content_type);

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
  m_request_connections = iscool::http::post(
      m_url, m_headers, m_body,
      [this](std::span<const char> body)
        {
          hello_ok(body);
        },
      [this](std::span<const char> body)
        {
          hello_ko(body);
        });
}

void bim::server::business_registration_service::hello_ok(
    std::span<const char> body)
{
  if (!body.empty())
    {
      const Json::Value response(iscool::json::parse_string(
          std::string_view(body.data(), body.size())));

      if (response != Json::nullValue)
        {
          const Json::Value json_delay = response["callback_delay_seconds"];

          if (iscool::json::is_of_type<std::uint32_t>(json_delay))
            {
              const std::chrono::seconds delay(
                  iscool::json::cast<std::uint32_t>(
                      response["callback_delay_seconds"]));

              ic_log(iscool::log::nature::info(),
                     "business_registration_service",
                     "Registered. Next registration in {}.", delay);

              m_retry_delay = {};
              schedule_registration(delay);
              return;
            }
        }
    }

  increment_retry_delay();

  ic_log(iscool::log::nature::error(), "business_registration_service",
         "Could not parse registration result: {}. Retrying in {}.", body,
         m_retry_delay);

  schedule_registration(m_retry_delay);
}

void bim::server::business_registration_service::hello_ko(
    std::span<const char> body)
{
  increment_retry_delay();

  ic_log(iscool::log::nature::error(), "business_registration_service",
         "Failed to register: {}. Retrying in {}.", body, m_retry_delay);

  schedule_registration(m_retry_delay);
}

void bim::server::business_registration_service::increment_retry_delay()
{
  if (m_retry_delay < std::chrono::minutes(1))
    m_retry_delay += std::chrono::seconds(5);
}
