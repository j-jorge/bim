// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen/shop.hpp>

#include <bim/axmol/app/analytics_service.hpp>
#include <bim/axmol/app/config.hpp>
#include <bim/axmol/app/part/wallet.hpp>
#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/preference/wallet.hpp>
#include <bim/axmol/app/shop_service.hpp>

#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/merge_named_node_groups.hpp>
#include <bim/axmol/widget/named_node_group.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/observer/single_key_observer.hpp>

#include <bim/axmol/find_child_by_path.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/i18n/numeric.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/log/nature/warning.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Label.h>

#define x_widget_scope bim::axmol::app::shop::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, back_button)                           \
      x_widget(bim::axmol::widget::button, coins_1_button)                    \
          x_widget(bim::axmol::widget::button, coins_2_button)                \
              x_widget(bim::axmol::widget::button, coins_3_button)            \
                  x_widget(bim::axmol::widget::button, coins_4_button)        \
                      x_widget(ax::Label, coins_1_label)                      \
                          x_widget(ax::Label, coins_2_label)                  \
                              x_widget(ax::Label, coins_3_label)              \
                                  x_widget(ax::Label, coins_4_label)

#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::shop, back, m_back);

bim::axmol::app::shop::shop(const context& context,
                            const iscool::style::declaration& style)
  : m_context(context)
  , m_escape(ax::EventKeyboard::KeyCode::KEY_BACK)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_wallet(new wallet(context, *style.get_declaration("wallet")))
  , m_shop(new shop_service())
  , m_style_loading(*style.get_declaration("display.loading"))
  , m_style_ready(*style.get_declaration("display.ready"))
  , m_amount_label{ m_controls->coins_1_label, m_controls->coins_2_label,
                    m_controls->coins_3_label, m_controls->coins_4_label }
  , m_message_popup(
        new message_popup(context, *style.get_declaration("message-popup")))
{
  const std::string& label_path =
      *style.get_string("coins-button-price-label-path");

  m_price_label[0] = dynamic_cast<ax::Label*>(
      find_child_by_path(*m_controls->coins_1_button, label_path));
  m_price_label[1] = dynamic_cast<ax::Label*>(
      find_child_by_path(*m_controls->coins_2_button, label_path));
  m_price_label[2] = dynamic_cast<ax::Label*>(
      find_child_by_path(*m_controls->coins_3_button, label_path));
  m_price_label[3] = dynamic_cast<ax::Label*>(
      find_child_by_path(*m_controls->coins_4_button, label_path));

  m_all_nodes = m_controls->all_nodes;
  bim::axmol::widget::merge_named_node_groups(m_all_nodes,
                                              m_wallet->display_nodes());

  m_inputs.push_back(m_escape);
  m_escape->connect_to_released(
      [this]()
      {
        dispatch_back();
      });

  m_inputs.push_back(m_controls->back_button->input_node());
  m_controls->back_button->connect_to_clicked(
      [this]()
      {
        dispatch_back();
      });

  m_inputs.push_back(m_controls->coins_1_button->input_node());
  m_controls->coins_1_button->connect_to_clicked(
      [this]()
      {
        start_purchase(0);
      });

  m_inputs.push_back(m_controls->coins_2_button->input_node());
  m_controls->coins_2_button->connect_to_clicked(
      [this]()
      {
        start_purchase(1);
      });

  m_inputs.push_back(m_controls->coins_3_button->input_node());
  m_controls->coins_3_button->connect_to_clicked(
      [this]()
      {
        start_purchase(2);
      });

  m_inputs.push_back(m_controls->coins_4_button->input_node());
  m_controls->coins_4_button->connect_to_clicked(
      [this]()
      {
        start_purchase(3);
      });

  m_products_connection = m_shop->connect_to_products_ready(
      [this](const std::unordered_map<std::string, std::string>& products)
      {
        products_ready(products);
      });
  m_products_error_connection = m_shop->connect_to_products_error(
      [this]()
      {
        products_error();
      });
  m_purchase_connection = m_shop->connect_to_purchase_completed(
      [this](std::string_view product, std::size_t quantity,
             std::string_view token)
      {
        purchase_completed(product, quantity, token);
      });
  m_purchase_error_connection = m_shop->connect_to_purchase_error(
      [this]()
      {
        purchase_error();
      });

  fetch_products();
}

