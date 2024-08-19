// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/message_type.hpp>

#include <iscool/net/message/raw_message.hpp>

namespace bim::net
{
  DECLARE_RAW_MESSAGE(authentication_ok, message_type::authentication_ok,
                      ((client_token)(request_token)) //
                      ((iscool::net::session_id)(session_id)));
}
