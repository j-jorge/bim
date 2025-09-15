// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/observer/rich_text_glue.hpp>

#include <axmol/ui/UIRichText.h>

bim::axmol::input::rich_text_glue::rich_text_glue(ax::ui::RichText& node)
  : axmol_node_touch_observer(node)
  , m_node(node)
{
  m_node.setTouchEnabled(false);
}

void bim::axmol::input::rich_text_glue::do_pressed(
    const touch_event_view& touches)
{
  if (should_ignore_touches())
    return;

  for (touch_event& touch : touches)
    if (touch.is_available())
      {
        touch.consume();
        m_pressed.insert(touch.get()->getID());
        m_node.onTouchBegan(touch.get(), nullptr);
      }
}

void bim::axmol::input::rich_text_glue::do_moved(
    const touch_event_view& touches)
{
  if (should_ignore_touches())
    return;

  for (touch_event& touch : touches)
    {
      if (!touch.is_available())
        continue;

      touch.consume();

      const int id = touch.get()->getID();

      if (m_pressed.insert(id).second)
        {
          m_pressed.insert(id);
          m_node.onTouchBegan(touch.get(), nullptr);
        }
      else
        m_node.onTouchMoved(touch.get(), nullptr);
    }
}

void bim::axmol::input::rich_text_glue::do_released(
    const touch_event_view& touches)
{
  if (should_ignore_touches())
    return;

  for (touch_event& touch : touches)
    {
      if (!touch.is_available())
        continue;

      if (m_pressed.erase(touch.get()->getID()))
        {
          touch.consume();
          m_node.onTouchEnded(touch.get(), nullptr);
        }
    }
}

void bim::axmol::input::rich_text_glue::do_cancelled(
    const touch_event_view& touches)
{
  if (should_ignore_touches())
    return;

  for (touch_event& touch : touches)
    {
      if (!touch.is_available())
        continue;

      if (m_pressed.erase(touch.get()->getID()))
        {
          touch.consume();
          m_node.onTouchCancelled(touch.get(), nullptr);
        }
    }
}
