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

#include <bim/net/message/authentication_error_code.hpp>
#include <bim/net/message/client_token.hpp>
#include <bim/net/message/message_type.hpp>

#include <iscool/net/message/raw_message.hpp>

namespace bim::net
{
  DECLARE_RAW_MESSAGE(authentication_ko, message_type::authentication_ko,
                      ((client_token)(request_token)) //
                      ((authentication_error_code)(error_code)));
}
