// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/send.hpp>
#include <iscool/json/bad_cast.hpp>
#include <iscool/json/parse_string.hpp>

#include <json/value.h>

#include <cassert>
#include <string>
#include <string_view>
#include <vector>

namespace bim::business
{
  namespace detail
  {
    bool body_to_json(Json::Value& json_body, std::span<const char> body);
    void log_request_body(std::string_view url, std::string_view body);
    void log_request_response(std::string_view url,
                              std::span<const char> body);
    void log_request_error(std::string_view url, int status,
                           std::span<const char> body);
    void log_parse_error(std::string_view url, std::string_view error_message,
                         std::span<const char> body);
    std::string json_body_to_string(std::string_view url,
                                    const Json::Value& body);

    template <typename ResultHandler, typename ProcessError>
    iscool::http::request_connection
    post(std::string_view url, std::vector<std::string> headers,
         std::string body, ResultHandler&& handle_result, ProcessError&& error)
    {
      assert(iscool::json::parse_string(body) != Json::nullValue);

      log_request_body(url, body);

      auto handle_error = [error = std::forward<ProcessError>(error),
                           url](int status, std::span<const char> body)
        {
          detail::log_request_error(url, status, body);
          error();
        };

      return iscool::http::post(
          std::string(url), std::move(headers), std::move(body),
          std::forward<ResultHandler>(handle_result), std::move(handle_error));
    }
  }

  template <typename Response, typename ProcessResult, typename ProcessError>
  iscool::http::request_connection
  post(Response& response_storage, std::string_view url,
       std::vector<std::string> headers, std::string body, ProcessResult&& ok,
       ProcessError&& error)
  {
    auto handle_result = [ok = std::forward<ProcessResult>(ok),
                          &response_storage, url,
                          error](std::span<const char> body)
      {
        detail::log_request_response(url, body);

        Json::Value json_body;
        bool valid = false;

        if (detail::body_to_json(json_body, body))
          try
            {
              valid = from_json(response_storage, json_body);
            }
          catch (const iscool::json::bad_cast& e)
            {
              detail::log_parse_error(url, e.what(), body);
            }

        if (valid)
          ok();
        else
          error();
      };

    return detail::post(url, std::move(headers), std::move(body),
                        std::move(handle_result),
                        std::forward<ProcessError>(error));
  }

  template <typename Response, typename ProcessResult, typename ProcessError>
  iscool::http::request_connection
  post(Response& response_storage, std::string_view url,
       std::vector<std::string> headers, const Json::Value& body,
       ProcessResult&& ok, ProcessError&& error)
  {
    std::string body_string = detail::json_body_to_string(url, body);

    return post<Response>(
        response_storage, url, std::move(headers), std::move(body_string),
        std::forward<ProcessResult>(ok), std::forward<ProcessError>(error));
  }

  template <typename Response, typename ProcessResult, typename ProcessError>
  iscool::http::request_connection
  post(Response& response_storage, std::string_view url,
       std::vector<std::string> headers, ProcessResult&& ok,
       ProcessError&& error)
  {
    return post<Response>(response_storage, url, std::move(headers),
                          Json::objectValue, std::forward<ProcessResult>(ok),
                          std::forward<ProcessError>(error));
  }

  template <typename Response, typename ProcessResult, typename ProcessError>
  iscool::http::request_connection
  post(std::string_view url, std::vector<std::string> headers,
       std::string body, ProcessResult&& ok, ProcessError&& error)
  {
    auto handle_result = [ok = std::forward<ProcessResult>(ok), url,
                          error](std::span<const char> body)
      {
        detail::log_request_response(url, body);

        Json::Value json_body;
        bool valid = false;
        Response r;

        if (detail::body_to_json(json_body, body))
          try
            {
              valid = from_json(r, json_body);
            }
          catch (const iscool::json::bad_cast& e)
            {
              detail::log_parse_error(url, e.what(), body);
            }

        if (valid)
          ok(std::move(r));
        else
          error();
      };

    return detail::post(url, std::move(headers), std::move(body),
                        std::move(handle_result),
                        std::forward<ProcessError>(error));
  }

  template <typename Response, typename ProcessResult, typename ProcessError>
  iscool::http::request_connection
  post(std::string_view url, std::vector<std::string> headers,
       const Json::Value& body, ProcessResult&& ok, ProcessError&& error)
  {
    std::string body_string = detail::json_body_to_string(url, body);

    return post<Response>(url, std::move(headers), std::move(body_string),
                          std::forward<ProcessResult>(ok),
                          std::forward<ProcessError>(error));
  }

  template <typename ProcessResult, typename ProcessError>
  iscool::http::request_connection
  post(std::string_view url, std::vector<std::string> headers,
       const Json::Value& body, ProcessResult&& ok, ProcessError&& error)
  {
    std::string body_string = detail::json_body_to_string(url, body);

    return detail::post(
        url, std::move(headers), std::move(body_string),
        [ok = std::forward<ProcessResult>(ok)](std::span<const char>)
          {
            ok();
          },
        std::forward<ProcessError>(error));
  }

  template <typename ProcessResult, typename ProcessError>
  iscool::http::request_connection
  post(std::string_view url, std::vector<std::string> headers,
       ProcessResult&& ok, ProcessError&& error)
  {
    return detail::post(
        url, std::move(headers), "{}",
        [ok = std::forward<ProcessResult>(ok)](std::span<const char>)
          {
            ok();
          },
        std::forward<ProcessError>(error));
  }

  void post(std::string_view url, std::vector<std::string> headers,
            const Json::Value& body);
}
