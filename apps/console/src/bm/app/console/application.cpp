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
#include <bm/app/console/application.hpp>

bm::app::console::application::application()
  : m_quit(false)
{}

void bm::app::console::application::quit()
{
  m_quit.store(true);
}

bool bm::app::console::application::should_quit() const
{
  return m_quit.load();
}

std::chrono::nanoseconds bm::app::console::application::update_interval() const
{
  // 60 updates per second.
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::duration<std::size_t, std::ratio<1, 60>>(1));
}
