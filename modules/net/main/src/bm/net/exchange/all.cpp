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
#include <bm/net/exchange/exchange.hpp>

#include <bm/net/exchange/detail/exchange.impl.hpp>

#include <bm/net/message/authentication.hpp>
#include <bm/net/message/authentication_ko.hpp>
#include <bm/net/message/authentication_ok.hpp>

template class bm::net::exchange<bm::net::authentication,
                                 bm::net::authentication_ok,
                                 bm::net::authentication_ko>;
