// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/frame_profiler.hpp>

#include <axmol/base/Director.h>
#include <axmol/base/EventListenerCustom.h>

#include <tracy/Tracy.hpp>

static const char* const g_tracy_tag_update = "update";
static const char* const g_tracy_tag_draw = "draw";

bim::axmol::app::frame_profiler::frame_profiler()
{
  ax::EventDispatcher& event_dispatcher =
      ax::Director::getInstance()->getEventDispatcher();

  m_event_listeners.push_back(
      event_dispatcher.addCustomEventListener(ax::Director::EVENT_AFTER_LOOP,
                                              [this](const ax::EventCustom*)
                                              {
                                                after_loop();
                                              }));

  m_event_listeners.push_back(event_dispatcher.addCustomEventListener(
      ax::Director::EVENT_BEFORE_UPDATE,
      [this](const ax::EventCustom*)
      {
        before_update();
      }));

  m_event_listeners.push_back(
      event_dispatcher.addCustomEventListener(ax::Director::EVENT_AFTER_UPDATE,
                                              [this](const ax::EventCustom*)
                                              {
                                                after_update();
                                              }));

  m_event_listeners.push_back(
      event_dispatcher.addCustomEventListener(ax::Director::EVENT_BEFORE_DRAW,
                                              [this](const ax::EventCustom*)
                                              {
                                                before_draw();
                                              }));

  m_event_listeners.push_back(
      event_dispatcher.addCustomEventListener(ax::Director::EVENT_AFTER_DRAW,
                                              [this](const ax::EventCustom*)
                                              {
                                                after_draw();
                                              }));
}

bim::axmol::app::frame_profiler::~frame_profiler()
{
  ax::EventDispatcher& event_dispatcher =
      ax::Director::getInstance()->getEventDispatcher();

  for (const bim::axmol::ref_ptr<ax::EventListenerCustom>& p :
       m_event_listeners)
    event_dispatcher.removeEventListener(p.get());
}

void bim::axmol::app::frame_profiler::after_loop()
{
  FrameMark;
}

void bim::axmol::app::frame_profiler::before_update()
{
  FrameMarkStart(g_tracy_tag_update);
}

void bim::axmol::app::frame_profiler::after_update()
{
  FrameMarkEnd(g_tracy_tag_update);
}

void bim::axmol::app::frame_profiler::before_draw()
{
  FrameMarkStart(g_tracy_tag_draw);
}

void bim::axmol::app::frame_profiler::after_draw()
{
  FrameMarkEnd(g_tracy_tag_draw);
}
