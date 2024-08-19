// SPDX-License-Identifier: AGPL-3.0-only
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
