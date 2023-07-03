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

#include <bim/axmol/files/bridge.hpp>
#include <bim/axmol/schedule/bridge.hpp>

namespace bim::axmol::app
{
  class bridge
  {
  public:
    bridge();
    bridge(const bridge&) = delete;
    bridge(bridge&&) = delete;

    bridge& operator=(const bridge&) = delete;
    bridge& operator=(bridge&&) = delete;

  private:
    bim::axmol::files::bridge m_files;
    bim::axmol::schedule::bridge m_schedule;
  };
}
