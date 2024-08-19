// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/flow.hpp>

#include <bim/axmol/input/key_event_view.hpp>
#include <bim/axmol/input/node.hpp>
#include <bim/axmol/input/touch_event_view.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <axmol/base/Director.h>
#include <axmol/base/Event.h>
#include <axmol/base/EventDispatcher.h>
#include <axmol/base/EventListenerTouch.h>

bim::axmol::input::flow::flow(ax::Scene& scene, node& root)
  : m_root(root)
  , m_key_listener(ax::EventListenerKeyboard::create())
  , m_touch_listener(ax::EventListenerTouchAllAtOnce::create())
{
  m_key_listener->onKeyPressed = AX_CALLBACK_2(flow::key_pressed, this);
  m_key_listener->onKeyReleased = AX_CALLBACK_2(flow::key_released, this);

  ax::EventDispatcher& dispatcher(
      *ax::Director::getInstance()->getEventDispatcher());

  dispatcher.addEventListenerWithSceneGraphPriority(m_key_listener.get(),
                                                    &scene);

  m_touch_listener->onTouchesBegan = AX_CALLBACK_2(flow::touches_began, this);
  m_touch_listener->onTouchesMoved = AX_CALLBACK_2(flow::touches_moved, this);
  m_touch_listener->onTouchesEnded = AX_CALLBACK_2(flow::touches_ended, this);
  m_touch_listener->onTouchesCancelled =
      AX_CALLBACK_2(flow::touches_cancelled, this);

  dispatcher.addEventListenerWithSceneGraphPriority(m_touch_listener.get(),
                                                    &scene);
}

bim::axmol::input::flow::~flow()
{
  ax::EventDispatcher& dispatcher(
      *ax::Director::getInstance()->getEventDispatcher());

  assert(m_key_listener != nullptr);
  dispatcher.removeEventListener(m_key_listener.get());

  assert(m_touch_listener != nullptr);
  dispatcher.removeEventListener(m_touch_listener.get());
}

void bim::axmol::input::flow::key_pressed(ax::EventKeyboard::KeyCode key,
                                          ax::Event* event)
{
  event->stopPropagation();

  m_key_event_storage.clear();
  m_key_event_storage.push_back(key_event(key));

  key_event_view events(m_key_event_storage);
  m_root.key_pressed(events);
}

void bim::axmol::input::flow::key_released(ax::EventKeyboard::KeyCode key,
                                           ax::Event* event)
{
  event->stopPropagation();

  m_key_event_storage.clear();
  m_key_event_storage.push_back(key_event(key));

  key_event_view events(m_key_event_storage);
  m_root.key_released(events);
}

void bim::axmol::input::flow::touches_began(
    const std::vector<ax::Touch*>& touches, ax::Event* event)
{
  event->stopPropagation();

  new_pressed_touches(touches);

  if (m_touch_event_storage.empty())
    return;

  bim::axmol::input::touch_event_view events(m_touch_event_storage);
  m_root.touch_pressed(events);
}

void bim::axmol::input::flow::touches_moved(
    const std::vector<ax::Touch*>& touches, ax::Event* event)
{
  event->stopPropagation();

  known_moving_touches(touches);

  if (m_touch_event_storage.empty())
    return;

  bim::axmol::input::touch_event_view events(m_touch_event_storage);
  m_root.touch_moved(events);
}

void bim::axmol::input::flow::touches_ended(
    const std::vector<ax::Touch*>& touches, ax::Event* event)
{
  event->stopPropagation();

  known_released_touches(touches);

  if (m_touch_event_storage.empty())
    return;

  bim::axmol::input::touch_event_view events(m_touch_event_storage);
  m_root.touch_released(events);
}

void bim::axmol::input::flow::touches_cancelled(
    const std::vector<ax::Touch*>& touches, ax::Event* event)
{
  event->stopPropagation();

  known_released_touches(touches);

  if (m_touch_event_storage.empty())
    return;

  bim::axmol::input::touch_event_view events(m_touch_event_storage);
  m_root.touch_cancelled(events);
}

void bim::axmol::input::flow::new_pressed_touches(
    const std::vector<ax::Touch*>& touches)
{
  m_touch_event_storage.clear();

  for (ax::Touch* const touch : touches)
    if (m_pressed_ids.insert(touch->getID()).second)
      m_touch_event_storage.emplace_back(touch);
}

void bim::axmol::input::flow::known_moving_touches(
    const std::vector<ax::Touch*>& touches)
{
  m_touch_event_storage.clear();

  for (ax::Touch* const touch : touches)
    if (m_pressed_ids.find(touch->getID()) != m_pressed_ids.end())
      m_touch_event_storage.emplace_back(touch);
}

void bim::axmol::input::flow::known_released_touches(
    const std::vector<ax::Touch*>& touches)
{
  m_touch_event_storage.clear();

  for (ax::Touch* const touch : touches)
    if (m_pressed_ids.erase(touch->getID()) == 1)
      m_touch_event_storage.emplace_back(touch);
}
