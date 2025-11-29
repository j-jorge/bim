// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/flow.hpp>

#include <bim/axmol/input/key_event_view.hpp>
#include <bim/axmol/input/node.hpp>
#include <bim/axmol/input/touch_event.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <axmol/base/Director.h>
#include <axmol/base/Event.h>
#include <axmol/base/EventDispatcher.h>
#include <axmol/base/EventListenerTouch.h>

bim::axmol::input::flow::flow(ax::Scene& scene, node& root)
  : m_root(root)
  , m_pressed_ids(0)
  , m_key_listener(ax::EventListenerKeyboard::create())
  , m_touch_listener(ax::EventListenerTouchOneByOne::create())
{
  m_key_listener->onKeyPressed = AX_CALLBACK_2(flow::key_pressed, this);
  m_key_listener->onKeyReleased = AX_CALLBACK_2(flow::key_released, this);

  ax::EventDispatcher& dispatcher(
      *ax::Director::getInstance()->getEventDispatcher());

  dispatcher.addEventListenerWithSceneGraphPriority(m_key_listener.get(),
                                                    &scene);

  m_touch_listener->onTouchBegan = AX_CALLBACK_2(flow::touch_began, this);
  m_touch_listener->onTouchMoved = AX_CALLBACK_2(flow::touch_moved, this);
  m_touch_listener->onTouchEnded = AX_CALLBACK_2(flow::touch_ended, this);
  m_touch_listener->onTouchCancelled =
      AX_CALLBACK_2(flow::touch_cancelled, this);

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

bool bim::axmol::input::flow::touch_began(ax::Touch* touch, ax::Event* event)
{
  event->stopPropagation();

  if (known_touch(touch))
    return true;

  new_pressed_touch(touch);

  touch_event t(touch);
  m_root.touch_pressed(t);

  return true;
}

bool bim::axmol::input::flow::touch_moved(ax::Touch* touch, ax::Event* event)
{
  event->stopPropagation();

  if (!known_touch(touch))
    return true;

  touch_event t(touch);
  m_root.touch_moved(t);

  return true;
}

bool bim::axmol::input::flow::touch_ended(ax::Touch* touch, ax::Event* event)
{
  event->stopPropagation();

  if (!known_touch(touch))
    return true;

  released_touch(touch);

  touch_event t(touch);
  m_root.touch_released(t);

  return true;
}

bool bim::axmol::input::flow::touch_cancelled(ax::Touch* touch,
                                              ax::Event* event)
{
  event->stopPropagation();

  if (!known_touch(touch))
    return true;

  released_touch(touch);

  touch_event t(touch);
  m_root.touch_cancelled(t);

  return true;
}

void bim::axmol::input::flow::new_pressed_touch(ax::Touch* touch)
{
  m_pressed_ids |= (1 << touch->getID());
}

bool bim::axmol::input::flow::known_touch(ax::Touch* touch)
{
  return m_pressed_ids & (1 << touch->getID());
}

void bim::axmol::input::flow::released_touch(ax::Touch* touch)
{
  m_pressed_ids &= ~(1 << touch->getID());
}
