// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/backend_event.hpp>
#include <bim/axmol/input/backend_event.impl.hpp>
#include <bim/axmol/input/backend_event_view.hpp>
#include <bim/axmol/input/backend_event_view.impl.hpp>

#include <gtest/gtest.h>

namespace
{
  using backend_event = bim::axmol::input::backend_event<char>;
  using backend_event_view =
      bim::axmol::input::backend_event_view<backend_event>;
}

TEST(bim_axmol_input_backend_event_view_test, consume_all)
{
  backend_event events[] = { backend_event('a'), backend_event('b'),
                             backend_event('c') };

  backend_event_view view(events);
  EXPECT_FALSE(view.is_fully_consumed());

  (view.begin() + 1)->consume();

  EXPECT_FALSE(view.is_fully_consumed());

  view.consume_all();

  for (const backend_event& event : view)
    EXPECT_FALSE(event.is_available());

  EXPECT_TRUE(view.is_fully_consumed());
}

TEST(bim_axmol_input_backend_event_view_test, consume_one_by_one)
{
  backend_event events[] = { backend_event('a'), backend_event('b'),
                             backend_event('c') };

  backend_event_view view(events);

  EXPECT_FALSE(view.is_fully_consumed());
  events[0].consume();

  EXPECT_FALSE(view.is_fully_consumed());
  events[1].consume();

  EXPECT_FALSE(view.is_fully_consumed());
  events[2].consume();

  EXPECT_TRUE(view.is_fully_consumed());
}

TEST(bim_axmol_input_backend_event_view_test, size)
{
  EXPECT_TRUE(backend_event_view({}).empty());

  backend_event events[] = { backend_event('a'), backend_event('b'),
                             backend_event('c') };

  backend_event_view view(events);

  EXPECT_EQ(std::size_t(3), view.size());
  EXPECT_FALSE(view.empty());
}
