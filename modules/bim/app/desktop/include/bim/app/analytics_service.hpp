// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <boost/container/small_vector.hpp>

#include <string_view>
#include <utility>

namespace bim::app
{
  class analytics_service
  {
  public:
    using property_list = boost::container::small_vector<
        std::pair<std::string_view, std::string_view>, 2>;

  public:
    analytics_service();
    ~analytics_service();

    void event(std::string_view name);
    void event(std::string_view name, const property_list& properties);

    void screen(std::string_view name);
    void screen(std::string_view name, const property_list& properties);
  };
}
