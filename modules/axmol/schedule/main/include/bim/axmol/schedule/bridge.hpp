#pragma once

#include <bim/axmol/ref_ptr.hpp>

#include <chrono>
#include <functional>
#include <mutex>

namespace ax
{
  class Node;
}

namespace bim::axmol::schedule
{
  class bridge
  {
  public:
    bridge();
    ~bridge();

    bridge(const bridge&) = delete;
    bridge& operator=(const bridge&) = delete;

  private:
    class scheduler_target_node;

  private:
    void delayed_call(const std::function<void()>& f,
                      const std::chrono::nanoseconds& delay);

    void tick();

  private:
    std::mutex m_mutex;
    std::chrono::nanoseconds m_last_tick_date;
    std::vector<std::uint32_t> m_scheduled_delays_ns;
    std::vector<std::function<void()>> m_scheduled_functions;
    std::vector<std::function<void()>> m_scheduled_functions_scratch;
    bim::axmol::ref_ptr<ax::Node> m_target;
  };
}