bim::axmol::app::shop::~shop() = default;

bim::axmol::input::node_reference bim::axmol::app::shop::input_node() const
{
  return m_inputs.root();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::shop::display_nodes() const
{
  return m_all_nodes;
}

void bim::axmol::app::shop::attached()
{
  m_wallet->attached();
}

void bim::axmol::app::shop::displaying()
{
  m_wallet->enter();

  if (m_index_in_products.empty())
    fetch_products();
}

void bim::axmol::app::shop::displayed()
{
  m_shop->refresh_purchases();
}

void bim::axmol::app::shop::dispatch_back()
{
  m_back();
}

void bim::axmol::app::shop::fetch_products()
{
  bim::axmol::widget::apply_display(m_context.get_widget_context().style_cache,
                                    m_controls->all_nodes, m_style_loading);

  const config& config = *m_context.get_config();
  std::vector<std::string_view> product_ids;
  product_ids.reserve(config.shop_products.size());

  for (const std::string& product_id : config.shop_products)
    product_ids.push_back(product_id);

  m_shop->fetch_products(product_ids);
}

void bim::axmol::app::shop::products_ready(
    const std::unordered_map<std::string, std::string>& products)
{
  m_index_in_products.clear();

  const config& config = *m_context.get_config();

  for (std::size_t i = 0, n = config.shop_products.size(); i != n; ++i)
    {
      const auto it = products.find(config.shop_products[i]);

      if (it == products.end())
        continue;

      const std::size_t j = m_index_in_products.size();
      m_index_in_products.push_back(i);

      m_price_label[j]->setString(it->second);
      m_amount_label[j]->setString(
          iscool::i18n::numeric::to_string(config.shop_product_coins[i]));

      if (m_index_in_products.size() == max_product_count)
        break;
    }

  bim::axmol::widget::apply_display(m_context.get_widget_context().style_cache,
                                    m_controls->all_nodes, m_style_ready);
}

void bim::axmol::app::shop::products_error()
{
  m_context.get_analytics()->event("error",
                                   { { "cause", "products-detail" } });

  ic_log(iscool::log::nature::error(), "shop",
         "Could not fetch the products detail.");
}

void bim::axmol::app::shop::start_purchase(std::size_t product_index)
{
  if (product_index >= m_index_in_products.size())
    return;

  const std::string& product_id =
      m_context.get_config()
          ->shop_products[m_index_in_products[product_index]];

  m_context.get_analytics()->event("purchase", { { "product", product_id } });

  m_shop->purchase(product_id);
}

void bim::axmol::app::shop::purchase_completed(std::string_view product,
                                               std::size_t quantity,
                                               std::string_view token)
{
  const config& config = *m_context.get_config();

  for (std::size_t i : m_index_in_products)
    if (config.shop_products[i] == product)
      {
        m_context.get_analytics()->event(
            "purchase-completed",
            { { "product", product },
              { "quantity", std::to_string(quantity) } });

        add_coins(*m_context.get_local_preferences(),
                  quantity * config.shop_product_coins[i]);
        m_wallet->animate_cash_flow();
        m_shop->consume(token);
        return;
      }

  m_context.get_analytics()->event(
      "purchase-completed-error",
      { { "product", product }, { "quantity", std::to_string(quantity) } });

  ic_log(iscool::log::nature::warning(), "shop",
         "Purchase completed on unknown product '{}'.", product);
}

void bim::axmol::app::shop::purchase_error()
{
  m_context.get_analytics()->event("error", { { "cause", "purchase" } });

  ic_log(iscool::log::nature::error(), "shop",
         "Could not perform the purchase.");

  m_message_popup->show(
      ic_gettext("An error occurred. The purchase could not be completed."));
}
