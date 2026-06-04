// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/request.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

class http_worker
{
public:
  http_worker();
  ~http_worker();

  void push(iscool::http::request request);
  void dispatch_responses();

private:
  struct result
  {
    iscool::http::request::response_handler response_handler;
    std::vector<char> data;
    int status;
  };

  struct thread_shared
  {
    std::vector<iscool::http::request> request_queue;
    bool quit;
    std::mutex in_mutex;
    std::condition_variable data_available;

    std::mutex out_mutex;
    std::vector<result> result_queue;
  };

  class thread;

private:
  thread_shared m_thread_shared;
  std::thread m_thread;
  std::vector<result> m_result_queue;
};
