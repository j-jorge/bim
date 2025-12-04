// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/script_director.hpp>

#include <bim/axmol/app/application_event_listener.hpp>

#include <bim/axmol/find_child_by_path.hpp>

#include <iscool/json/from_file.hpp>
#include <iscool/json/is_member.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <axmol/base/Director.h>
#include <axmol/base/EventDispatcher.h>
#include <axmol/base/EventTouch.h>
#include <axmol/base/Touch.h>
#include <axmol/base/Utils.h>
#include <axmol/platform/Image.h>

#include <json/value.h>

#include <fmt/format.h>

static void dump_node(ax::Node& n, int indent)
{
  const std::string_view name = n.getName();

  printf("%*s- %*s\n", indent, "", (int)name.size(), name.data());

  for (ax::Node* c : n.getChildren())
    dump_node(*c, indent + 2);
}

IMPLEMENT_SIGNAL(bim::axmol::app::script_director, done, m_done);

enum class bim::axmol::app::script_director::step_kind : std::uint8_t
{
  wait,
  click,
  capture
};

struct bim::axmol::app::script_director::step
{
  step_kind kind;
  std::uint8_t index;
};

struct bim::axmol::app::script_director::click_target
{
  std::string node_path;
  std::string event;
};

bim::axmol::app::script_director::script_director(
    const application_event_listener& events, const std::string& script_file,
    bool number_screenshots, std::chrono::seconds timeout)
  : m_next_step(0)
  , m_timeout(timeout)
  , m_event_connection(events.connect_to_event(
        [this](std::string_view name)
        {
          check_event(name);
        }))
{
  const auto fail = [this]()
  {
    m_tick_connection = iscool::schedule::delayed_call(
        [this]()
        {
          m_done(result::fail);
        });
  };

  const Json::Value script = iscool::json::from_file(script_file);

  if (!script.isObject())
    {
      ic_log(iscool::log::nature::error(), "script_director",
             "{}: not a Json object.", script_file);
      fail();
      return;
    }

  const Json::Value& actions = script["actions"];

  if (!actions.isArray())
    {
      ic_log(iscool::log::nature::error(), "script_director",
             "{}: member array 'actions' is missing.", script_file);
      fail();
      return;
    }

  const Json::ArrayIndex n = actions.size();
  m_steps.reserve(n);

  const auto action_property_to_string = [&](std::string& result,
                                             const Json::Value& action,
                                             const std::string& key, int i)
  {
    const Json::Value& value = action[key];

    if (!value.isString())
      {
        ic_log(iscool::log::nature::error(), "script_director",
               "{}: action {}'s '{}' is not a string.", script_file, key, i);
        return false;
      }

    result = value.asString();
    return true;
  };

  std::uint8_t next_screenshot_number = 0;

  for (Json::ArrayIndex i = 0; i != n; ++i)
    {
      const Json::Value& action = actions[i];

      if (!action.isObject())
        {
          ic_log(iscool::log::nature::error(), "script_director",
                 "{}: action {} is not a Json object.", script_file, i);
          continue;
        }

      std::string kind_string;

      if (!action_property_to_string(kind_string, action, "kind", i))
        continue;

      if (kind_string == "capture")
        {
          std::string file_name;

          if (!action_property_to_string(file_name, action, "file", i))
            continue;

          m_steps.emplace_back(step_kind::capture, m_capture_steps.size());

          if (number_screenshots)
            {
              m_capture_steps.emplace_back(
                  fmt::format("{:03}-{}", next_screenshot_number, file_name));
              ++next_screenshot_number;
            }
          else
            m_capture_steps.emplace_back(std::move(file_name));
        }
      else if (kind_string == "click")
        {
          std::string path;

          if (!action_property_to_string(path, action, "node", i))
            continue;

          std::string event;

          if (iscool::json::is_member("event", action)
              && !action_property_to_string(event, action, "event", i))
            continue;

          m_steps.emplace_back(step_kind::click, m_click_steps.size());
          m_click_steps.emplace_back(std::move(path), std::move(event));
        }
      else if (kind_string == "wait")
        {
          std::string event;

          if (!action_property_to_string(event, action, "event", i))
            continue;

          m_steps.emplace_back(step_kind::wait, m_wait_steps.size());
          m_wait_steps.emplace_back(std::move(event));
        }
      else
        ic_log(iscool::log::nature::error(), "script_director",
               "{}: unknown kind {} for action {}.", script_file, kind_string,
               i);
    }

  schedule_tick();
}

