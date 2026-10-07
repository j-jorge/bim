// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/user_id.hpp>

#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/connection.hpp>
#include <iscool/signals/signal.hpp>

#include <boost/unordered/unordered_map.hpp>
#include <boost/unordered/unordered_set.hpp>

#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace bim::business
{
  class request_headers;
}

namespace bim::app
{
  class analytics_service;
  class fetch_user_profiles_job;
  struct user_profile;

  class user_profile_cache
  {
  public:
    user_profile_cache(analytics_service& analytics,
                       const bim::business::request_headers& r);
    ~user_profile_cache();

    void invalidate(bim::net::user_id id);

    const user_profile* get(bim::net::user_id id) const;

    iscool::signals::connection
    fetch(bim::net::user_id id,
          std::function<void(const user_profile&)> ready);

  private:
    struct user_profile_hash
    {
      using is_transparent = std::true_type;

      std::size_t operator()(const user_profile& p) const;
      std::size_t operator()(bim::net::user_id id) const;
    };

    struct user_profile_equals
    {
      using is_transparent = std::true_type;

      bool operator()(const user_profile& lhs, const user_profile& rhs) const;
      bool operator()(bim::net::user_id lhs, const user_profile& rhs) const;
    };

    using profile_set = boost::unordered_set<user_profile, user_profile_hash,
                                             user_profile_equals>;

    using signal_map = boost::unordered_map<
        bim::net::user_id, iscool::signals::signal<void(const user_profile&)>>;

  private:
    void fetch_profiles();
    void insert_profiles(std::span<user_profile> profiles);

  private:
    profile_set m_profiles;

    signal_map m_signals;

    std::unique_ptr<fetch_user_profiles_job> m_job;

    std::vector<bim::net::user_id> m_to_be_fetched;
    iscool::schedule::scoped_connection m_job_start_connection;
  };
}
