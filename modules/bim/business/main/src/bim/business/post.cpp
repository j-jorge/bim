// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/business/post.hpp>

#include <iscool/json/parse_string.hpp>
#include <iscool/json/write_to_string.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>

#include <json/value.h>

bool bim::business::detail::body_to_json(Json::Value& json_body,
                                         std::span<const char> body)
{
  if (!body.empty())
    {
      json_body = iscool::json::parse_string(std::string_view(body));

      if (json_body != Json::nullValue)
        return true;
    }

  ic_log(iscool::log::nature::error(), "business_interface",
         "Could not parse response: {}.", std::string_view(body));

  return false;
}

void bim::business::detail::log_request_error(std::string_view url, int status,
                                              std::span<const char> body)
{
  ic_log(iscool::log::nature::error(), "business_interface",
         "Request failed for '{}': {}, {}.", url, status,
         std::string_view(body));
}

void bim::business::detail::log_parse_error(std::string_view url,
                                            std::string_view error_message,
                                            std::span<const char> body)
{
  ic_log(iscool::log::nature::error(), "business_interface",
         "Could not parse response of '{}': {}. Body was {}", url,
         error_message, std::string_view(body));
}

std::string bim::business::detail::json_body_to_string(std::string_view url,
                                                       const Json::Value& body)
{
  std::string body_string;

  if (iscool::json::write_to_string(body_string, body))
    ic_log(iscool::log::nature::error(), "post",
           "Could not convert to body to string, url is {}.", url);

  return body_string;
}
