// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/device_id.hpp>

#include <iscool/files/full_path_exists.hpp>
#include <iscool/files/get_writable_path.hpp>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <fstream>

std::string bim::app::device_id()
{
  const std::string path =
      iscool::files::get_writable_path() + "/device-id.txt";

  if (iscool::files::full_path_exists(path))
    {
      std::ifstream f(path);
      boost::uuids::uuid uuid;

      if ((f >> uuid).ignore('\n').eof())
        return boost::uuids::to_string(uuid);
    }

  std::string result =
      boost::uuids::to_string(boost::uuids::random_generator()());

  std::ofstream f(path, std::ios_base::trunc | std::ios_base::out);
  f << result;

  return result;
}
