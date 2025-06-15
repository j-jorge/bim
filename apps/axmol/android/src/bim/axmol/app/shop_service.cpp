// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/shop_service.hpp>

#include <iscool/jni/array_list.hpp>
#include <iscool/jni/cast_hash_map.hpp>
#include <iscool/jni/cast_jint.hpp>
#include <iscool/jni/cast_std_string.hpp>
#include <iscool/jni/get_static_method.hpp>
#include <iscool/jni/new_java_string.hpp>
#include <iscool/jni/static_method_void.hpp>
#include <iscool/jni/to_string.hpp>
#include <iscool/signals/implement_signal.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::shop_service, products_ready,
                 m_products_ready)
IMPLEMENT_SIGNAL(bim::axmol::app::shop_service, products_error,
                 m_products_error)
IMPLEMENT_SIGNAL(bim::axmol::app::shop_service, purchase_completed,
                 m_purchase_completed)
IMPLEMENT_SIGNAL(bim::axmol::app::shop_service, purchase_error,
                 m_purchase_error)

bim::axmol::app::shop_service::shop_service()
  : m_products_ready_from_java(
        std::function<void(
            const iscool::jni::hash_map<jstring, jstring>& products)>(
            [this](const iscool::jni::hash_map<jstring, jstring>& products)
            {
              dispatch_products_ready(products);
            }))
  , m_products_error_from_java(std::function<void()>(
        [this]()
        {
          m_products_error();
        }))
  , m_purchase_completed_from_java(
        std::function<void(const std::string&, int quantity,
                           const std::string&)>(
            [this](const std::string& product_id, int quantity,
                   const std::string& token)
            {
              m_purchase_completed(product_id, quantity, token);
            }))
  , m_purchase_error_from_java(std::function<void()>(
        [this]()
        {
          m_purchase_error();
        }))
{
  const iscool::jni::static_method<void> set_callbacks(
      iscool::jni::get_static_method<void>("bim/app/ShopService",
                                           "setCallbacks", "(JJJJ)V"));

  set_callbacks(m_products_ready_from_java.get_id(),
                m_products_error_from_java.get_id(),
                m_purchase_completed_from_java.get_id(),
                m_purchase_error_from_java.get_id());
}

bim::axmol::app::shop_service::~shop_service()
{
  const iscool::jni::static_method<void> set_callbacks(
      iscool::jni::get_static_method<void>("bim/app/ShopService",
                                           "setCallbacks", "(JJJJ)V"));
  set_callbacks(0, 0, 0, 0);
}

void bim::axmol::app::shop_service::fetch_products(
    std::span<std::string_view> ids)
{
  iscool::jni::array_list<jstring> product_ids;

  for (const std::string_view& s : ids)
    product_ids.add(iscool::jni::new_java_string(std::string(s)));

  const iscool::jni::static_method<void> fetch_products(
      iscool::jni::get_static_method<void>(
          "bim/app/ShopService", "fetchProducts", "(Ljava/util/List;)V"));

  fetch_products(product_ids);
}

void bim::axmol::app::shop_service::refresh_purchases()
{
  const iscool::jni::static_method<void> refresh_purchases(
      iscool::jni::get_static_method<void>("bim/app/ShopService",
                                           "refreshPurchases", "()V"));

  refresh_purchases();
}

void bim::axmol::app::shop_service::purchase(std::string_view id)
{
  const iscool::jni::static_method<void> purchase(
      iscool::jni::get_static_method<void>("bim/app/ShopService", "purchase",
                                           "(Ljava/lang/String;)V"));

  purchase(iscool::jni::new_java_string(std::string(id)));
}

void bim::axmol::app::shop_service::consume(std::string_view token)
{
  const iscool::jni::static_method<void> purchase(
      iscool::jni::get_static_method<void>(
          "bim/app/ShopService", "consumePurchase", "(Ljava/lang/String;)V"));

  purchase(iscool::jni::new_java_string(std::string(token)));
}

void bim::axmol::app::shop_service::dispatch_products_ready(
    const iscool::jni::hash_map<jstring, jstring>& products) const
{
  std::unordered_map<std::string, std::string> p;

  for (const iscool::jni::map_entry<jstring, jstring>& e :
       products.get_entry_set())
    p[iscool::jni::to_string(e.get_key())] =
        iscool::jni::to_string(e.get_value());

  m_products_ready(p);
}
