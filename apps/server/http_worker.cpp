// SPDX-License-Identifier: AGPL-3.0-only
#include "http_worker.hpp"

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>

#include <curl/curl.h>

#include <cassert>
#include <utility>

static size_t write_callback(char* ptr, size_t, size_t nmemb, void* userdata)
{
  std::vector<char>& response =
      *reinterpret_cast<std::vector<char>*>(userdata);

  response.insert(response.end(), ptr, ptr + nmemb);

  return nmemb;
}

class http_worker::thread
{
public:
  explicit thread(http_worker::thread_shared& shared)
    : m_thread_shared(shared)
  {}

  void operator()()
  {
    m_curl = curl_easy_init();

    if (!m_curl)
      {
        ic_log(iscool::log::nature::error(), "http_worker",
               "Failed to create CURL handle.");
        return;
      }

    while (true)
      {
        std::unique_lock lock(m_thread_shared.in_mutex);

        m_thread_shared.data_available.wait(
            lock,
            [this]()
              {
                return m_thread_shared.quit
                       || !m_thread_shared.request_queue.empty();
              });

        if (m_thread_shared.quit)
          return;

        iscool::http::request request =
            std::move(m_thread_shared.request_queue[0]);
        m_thread_shared.request_queue.erase(
            m_thread_shared.request_queue.begin());

        lock.unlock();
        process_request(std::move(request));
      }

    curl_easy_cleanup(m_curl);
  }

private:
  void process_request(iscool::http::request request)
  {
    std::vector<char> response_data;
    curl_slist* headers = nullptr;

    auto queue_result = [&, this](int response_code)
      {
        if (headers)
          curl_slist_free_all(headers);

        std::lock_guard lock(m_thread_shared.out_mutex);
        m_thread_shared.result_queue.emplace_back(
            std::move(request.result_handler), std::move(response_data),
            response_code);
      };

    curl_easy_reset(m_curl);

    CURLcode result =
        curl_easy_setopt(m_curl, CURLOPT_URL, request.url.c_str());

    if (result != CURLE_OK)
      {
        ic_log(iscool::log::nature::error(), "http_thread",
               "Failed to set URL {}: {}", request.url,
               curl_easy_strerror(result));
        return queue_result(0);
      }

    result = curl_easy_setopt(m_curl, CURLOPT_REDIR_PROTOCOLS_STR, "https");

    if (result != CURLE_OK)
      {
        ic_log(iscool::log::nature::error(), "http_thread",
               "Failed to set redirect protocols: {}",
               curl_easy_strerror(result));
        return queue_result(0);
      }

    result = curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, CURLFOLLOW_ALL);

    if (result != CURLE_OK)
      {
        ic_log(iscool::log::nature::error(), "http_thread",
               "Failed to enable redirect: {}", curl_easy_strerror(result));
        return queue_result(0);
      }

    for (const std::string& header : request.headers)
      headers = curl_slist_append(headers, header.c_str());

    if (headers)
      {
        result = curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);

        if (result != CURLE_OK)
          {
            ic_log(iscool::log::nature::error(), "http_thread",
                   "Failed to set HTTP headers: {}",
                   curl_easy_strerror(result));
            return queue_result(0);
          }
      }

    if (request.request_type == iscool::http::request::type::post)
      {
        result =
            curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, request.body.c_str());

        if (result != CURLE_OK)
          {
            ic_log(iscool::log::nature::error(), "http_thread",
                   "Failed to set POST fields: {}",
                   curl_easy_strerror(result));
            return queue_result(0);
          }
      }

    response_data.reserve(512);
    result = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &response_data);

    if (result != CURLE_OK)
      {
        ic_log(iscool::log::nature::error(), "http_thread",
               "Failed to pass write data: {}", curl_easy_strerror(result));
        return queue_result(0);
      }

    result = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_callback);

    if (result != CURLE_OK)
      {
        ic_log(iscool::log::nature::error(), "http_thread",
               "Failed to pass write callback: {}",
               curl_easy_strerror(result));
        return queue_result(0);
      }

    result = curl_easy_perform(m_curl);

    if (result != CURLE_OK)
      {
        ic_log(iscool::log::nature::error(), "http_thread",
               "curl_easy_perform() failed: {}", curl_easy_strerror(result));
        return queue_result(0);
      }

    long response_code;
    result = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &response_code);

    if (result != CURLE_OK)
      {
        ic_log(iscool::log::nature::error(), "http_thread",
               "Failed to get response code: {}", curl_easy_strerror(result));
        return queue_result(0);
      }

    queue_result(response_code);
  }

private:
  http_worker::thread_shared& m_thread_shared;
  CURL* m_curl;
};

http_worker::http_worker()
{
  const CURLcode result = curl_global_init(CURL_GLOBAL_ALL);

  if (result != CURLE_OK)
    {
      ic_log(iscool::log::nature::error(), "http_worker",
             "Failed to initialize CURL {}.", curl_easy_strerror(result));
      return;
    }

  m_thread_shared.quit = false;
  m_thread_shared.request_queue.reserve(8);
  m_thread = std::thread(thread(m_thread_shared));
}

http_worker::~http_worker()
{
  {
    const std::lock_guard lock(m_thread_shared.in_mutex);
    m_thread_shared.quit = true;
    m_thread_shared.request_queue.clear();
  }

  m_thread_shared.data_available.notify_all();

  if (m_thread.joinable())
    m_thread.join();

  curl_global_cleanup();
}

void http_worker::push(iscool::http::request request)
{
  {
    const std::lock_guard lock(m_thread_shared.in_mutex);
    m_thread_shared.request_queue.push_back(std::move(request));
  }

  m_thread_shared.data_available.notify_one();
}

void http_worker::dispatch_responses()
{
  assert(m_result_queue.empty());

  {
    std::lock_guard lock(m_thread_shared.out_mutex);
    m_result_queue.swap(m_thread_shared.result_queue);
  }

  for (const result& r : m_result_queue)
    r.response_handler(iscool::http::response(r.status, r.data));

  m_result_queue.clear();
}
