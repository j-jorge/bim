// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/monitoring/declare_state_monitor.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <iscool/language_name_fwd.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace bim::axmol::app
{
  class matchmaking_wait_message
  {
    DECLARE_SIGNAL(void(std::string_view), updated, m_updated)

  public:
    matchmaking_wait_message();
    ~matchmaking_wait_message();

    void start();
    void pause();
    void stop();

  private:
    using vector_of_strings = std::vector<std::string>;

  private:
    void load_messages(iscool::language_name language);

    void start_default_script();
    void next_script();

    void schedule_tick();
    void tick();
    void dispatch_updated();

  private:
    ic_declare_state_monitor(m_monitor);

    std::vector<vector_of_strings> m_scripts;
    std::size_t m_current_script;
    std::size_t m_current_line;
    std::size_t m_remaining_loops_on_default_script;
    std::size_t m_next_script;

    iscool::signals::scoped_connection m_connection;
  };
}
