// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/action/runner.hpp>

#include <axmol/2d/ActionManager.h>
#include <axmol/2d/Node.h>
#include <axmol/base/Director.h>
#include <axmol/base/Scheduler.h>

#include <fmt/format.h>

#include <cassert>

bim::axmol::action::runner::runner()
  : m_action_manager(new ax::ActionManager())
  , m_target(new ax::Node())
  , m_scheduler_key(fmt::format("{}", (void*)this))
  , m_update_scheduled(false)
{
  m_target->setActionManager(m_action_manager.get());
  m_target->onEnter();
}

bim::axmol::action::runner::~runner()
{
  if (m_update_scheduled)
    stop();

  m_target->onExit();
}

void bim::axmol::action::runner::run(ax::Action& action)
{
  m_target->runAction(&action);

  if (m_update_scheduled)
    return;

  ax::Director::getInstance()->getScheduler()->schedule(
      [this](float elapsed_time)
      {
        update(elapsed_time);
      },
      this, 0, false, m_scheduler_key);

  m_update_scheduled = true;
}

void bim::axmol::action::runner::stop()
{
  m_action_manager->removeAllActionsFromTarget(m_target.get());

  if (m_update_scheduled)
    unschedule_update();
}

void bim::axmol::action::runner::update(float elapsed_time)
{
  if (m_action_manager->getNumberOfRunningActionsInTarget(m_target.get()) == 0)
    unschedule_update();
  else
    m_action_manager->update(elapsed_time);
}

void bim::axmol::action::runner::unschedule_update()
{
  assert(m_update_scheduled);

  ax::Director::getInstance()->getScheduler()->unschedule(m_scheduler_key,
                                                          this);

  m_update_scheduled = false;
}
