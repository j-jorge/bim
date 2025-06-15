package bim.app;

import android.app.Activity;
import bim.app.IShop;
import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.PendingPurchasesParams;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.ProductDetailsResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryPurchasesParams;
import iscool.jni.NativeCall;
import iscool.log.Log;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

class PlayServicesShop implements IShop
{
  private final Activity m_activity;
  private final BillingClient m_client;

  private final BillingClientStateListener m_client_state_listener =
      new BillingClientStateListener() {
        @Override
        public void onBillingSetupFinished(BillingResult result)
        {}

        @Override
        public void onBillingServiceDisconnected()
        {
          connectClient();
        }
      };

  private final ProductDetailsResponseListener m_product_detail_listener =
      new ProductDetailsResponseListener() {
        public void onProductDetailsResponse(BillingResult result,
                                             List<ProductDetails> products)
        {
          setProducts(result, products);
        }
      };

  private final PurchasesUpdatedListener m_purchases_updated_listener =
      new PurchasesUpdatedListener() {
        @Override
        public void onPurchasesUpdated(BillingResult result,
                                       List<Purchase> purchases)
        {
          processPurchases(result, purchases);
        }
      };

  private final PurchasesResponseListener m_purchases_response_listener =
      new PurchasesResponseListener() {
        @Override
        public void onQueryPurchasesResponse(BillingResult result,
                                             List<Purchase> purchases)
        {
          processPurchases(result, purchases);
        }
      };

  private final ConsumeResponseListener m_consume_response_listener =
      new ConsumeResponseListener() {
        @Override
        public void onConsumeResponse(BillingResult result,
                                      String purchaseToken)
        {
          if (result.getResponseCode() != BillingClient.BillingResponseCode.OK)
            Log.e("Bim",
                  "Consume error, response_code=" + result.getResponseCode()
                      + ": " + result.getDebugMessage());
        }
      };

  private long m_products_ready = 0;
  private long m_products_error = 0;
  private long m_purchase_completed = 0;
  private long m_purchase_error = 0;

  private final HashMap<String, ProductDetails> m_products =
      new HashMap<String, ProductDetails>();

  public PlayServicesShop(Activity activity)
  {
    m_activity = activity;

    final PendingPurchasesParams pending_purchases =
        PendingPurchasesParams.newBuilder().enableOneTimeProducts().build();

    m_client = BillingClient.newBuilder(activity)
                   .setListener(m_purchases_updated_listener)
                   .enablePendingPurchases(pending_purchases)
                   .build();

    connectClient();
  }

  public boolean supported()
  {
    return true;
  }

  public void setCallbacks(long products_ready, long products_error,
                           long purchase_completed, long purchase_error)
  {
    m_products_ready = products_ready;
    m_products_error = products_error;
    m_purchase_completed = purchase_completed;
    m_purchase_error = purchase_error;
  }

  public void fetchProducts(List<String> ids)
  {
    if (ids.isEmpty())
      return;

    final List<QueryProductDetailsParams.Product> products =
        new ArrayList<>(ids.size());

    for (String id : ids)
      products.add(QueryProductDetailsParams.Product.newBuilder()
                       .setProductId(id)
                       .setProductType(BillingClient.ProductType.INAPP)
                       .build());

    final QueryProductDetailsParams params =
        QueryProductDetailsParams.newBuilder()
            .setProductList(products)
            .build();

    m_client.queryProductDetailsAsync(params, m_product_detail_listener);
  }

  public void refreshPurchases()
  {
    final QueryPurchasesParams params =
        QueryPurchasesParams.newBuilder()
            .setProductType(BillingClient.ProductType.INAPP)
            .build();

    m_client.queryPurchasesAsync(params, m_purchases_response_listener);
  }

  public void purchase(String id)
  {
    final ProductDetails p = m_products.get(id);

    if (p == null)
      {
        Log.e("Bim", "Can't purchase unknown product '" + id + "'.");

        if (m_purchase_error != 0)
          NativeCall.call(m_purchase_error);

        return;
      }

    final BillingFlowParams.ProductDetailsParams details =
        BillingFlowParams.ProductDetailsParams.newBuilder()
            .setProductDetails(p)
            .build();

    final BillingFlowParams flow =
        BillingFlowParams.newBuilder()
            .setProductDetailsParamsList(List.of(details))
            .build();

    final BillingResult result = m_client.launchBillingFlow(m_activity, flow);

    if (result.getResponseCode() != BillingClient.BillingResponseCode.OK)
      {
        Log.e("Bim",
              "Purchase error, response_code=" + result.getResponseCode()
                  + ": " + result.getDebugMessage());

        if (m_purchase_error != 0)
          NativeCall.call(m_purchase_error);
      }
  }

  public void consumePurchase(String token)
  {
    final ConsumeParams params =
        ConsumeParams.newBuilder().setPurchaseToken(token).build();

    m_client.consumeAsync(params, m_consume_response_listener);
  }

  private void connectClient()
  {
    m_client.startConnection(m_client_state_listener);
  }

  private void setProducts(BillingResult result, List<ProductDetails> products)
  {
    if (result.getResponseCode() != BillingClient.BillingResponseCode.OK)
      {
        Log.e("Bim",
              "Error fetching shop products: " + result.getDebugMessage());

        if (m_products_error != 0)
          NativeCall.call(m_products_error);

        return;
      }

    m_products.clear();

    final HashMap<String, String> prices = new HashMap<String, String>();

    for (ProductDetails p : products)
      {
        m_products.put(p.getProductId(), p);
        prices.put(p.getProductId(),
                   p.getOneTimePurchaseOfferDetails().getFormattedPrice());
      }

    if (m_products_ready != 0)
      NativeCall.call(m_products_ready, prices);
  }

  private void processPurchases(BillingResult result, List<Purchase> purchases)
  {
    if ((purchases == null)
        || (result.getResponseCode() != BillingClient.BillingResponseCode.OK)
        || (m_purchase_completed == 0))
      {
        return;
      }

    for (Purchase purchase : purchases)
      {
        if (purchase.getPurchaseState() != Purchase.PurchaseState.PURCHASED)
          continue;

        if (purchase.isAcknowledged())
          {
            notifyPurchase(purchase);
            continue;
          }

        final AcknowledgePurchaseParams params =
            AcknowledgePurchaseParams.newBuilder()
                .setPurchaseToken(purchase.getPurchaseToken())
                .build();

        m_client.acknowledgePurchase(params, r -> {
          if (r.getResponseCode() == BillingClient.BillingResponseCode.OK)
            notifyPurchase(purchase);
        });
      }
  }

  private void notifyPurchase(Purchase purchase)
  {
    final int quantity = purchase.getQuantity();

    for (String id : purchase.getProducts())
      NativeCall.call(m_purchase_completed, id, quantity,
                      purchase.getPurchaseToken());
  }
}
