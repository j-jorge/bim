// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/observer/rich_text_glue.hpp>

#include <axmol/ui/UIRichText.h>

bim::axmol::input::rich_text_glue::rich_text_glue(ax::ui::RichText& node)
  : axmol_node_touch_observer(node)
  , m_node(node)
{
  m_node.setTouchEnabled(false);
}

void bim::axmol::input::rich_text_glue::do_pressed(touch_event& touch)
{
  if (should_ignore_touches())
    return;

  if (touch.is_available())
    {
      touch.consume();
      m_pressed.insert(touch.get()->getID());
      m_node.onTouchBegan(touch.get(), nullptr);
    }
}

void bim::axmol::input::rich_text_glue::do_moved(touch_event& touch)
{
  if (should_ignore_touches())
    return;

  if (!touch.is_available())
    return;

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

void bim::axmol::input::rich_text_glue::do_released(touch_event& touch)
{
  if (should_ignore_touches())
    return;

  if (!touch.is_available())
    return;

  if (m_pressed.erase(touch.get()->getID()))
    {
      touch.consume();
      m_node.onTouchEnded(touch.get(), nullptr);
    }
}

void bim::axmol::input::rich_text_glue::do_cancelled(touch_event& touch)
{
  if (should_ignore_touches())
    return;

  if (!touch.is_available())
    return;

  if (m_pressed.erase(touch.get()->getID()))
    {
      touch.consume();
      m_node.onTouchCancelled(touch.get(), nullptr);
    }
}

void bim::axmol::input::rich_text_glue::do_unplugged()
{
  m_pressed.clear();
}
