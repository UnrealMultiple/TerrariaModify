package zig.cheat.qq;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.widget.Toast;

import zig.cheat.qq.ui.Menu;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("TerrariaModify");
    }

    private Menu menu;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Menu.setHighRefreshRate(this);
        menu = new Menu(this);
        menu.show();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (menu != null) menu.destroy();
    }
}
