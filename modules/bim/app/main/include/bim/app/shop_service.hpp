// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/app/business/purchase_validation_status_fwd.hpp>

#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace bim::business
{
  class request_headers;
}

namespace bim::app
{
  class analytics_service;
  class player_profile;
  class shop_implementation;
  class validate_purchase_job;

  class shop_service
  {
    DECLARE_SIGNAL(void(), purchase_error, m_purchase_error)

  public:
    shop_service(analytics_service& analytics,
                 const bim::business::request_headers& r, player_profile& p);
    ~shop_service();

    iscool::signals::connection connect_to_products_ready(
        std::function<
            void(const std::unordered_map<std::string, std::string>&)>
            f) const;
    iscool::signals::connection
    connect_to_products_error(std::function<void()> f) const;

    iscool::signals::connection connect_to_purchase_completed(
        std::function<void(purchase_validation_status)> f) const;

    void fetch_products(std::span<const std::string_view> skus);
    void refresh_purchases();
    void purchase(std::string_view sku);

  private:
    void products_error() const;
    void purchase_error() const;

    void validate_purchase(std::string_view sku, std::size_t quantity,
                           std::string_view token);

  private:
    analytics_service& m_analytics;
    std::unique_ptr<shop_implementation> m_shop;
    std::unique_ptr<validate_purchase_job> m_validation_job;
  };
}
