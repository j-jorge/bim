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
    void stop();

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
