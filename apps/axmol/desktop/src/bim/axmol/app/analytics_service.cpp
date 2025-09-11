// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/analytics_service.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>

bim::axmol::app::analytics_service::analytics_service() = default;
bim::axmol::app::analytics_service::~analytics_service() = default;

void bim::axmol::app::analytics_service::event(std::string_view name)
{
  ic_log(iscool::log::nature::info(), "analytics_service", "Event '{}'.",
         name);
}

void bim::axmol::app::analytics_service::event(std::string_view name,
                                               const property_list& properties)
{
  std::string p;

  for (const std::pair<std::string_view, std::string_view>& e : properties)
    {
      p += ' ';
      p += e.first;
      p += "='";
      p += e.second;
      p += '\'';
    }

  ic_log(iscool::log::nature::info(), "analytics_service", "Event '{}',{}.",
         name, p);
}

void bim::axmol::app::analytics_service::screen(std::string_view name)
{
  ic_log(iscool::log::nature::info(), "analytics_service", "Screen '{}'.",
         name);
}

void bim::axmol::app::analytics_service::screen(
    std::string_view name, const property_list& properties)
{
  std::string p;

  for (const std::pair<std::string_view, std::string_view>& e : properties)
    {
      p += ' ';
      p += e.first;
      p += "='";
      p += e.second;
      p += '\'';
    }

  ic_log(iscool::log::nature::info(), "analytics_service", "Screen '{}',{}.",
         name, p);
}
