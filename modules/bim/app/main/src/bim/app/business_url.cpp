// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/business_url.hpp>

#include <cstdlib>
#include <string>

const std::string bim::app::business_url = []()
  {
    const char* const e = std::getenv("BIM_BUSINESS_SERVER_URL");

    if (e)
      return e;

    return BIM_BUSINESS_SERVER_URL_STR;
  }();
