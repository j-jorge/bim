// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/net/message/message.hpp>

#include <exception>
#include <optional>

namespace bim::net
{
  template <typename M>
  std::optional<M> try_deserialize_message(const iscool::net::message& message)
  {
    try
      {
        return M(message.get_content());
      }
    catch (const std::exception& e)
      {
        return std::nullopt;
      }
  }
}
