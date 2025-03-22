// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/context.hpp>

#include <cstdint>

namespace bim::game
{
  class contest_result;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::axmol::app
{
  class player_progress_tracker
  {
    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties( //
            ((iscool::preferences::local_preferences*)(local_preferences))),
        ic_context_no_properties);

  public:
    explicit player_progress_tracker(const context& context);
    ~player_progress_tracker();

    void game_over_in_public_arena(const bim::game::contest_result& result,
                                   std::uint8_t local_player_index);
  };
}
