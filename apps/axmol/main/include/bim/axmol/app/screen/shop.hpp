// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/observer/single_key_observer_handle.hpp>
#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace bim::axmol::widget
{
  class context;
}

namespace bim::app
{
  struct config;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace iscool::style
{
  class declaration;
}

namespace ax
{
  class Label;
}

namespace bim::axmol::app
{
  class analytics_service;
  class main_scene;
  class message_popup;
  class shop_service;
  class wallet;

  class shop
  {
    DECLARE_VOID_SIGNAL(back, m_back)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((analytics_service*)(analytics))                              //
            ((const bim::app::config*)(config))                            //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ),
        ic_context_no_properties);

  public:
    shop(const context& context, const iscool::style::declaration& style);
    ~shop();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& display_nodes() const;

    void attached();
    void displaying();
    void displayed();

  private:
    void dispatch_back();

    void fetch_products();
    void products_ready(
        const std::unordered_map<std::string, std::string>& products);
    void products_error();

    void start_purchase(std::size_t product_index);
    void purchase_completed(std::string_view product, std::size_t quantity,
                            std::string_view token);
    void purchase_error();

  private:
    static constexpr std::size_t max_product_count = 4;

    bim::axmol::input::single_key_observer_handle m_escape;
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 9);

    const std::unique_ptr<wallet> m_wallet;

    const std::unique_ptr<shop_service> m_shop;
    iscool::signals::scoped_connection m_products_connection;
    iscool::signals::scoped_connection m_products_error_connection;
    iscool::signals::scoped_connection m_purchase_connection;
    iscool::signals::scoped_connection m_purchase_error_connection;

    bim::axmol::widget::named_node_group m_all_nodes;

    const iscool::style::declaration& m_style_loading;
    const iscool::style::declaration& m_style_ready;

    ax::Label* m_amount_label[max_product_count];
    ax::Label* m_price_label[max_product_count];

    /// Index of the displayed products in the config.
    std::vector<std::uint8_t> m_index_in_products;

    std::unique_ptr<message_popup> m_message_popup;
  };
}
