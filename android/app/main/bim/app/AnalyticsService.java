package bim.app;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import androidx.annotation.Keep;
import com.posthog.PostHog;
import com.posthog.android.PostHogAndroid;
import com.posthog.android.PostHogAndroidConfig;
import java.security.MessageDigest;
import java.util.HashMap;
import java.util.Map;
import org.apache.commons.codec.binary.Hex;

@Keep
class AnalyticsService
{
  static public void init(Activity activity, boolean with_shop)
  {
    final PostHogAndroidConfig config = new PostHogAndroidConfig(
        "phc_57UXj7TS4OWO4UOPydBUssez5Z7OarK2BRS4QfjSo07",
        "https://eu.i.posthog.com");

    PostHogAndroid.Companion.setup(activity, config);

    send_start_event(activity, with_shop);
  }

  static public void screen(String name)
  {
    screen(name, null);
  }

  static public void screen(String name, Map<String, String> properties)
  {
    PostHog.Companion.screen(name, properties);
  }

  static public void event(String name)
  {
    event(name, null);
  }

  static public void event(String name, Map<String, String> properties)
  {
    PostHog.Companion.capture(name, null, properties, null, null, null);
  }

  static private void send_start_event(Activity activity, boolean with_shop)
  {
    final HashMap<String, String> properties = new HashMap<String, String>();
    final Context context = activity.getApplicationContext();
    final String package_name = context.getPackageName();

    properties.put("app", package_name);
    properties.put("shop", with_shop ? "yes" : "no");

    try
      {
        for (Signature signature :
             context.getPackageManager()
                 .getPackageInfo(package_name, PackageManager.GET_SIGNATURES)
                 .signatures)
          {
            final MessageDigest message_digest =
                MessageDigest.getInstance("SHA-256");
            message_digest.update(signature.toByteArray());

            final String str = Hex.encodeHexString(message_digest.digest());
            properties.put("signature", str);

            break;
          }
      }
    catch (Throwable t)
      {}

    event("start", properties);
  }
}
