// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/key_event.hpp>
#include <bim/axmol/input/touch_event.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <axmol/base/EventListenerKeyboard.h>

#include <unordered_set>
#include <vector>

namespace ax
{
  class Event;
  class EventListener;
  class EventListenerTouchOneByOne;
  class Scene;
}

namespace bim::axmol::input
{
  class node;

  class flow : public ax::Object
  {
  public:
    flow(ax::Scene& scene, node& root);
    ~flow();

    flow(const flow&) = delete;
    flow& operator=(const flow&) = delete;

  private:
    void key_pressed(ax::EventKeyboard::KeyCode key, ax::Event* event);
    void key_released(ax::EventKeyboard::KeyCode key, ax::Event* event);

    bool touch_began(ax::Touch* touch, ax::Event* event);
    bool touch_moved(ax::Touch* touch, ax::Event* event);
    bool touch_ended(ax::Touch* touch, ax::Event* event);
    bool touch_cancelled(ax::Touch* touch, ax::Event* event);

    void new_pressed_touch(ax::Touch* touch);
    bool known_touch(ax::Touch* touch);
    void released_touch(ax::Touch* touch);

  private:
    node& m_root;
    std::uint16_t m_pressed_ids;

    bim::axmol::ref_ptr<ax::EventListenerKeyboard> m_key_listener;
    std::vector<key_event> m_key_event_storage;

    bim::axmol::ref_ptr<ax::EventListenerTouchOneByOne> m_touch_listener;
  };
}
