/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <termios.h>

namespace bim::app::console
{
  struct scoped_terminal_setup
  {
    explicit scoped_terminal_setup(tcflag_t flags);

    scoped_terminal_setup(const scoped_terminal_setup&) = delete;
    scoped_terminal_setup& operator=(const scoped_terminal_setup&) = delete;

    ~scoped_terminal_setup();

  private:
    termios m_original_terminal;
    termios m_custom_terminal;
  };
}
