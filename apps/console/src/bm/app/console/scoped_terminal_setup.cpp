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
#include <bm/app/console/scoped_terminal_setup.hpp>

#include <unistd.h>

bm::app::console::scoped_terminal_setup::scoped_terminal_setup(tcflag_t flags)
{
  tcgetattr(STDIN_FILENO, &m_original_terminal);
  m_custom_terminal = m_original_terminal;
  m_custom_terminal.c_lflag &= flags;
  tcsetattr(STDIN_FILENO, TCSANOW, &m_custom_terminal);
}

bm::app::console::scoped_terminal_setup::~scoped_terminal_setup()
{
  tcsetattr(STDIN_FILENO, TCSANOW, &m_original_terminal);
}
