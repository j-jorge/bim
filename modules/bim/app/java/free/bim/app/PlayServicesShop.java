// SPDX-License-Identifier: AGPL-3.0-only
package bim.app;

import android.app.Activity;
import bim.app.IShop;
import iscool.jni.NativeCall;
import java.util.List;

class PlayServicesShop implements IShop
{
  private long m_products_error = 0;
  private long m_purchase_error = 0;

  public PlayServicesShop(Activity activity)
  {}

  public boolean supported()
  {
    return false;
  }

  public void setCallbacks(long products_ready, long products_error,
                           long purchase_completed, long purchase_error)
  {
    m_products_error = products_error;
    m_purchase_error = purchase_error;
  }

  public void fetchProducts(List<String> ids)
  {
    if (m_products_error != 0)
      NativeCall.call(m_products_error);
  }

  public void refreshPurchases()
  {}

  public void purchase(String id)
  {
    if (m_purchase_error != 0)
      NativeCall.call(m_purchase_error);
  }

  public void consumePurchase(String token)
  {}
}
