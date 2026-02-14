// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/strings/unordered_string_map.hpp>

#include <string>
#include <string_view>

namespace Json
{
  class Value;
}

namespace bim::axmol::widget
{
  struct animation;

  class animation_cache
  {
  public:
    animation_cache();
    ~animation_cache();

    void load(const std::string_view& json_path);
    void load(const Json::Value& config);

    const animation& get(const std::string_view& name) const;

  private:
    using animation_map = iscool::strings::unordered_string_map<animation>;

  private:
    animation_map m_animations;
  };
}
