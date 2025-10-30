// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace bim::app
{
  /// Shop service that returns mockup responses.
  class shop_service
  {
    DECLARE_SIGNAL(void(const std::unordered_map<std::string, std::string>&),
                   products_ready, m_products_ready)
    DECLARE_VOID_SIGNAL(products_error, m_products_error)
    DECLARE_SIGNAL(void(std::string_view, std::size_t, std::string_view),
                   purchase_completed, m_purchase_completed)
    DECLARE_VOID_SIGNAL(purchase_error, m_purchase_error)

  public:
    shop_service();
    ~shop_service();

    void fetch_products(std::span<std::string_view> ids);
    void refresh_purchases();
    void purchase(std::string_view id);
    void consume(std::string_view token);

  private:
    /**
     * These flags are toggled on each request such that the response is
     * sometimes positive, sometimes negative.
     */
    bool m_fetch_ok;
    bool m_refresh_ok;
    bool m_purchase_ok;

    iscool::schedule::scoped_connection m_fetch_response;
    iscool::schedule::scoped_connection m_refresh_response;
    iscool::schedule::scoped_connection m_purchase_response;

    std::vector<std::string> m_product_ids;
  };
}
