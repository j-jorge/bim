// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <memory>
#include <string>

namespace ax
{
  class Action;
  class ActionManager;
  class Node;
}

namespace bim::axmol::action
{
  class runner
  {
  public:
    runner();
    ~runner();

    void run(ax::Action& action);
    void run_complete(ax::Action& action);
    void stop();

    std::size_t running_action_count() const;
    bool running() const;

  private:
    void update(float elapsed_time);

    void unschedule_update();

  private:
    const std::unique_ptr<ax::ActionManager> m_action_manager;
    const std::unique_ptr<ax::Node> m_target;
    const std::string m_scheduler_key;
    bool m_update_scheduled;
  };
}
