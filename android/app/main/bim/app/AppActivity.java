package bim.app;

import android.os.Build;
import android.os.Bundle;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import bim.app.ShopService;
import dev.axmol.lib.AxmolActivity;
import iscool.jni.JniService;
import iscool.jni.NativeLog;
import iscool.log.Log;
import iscool.social.LinkService;
import iscool.social.ShareService;
import iscool.system.SystemService;

public class AppActivity extends AxmolActivity
{
  @Override
  protected void onCreate(Bundle savedInstanceState)
  {
    super.setEnableVirtualButton(false);
    super.onCreate(savedInstanceState);

    JniService.init(this);
    Log.registerDelegate(NativeLog.defaultInstance());
    LinkService.init(this);
    ShareService.init(this, getPackageName() + ".fileprovider");
    SystemService.init(this);
    ShopService.init(this);

    // Workaround in
    // https://stackoverflow.com/questions/16283079/re-launch-of-activity-on-home-button-but-only-the-first-time/16447508
    if (!isTaskRoot())
      {
        // Android launched another instance of the root activity
        // into an existing task so just quietly finish and go
        // away, dropping the user back into the activity at the
        // top of the stack (ie: the last state of this task)
        // Don't need to finish it again since it's finished in
        // super.onCreate .
        return;
      }

    // Make sure we're running on Pie or higher to change cutout mode
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
      {
        // Enable rendering into the cutout area
        WindowManager.LayoutParams lp = getWindow().getAttributes();
        lp.layoutInDisplayCutoutMode =
            WindowManager.LayoutParams
                .LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        getWindow().setAttributes(lp);
      }
  }
}
