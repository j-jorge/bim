// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/message_type.hpp>
#include <bim/net/message/version.hpp>

#include <iscool/net/message/raw_message.hpp>

namespace bim::net
{
  DECLARE_RAW_MESSAGE(authentication, message_type::authentication,
                      ((version)(protocol_version)) //
                      ((client_token)(request_token)));
}
