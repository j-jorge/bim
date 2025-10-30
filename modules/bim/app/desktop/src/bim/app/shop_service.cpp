// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/shop_service.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>

IMPLEMENT_SIGNAL(bim::app::shop_service, products_ready, m_products_ready)
IMPLEMENT_SIGNAL(bim::app::shop_service, products_error, m_products_error)
IMPLEMENT_SIGNAL(bim::app::shop_service, purchase_completed,
                 m_purchase_completed)
IMPLEMENT_SIGNAL(bim::app::shop_service, purchase_error, m_purchase_error)

bim::app::shop_service::shop_service()
  : m_fetch_ok(true)
  , m_refresh_ok(true)
  , m_purchase_ok(true)
{}

bim::app::shop_service::~shop_service() = default;

void bim::app::shop_service::fetch_products(std::span<std::string_view> ids)
{
  m_fetch_ok = !m_fetch_ok;

  if (m_fetch_ok)
    {
      m_product_ids = std::vector<std::string>(ids.begin(), ids.end());

      m_fetch_response = iscool::schedule::delayed_call(
          [this]() -> void
          {
            std::unordered_map<std::string, std::string> prices;
            int i = 0;
            for (const std::string& id : m_product_ids)
              {
                ++i;

                // Ignore the first item, just to test the UI in the case of
                // an incomplete list.
                if (i >= 2)
                  prices[id] = "$" + std::to_string(i * 10 - 1) + ".99";
              }

            m_products_ready(prices);
          },
          std::chrono::seconds(1));
    }
  else
    m_fetch_response = iscool::schedule::delayed_call(
        [this]() -> void
        {
          m_products_error();
        },
        std::chrono::seconds(1));
}

void bim::app::shop_service::refresh_purchases()
{
  m_refresh_ok = !m_refresh_ok;

  if (m_refresh_ok && !m_product_ids.empty())
    m_refresh_response = iscool::schedule::delayed_call(
        [this]() -> void
        {
          m_purchase_completed(m_product_ids[0], 12, "refresh-token");
        },
        std::chrono::seconds(1));
}

void bim::app::shop_service::purchase(std::string_view id)
{
  m_purchase_ok = !m_purchase_ok;

  if (m_purchase_ok)
    m_purchase_response = iscool::schedule::delayed_call(
        [this, id]() -> void
        {
          m_purchase_completed(id, 2, "purchase-token");
        },
        std::chrono::seconds(1));
  else
    m_purchase_response = iscool::schedule::delayed_call(
        [this]() -> void
        {
          m_purchase_error();
        },
        std::chrono::seconds(1));
}

void bim::app::shop_service::consume(std::string_view token)
{
  ic_log(iscool::log::nature::info(), "shop_service",
         "Consume purchase with token='{}'.", token);
}
