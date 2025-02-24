// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/message_type.hpp>

#include <iscool/net/message/raw_message.hpp>

namespace bim::net
{
  DECLARE_EMPTY_RAW_MESSAGE(keep_alive, message_type::keep_alive);
}
