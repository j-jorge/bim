// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/shop_service.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/analytics_service.hpp>
#include <bim/app/job/validate_purchase_job.hpp>
#include <bim/app/shop_implementation.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/signals/implement_signal.hpp>

IMPLEMENT_SIGNAL(bim::app::shop_service, purchase_error, m_purchase_error)

bim::app::shop_service::shop_service(analytics_service& analytics,
                                     const bim::business::request_headers& r,
                                     player_profile& p)
  : m_analytics(analytics)
  , m_shop(new shop_implementation())
  , m_validation_job(new validate_purchase_job(analytics, r, p))
{
  m_shop->connect_to_products_error(
      [this]()
        {
          products_error();
        });
  m_shop->connect_to_purchase_error(
      [this]()
        {
          purchase_error();
        });
  m_validation_job->connect_to_error(
      [this]()
        {
          m_purchase_error();
        });
  m_shop->connect_to_purchase_completed(
      [this](std::string_view sku, std::size_t quantity,
             std::string_view token)
        {
          validate_purchase(sku, quantity, token);
        });
}

bim::app::shop_service::~shop_service() = default;

iscool::signals::connection bim::app::shop_service::connect_to_products_ready(
    std::function<void(const std::unordered_map<std::string, std::string>&)> f)
    const
{
  return m_shop->connect_to_products_ready(std::move(f));
}

iscool::signals::connection bim::app::shop_service::connect_to_products_error(
    std::function<void()> f) const
{
  return m_shop->connect_to_products_error(std::move(f));
}

iscool::signals::connection
bim::app::shop_service::connect_to_purchase_completed(
    std::function<void(purchase_validation_status)> f) const
{
  return m_validation_job->connect_to_done(std::move(f));
}

void bim::app::shop_service::fetch_products(
    std::span<const std::string_view> skus)
{
  m_shop->fetch_products(skus);
}

void bim::app::shop_service::refresh_purchases()
{
  m_shop->refresh_purchases();
}

void bim::app::shop_service::purchase(std::string_view sku)
{
  m_analytics.event("purchase", { { "product", sku } });

  m_shop->purchase(sku);
}

void bim::app::shop_service::products_error() const
{
  bim::app::error(m_analytics, "products-detail");

  ic_log(iscool::log::nature::error(), "shop_service",
         "Could not fetch the products detail.");
}

void bim::app::shop_service::purchase_error() const
{
  ic_log(iscool::log::nature::error(), "shop_service",
         "Could not perform the purchase.");

  bim::app::error(m_analytics, "purchase");

  m_purchase_error();
}

void bim::app::shop_service::validate_purchase(std::string_view sku,
                                               std::size_t quantity,
                                               std::string_view token)
{
  m_analytics.event(
      "validate-purchase",
      { { "product", sku }, { "quantity", std::to_string(quantity) } });

  m_validation_job->start(std::string(token));
}
