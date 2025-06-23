// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/signals/scoped_connection.hpp>

#include <boost/unordered/unordered_flat_map.hpp>

#include <string>
#include <string_view>

struct MMDB_s;

namespace bim::server
{
  struct config;

  class geolocation_service
  {
  public:
    struct address_info
    {
      std::uint64_t id;
      std::string_view country_code;
      std::string_view country;
    };

  public:
    explicit geolocation_service(const config& config);
    ~geolocation_service();

    address_info lookup(const std::string& ip);

  private:
    struct internal_info;

    using ip_to_id_map = boost::unordered_flat_map<std::string, std::uint64_t>;
    using id_to_info_map =
        boost::unordered_flat_map<std::uint64_t, internal_info>;
    using release_date_map =
        boost::unordered_flat_map<std::uint64_t, std::chrono::seconds>;

  private:
    address_info insert_new_ip(const std::string& ip);
    void fill_info(internal_info& info);
    void fill_unknown(internal_info& info) const;

    void schedule_clean_up();
    void clean_up();

    void schedule_update();
    void update_mmdb();
    void close_mmdb();

  private:
    ip_to_id_map m_ip_to_id;
    id_to_info_map m_address_info;

    std::unique_ptr<MMDB_s> m_mmdb;
    std::uint64_t m_next_id;

    std::uint64_t m_database_version;

    release_date_map m_release_date;
    iscool::signals::scoped_connection m_clean_up_connection;
    iscool::signals::scoped_connection m_update_connection;

    const std::chrono::seconds m_clean_up_interval;
    const std::chrono::minutes m_update_interval;
    const std::string m_database_path;
  };
}
