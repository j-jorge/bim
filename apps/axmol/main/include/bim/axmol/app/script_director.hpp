// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <string>
#include <vector>

namespace ax
{
  class Touch;
}

namespace bim::axmol::app
{
  class application_event_listener;
  class main_scene;

  class script_director
  {
  public:
    enum result
    {
      ok,
      fail
    };

    DECLARE_SIGNAL(void(result), done, m_done)

  public:
    script_director(const application_event_listener& events,
                    const std::string& script_file, bool number_screenshots);
    ~script_director();

  private:
    enum class step_kind : std::uint8_t;
    struct click_target;
    struct step;

  private:
    void schedule_tick();
    void tick();

    void capture(const std::string& file_name) const;

    void click(const std::string& node_path) const;
    void press(ax::Touch& touch) const;
    void release(ax::Touch& touch) const;

    void check_event(std::string_view name);

    void schedule_timeout();
    void timeout();

  private:
    std::vector<step> m_steps;
    std::vector<std::string> m_wait_steps;
    std::vector<click_target> m_click_steps;
    std::vector<std::string> m_capture_steps;
    std::string_view m_pending_event;
    std::size_t m_next_step;

    const iscool::signals::scoped_connection m_event_connection;
    iscool::schedule::scoped_connection m_tick_connection;
    iscool::schedule::scoped_connection m_timeout_connection;
  };
}
