package bim.app;

import java.util.List;

interface IShop
{
  boolean supported();

  void setCallbacks(long products_ready, long products_error,
                    long purchase_completed, long purchase_error);

  void fetchProducts(List<String> ids);
  void refreshPurchases();
  void purchase(String id);
  void consumePurchase(String token);
}
