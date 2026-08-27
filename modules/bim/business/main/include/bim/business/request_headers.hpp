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
    request_headers();
    explicit request_headers(std::string_view authorization_token);

    void push_authorization(std::string_view authorization_token);

  public:
    std::vector<std::string> headers;
  };
}