bim::axmol::app::script_director::~script_director() = default;

void bim::axmol::app::script_director::schedule_tick()
{
  // Sets a delay to ensure that it won't run in the same game tick.
  m_tick_connection = iscool::schedule::delayed_call(
      [this]()
      {
        tick();
      },
      std::chrono::seconds(0));
}

void bim::axmol::app::script_director::tick()
{
  const std::size_t step_count = m_steps.size();
  assert(m_next_step <= step_count);

  if (m_next_step == step_count)
    {
      m_done(result::ok);
      return;
    }

  const std::size_t s = m_next_step;
  ++m_next_step;

  const std::size_t i = m_steps[s].index;

  switch (m_steps[s].kind)
    {
    case step_kind::capture:
      ic_log(iscool::log::nature::info(), "script_director", "Capture '{}'.",
             m_capture_steps[i]);

      capture(m_capture_steps[i]);
      // The capture is scheduled for the end of the frame. We don't run
      // anything else yet to avoid modifying the state before the capture.
      schedule_tick();
      break;
    case step_kind::click:
      ic_log(iscool::log::nature::info(), "script_director",
             "Click '{}', wait '{}'.", m_click_steps[i].node_path,
             m_pending_event);

      click(m_click_steps[i].node_path);

      m_pending_event = m_click_steps[i].event;

      if (m_pending_event.empty())
        schedule_tick();
      else
        schedule_timeout();
      break;
    case step_kind::wait:
      ic_log(iscool::log::nature::info(), "script_director", "Wait '{}'.",
             m_wait_steps[i]);

      m_pending_event = m_wait_steps[i];
      schedule_timeout();
      break;
    }
}

void bim::axmol::app::script_director::capture(
    const std::string& file_name) const
{
  ax::utils::captureScreen(
      [=](ax::RefPtr<ax::Image> image) -> void
      {
        image->saveToFile(file_name.c_str());
      });
}

void bim::axmol::app::script_director::click(
    const std::string& node_path) const
{
  ax::Director& director = *ax::Director::getInstance();

  ax::Node* const n =
      bim::axmol::find_child_by_path(*director.getRunningScene(), node_path);

  if (n == nullptr)
    {
      dump_node(*director.getRunningScene(), 0);
      ic_log(iscool::log::nature::error(), "script_director",
             "Cannot find node '{}'.", node_path);
      m_done(result::fail);

      return;
    }

  const ax::Vec2 p =
      director.convertToUI(n->convertToWorldSpace(n->getContentSize() / 2));

  ic_log(iscool::log::nature::info(), "script_director",
         "Click '{}', at ({}, {}).", node_path, p.x, p.y);

  ax::Touch touch;
  touch.setTouchInfo(0, p.x, p.y);

  press(touch);

  // We need to set it twice, otherwise the release event is ignored.
  touch.setTouchInfo(0, p.x, p.y);
  release(touch);
}

void bim::axmol::app::script_director::press(ax::Touch& touch) const
{
  std::vector<ax::Touch*> touches;
  touches.emplace_back(&touch);

  ax::EventTouch event;
  event.setEventCode(ax::EventTouch::EventCode::BEGAN);
  event.setTouches(std::move(touches));

  ax::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
}

void bim::axmol::app::script_director::release(ax::Touch& touch) const
{
  std::vector<ax::Touch*> touches;
  touches.emplace_back(&touch);

  ax::EventTouch event;
  event.setEventCode(ax::EventTouch::EventCode::ENDED);
  event.setTouches(std::move(touches));

  ax::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
}

void bim::axmol::app::script_director::check_event(std::string_view name)
{
  if (name == m_pending_event)
    {
      m_pending_event = {};
      m_timeout_connection.disconnect();
      schedule_tick();
    }
}

void bim::axmol::app::script_director::schedule_timeout()
{
  m_timeout_connection = iscool::schedule::delayed_call(
      [this]()
      {
        timeout();
      },
      m_timeout);
}

void bim::axmol::app::script_director::timeout()
{
  ic_log(iscool::log::nature::error(), "script_director",
         "Timeout while waiting for event '{}'.", m_pending_event);

  m_done(result::fail);
}
