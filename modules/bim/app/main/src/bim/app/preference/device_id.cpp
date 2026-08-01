// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/preference/device_id.hpp>

#include <iscool/preferences/local_preferences.hpp>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string
bim::app::device_id(const iscool::preferences::local_preferences& p)
{
  return p.get_value("device_id", std::string());
}

void bim::app::ensure_device_id_exists(
    iscool::preferences::local_preferences& p)
{
  if (device_id(p).empty())
    p.set_value("device_id",
                boost::uuids::to_string(boost::uuids::random_generator()()));
}
