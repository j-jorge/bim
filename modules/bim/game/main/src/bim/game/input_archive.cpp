// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/input_archive.hpp>

bim::game::input_archive::input_archive(const char* bytes)
  : m_cursor(bytes)
{}
