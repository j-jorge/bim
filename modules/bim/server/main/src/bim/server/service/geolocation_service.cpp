// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/geolocation_service.hpp>

#include <bim/server/config.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/log/nature/warning.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/time/now.hpp>

#include <maxminddb.h>

struct bim::server::geolocation_service::internal_info
{
  std::uint64_t database_version;
  std::string country_code;
  std::string country;
  std::string ip;
};

bim::server::geolocation_service::geolocation_service(const config& config)
  : m_next_id(0)
  , m_database_version(0)
  , m_clean_up_interval(config.geolocation_clean_up_interval)
  , m_update_interval(config.geolocation_update_interval)
  , m_database_path(config.geolocation_database_path)
{
  if (config.enable_geolocation)
    {
      m_mmdb.reset(new MMDB_s{});
      schedule_clean_up();
      update_mmdb();
    }
}

bim::server::geolocation_service::~geolocation_service()
{
  if (m_mmdb)
    close_mmdb();
}

bim::server::geolocation_service::address_info
bim::server::geolocation_service::lookup(const std::string& ip)
{
  const ip_to_id_map::iterator it_id = m_ip_to_id.find(ip);

  if (it_id == m_ip_to_id.end())
    return insert_new_ip(ip);

  const id_to_info_map::iterator it_info = m_address_info.find(it_id->second);
  const std::chrono::seconds now = iscool::time::now<std::chrono::seconds>();

  assert(it_info != m_address_info.end());

  internal_info* info = &it_info->second;

  assert(info->ip == ip);

  if (info->database_version == m_database_version)
    {
      m_release_date[it_id->second] = now + m_clean_up_interval;
      return { it_id->second, info->country_code, info->country };
    }

  // The database has changed since the last time we got this IP, we'll do
  // a refresh.
  internal_info internal;
  fill_info(internal);

  if ((internal.country_code != info->country_code)
      || (internal.country != info->country))
    {
      // The country has changed, we'll need a new id.
      m_address_info.erase(it_info);

      const std::uint64_t id = m_next_id;
      ++m_next_id;

      m_release_date[id] = now + m_clean_up_interval;
      it_id->second = id;

      info = &m_address_info[id];
      info->country = std::move(internal.country);
      info->country_code = std::move(internal.country_code);
      info->ip = ip;
    }

  return { it_id->second, info->country_code, info->country };
}

bim::server::geolocation_service::address_info
bim::server::geolocation_service::insert_new_ip(const std::string& ip)
{
  const std::uint64_t id = m_next_id;
  ++m_next_id;

  m_release_date[id] =
      iscool::time::now<std::chrono::seconds>() + m_clean_up_interval;

  m_ip_to_id[ip] = id;

  internal_info& info = m_address_info[id];
  info.ip = ip;
  fill_info(info);

  return { id, info.country_code, info.country };
}

void bim::server::geolocation_service::fill_info(internal_info& info)
{
  info.database_version = m_database_version;

  if (!m_mmdb)
    {
      fill_unknown(info);
      return;
    }

  int get_address_info_error;
  int mmdb_error;

  MMDB_lookup_result_s r = MMDB_lookup_string(
      m_mmdb.get(), info.ip.c_str(), &get_address_info_error, &mmdb_error);

  if (get_address_info_error != 0)
    {
      ic_log(iscool::log::nature::error(), "geolocation_service",
             "getaddrinfo() error {}: {}.", get_address_info_error,
             gai_strerror(get_address_info_error));
      fill_unknown(info);
      return;
    }

  if (mmdb_error != MMDB_SUCCESS)
    {
      ic_log(iscool::log::nature::error(), "geolocation_service",
             "mmdb error {}: {}.", mmdb_error, MMDB_strerror(mmdb_error));
      fill_unknown(info);
      return;
    }

  if (!r.found_entry)
    {
      ic_log(iscool::log::nature::warning(), "geolocation_service",
             "Could not find an entry for the given IP.");
      fill_unknown(info);
      return;
    }

  MMDB_entry_data_s country;
  MMDB_get_value(&r.entry, &country, "country", "names", "en", nullptr);

  if (!country.has_data)
    {
      ic_log(iscool::log::nature::warning(), "geolocation_service",
             "Country is not set.");
      fill_unknown(info);
      return;
    }

  if (country.type != MMDB_DATA_TYPE_UTF8_STRING)
    {
      ic_log(iscool::log::nature::warning(), "geolocation_service",
             "Unexpected data type {} for country.", country.type);
      fill_unknown(info);
      return;
    }

  MMDB_entry_data_s country_code;
  MMDB_get_value(&r.entry, &country_code, "country", "iso_code", nullptr);

  if (!country_code.has_data)
    {
      ic_log(iscool::log::nature::warning(), "geolocation_service",
             "Country code is not set.");
      fill_unknown(info);
      return;
    }

  if (country_code.type != MMDB_DATA_TYPE_UTF8_STRING)
    {
      ic_log(iscool::log::nature::warning(), "geolocation_service",
             "Unexpected data type {} for country code.", country.type);
      fill_unknown(info);
      return;
    }

  info.country.clear();
  info.country.insert(info.country.end(), country.utf8_string,
                      country.utf8_string + country.data_size);

  info.country_code.clear();
  info.country_code.insert(info.country_code.end(), country_code.utf8_string,
                           country_code.utf8_string + country_code.data_size);
}

void bim::server::geolocation_service::fill_unknown(internal_info& info) const
{
  info.country_code = "UNK";
  info.country = "Unknown";
}

void bim::server::geolocation_service::schedule_clean_up()
{
  m_clean_up_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        clean_up();
      },
      m_clean_up_interval);
}

void bim::server::geolocation_service::clean_up()
{
  ic_log(iscool::log::nature::info(), "geolocation_service",
         "Removing old IPs.");

  const std::chrono::seconds now = iscool::time::now<std::chrono::seconds>();

  for (release_date_map::iterator it = m_release_date.begin();
       it != m_release_date.end();)
    if (it->second <= now)
      {
        const id_to_info_map::iterator it_info =
            m_address_info.find(it->first);
        assert(it_info != m_address_info.end());
        m_ip_to_id.erase(it_info->second.ip);
        m_address_info.erase(it_info);

        it = m_release_date.erase(it);
      }
    else
      ++it;

  assert(m_ip_to_id.size() == m_address_info.size());
  assert(m_ip_to_id.size() == m_release_date.size());

  schedule_clean_up();
}

void bim::server::geolocation_service::schedule_update()
{
  m_update_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        update_mmdb();
      },
      m_update_interval);
}

void bim::server::geolocation_service::update_mmdb()
{
  ic_log(iscool::log::nature::info(), "geolocation_service",
         "Updating GeoIP database '{}'.", m_database_path);
  schedule_update();

  MMDB_s mmdb;

  const int status = MMDB_open(m_database_path.c_str(), MMDB_MODE_MMAP, &mmdb);

  if (status != MMDB_SUCCESS)
    {
      ic_log(iscool::log::nature::error(), "geolocation_service",
             "Failed to open database '{}': {}.", m_database_path,
             MMDB_strerror(status));
      return;
    }

  if (m_database_version != 0)
    MMDB_close(m_mmdb.get());

  *m_mmdb = mmdb;
  ++m_database_version;
}

void bim::server::geolocation_service::close_mmdb()
{
  MMDB_close(m_mmdb.get());
}
