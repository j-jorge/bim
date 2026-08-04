// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace bim::business
{
  /**
   * HTTP headers common to all business requests: authorization tokend,+ JSON
   * in, JSON out.
   */
  class request_headers
  {
  public:
    explicit request_headers(std::string_view authorization_token);

    std::vector<std::string> headers;
  };
}
