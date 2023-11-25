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
  class EventListenerTouchAllAtOnce;
  class Scene;
}

namespace bim::axmol::input
{
  class node;

  class flow : public ax::Ref
  {
  public:
    flow(ax::Scene& scene, node& root);
    ~flow();

    flow(const flow&) = delete;
    flow& operator=(const flow&) = delete;

  private:
    void key_pressed(ax::EventKeyboard::KeyCode key, ax::Event* event);
    void key_released(ax::EventKeyboard::KeyCode key, ax::Event* event);

    void touches_began(const std::vector<ax::Touch*>& touches,
                       ax::Event* event);

    void touches_moved(const std::vector<ax::Touch*>& touches,
                       ax::Event* event);

    void touches_ended(const std::vector<ax::Touch*>& touches,
                       ax::Event* event);

    void touches_cancelled(const std::vector<ax::Touch*>& touches,
                           ax::Event* event);

    void new_pressed_touches(const std::vector<ax::Touch*>& touches);

    void known_moving_touches(const std::vector<ax::Touch*>& touches);

    void known_released_touches(const std::vector<ax::Touch*>& touches);

  private:
    node& m_root;
    // TODO: not needed, mono touch is enough.
    std::unordered_set<int> m_pressed_ids;

    bim::axmol::ref_ptr<ax::EventListenerKeyboard> m_key_listener;
    std::vector<key_event> m_key_event_storage;

    bim::axmol::ref_ptr<ax::EventListenerTouchAllAtOnce> m_touch_listener;
    std::vector<touch_event> m_touch_event_storage;
  };
}
