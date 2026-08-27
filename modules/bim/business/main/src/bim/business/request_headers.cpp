// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/business/request_headers.hpp>

#include <iscool/http/json/headers.hpp>

#include <utility>

bim::business::request_headers::request_headers()
{
  // Keep some room for a future authorization token.
  headers.reserve(3);

  headers.emplace_back(iscool::http::json::headers::content_type);
  headers.emplace_back(iscool::http::json::headers::accept);
}

bim::business::request_headers::request_headers(
    std::string_view authorization_token)
  : request_headers()
{
  push_authorization(authorization_token);
}

void bim::business::request_headers::push_authorization(
    std::string_view authorization_token)
{
  constexpr std::string_view authorization_key = "Authorization: ";
  std::string authorization;
  authorization.reserve(authorization_key.size() + authorization_token.size());
  authorization = authorization_key;
  authorization += authorization_token;

  headers.emplace_back(std::move(authorization));
}
