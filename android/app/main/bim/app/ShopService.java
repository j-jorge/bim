package bim.app;

import android.app.Activity;
import androidx.annotation.Keep;
import iscool.log.Log;
import java.util.List;

@Keep
class ShopService
{
  static private IShop m_impl = null;

  static public void init(Activity activity)
  {
    m_impl = new PlayServicesShop(activity);
  }

  static public boolean supported()
  {
    return (m_impl != null) && m_impl.supported();
  }

  static public void setCallbacks(long products_ready, long products_error,
                                  long purchase_completed, long purchase_error)
  {
    if (m_impl != null)
      m_impl.setCallbacks(products_ready, products_error, purchase_completed,
                          purchase_error);
    else
      Log.e("Bim", "Can't set shop callbacks, impl is null.");
  }

  static public void fetchProducts(List<String> ids)
  {
    try
      {
        if (m_impl != null)
          m_impl.fetchProducts(ids);
        else
          Log.e("Bim", "Can't fetch the shop products, impl is null.");
      }
    catch (Throwable e)
      {
        Log.e("Bim", "Error in fetchProducts().", e);
      }
  }

  static public void refreshPurchases()
  {
    try
      {
        m_impl.refreshPurchases();
      }
    catch (Throwable e)
      {
        Log.e("Bim", "Error in refreshPurchases().", e);
      }
  }

  static public void purchase(String id)
  {
    try
      {
        m_impl.purchase(id);
      }
    catch (Throwable e)
      {
        Log.e("Bim", "Error in purchase(" + id + ")", e);
      }
  }

  static public void consumePurchase(String token)
  {
    try
      {
        m_impl.consumePurchase(token);
      }
    catch (Throwable e)
      {
        Log.e("Bim", "Error in consumePurchase(" + token + ")", e);
      }
  }
}
