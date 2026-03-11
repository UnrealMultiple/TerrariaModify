package zig.cheat.qq;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.ResultReceiver;

public class FilePickerActivity extends Activity {
    private static final int REQUEST_CODE_PICK_FILE = 1000;
    private ResultReceiver resultReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        resultReceiver = getIntent().getParcelableExtra("receiver");

        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        String[] mimeTypes = {"text/plain", "application/octet-stream"};
        intent.putExtra(Intent.EXTRA_MIME_TYPES, mimeTypes);
        startActivityForResult(intent, REQUEST_CODE_PICK_FILE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE_PICK_FILE) {
            if (resultCode == RESULT_OK && data != null) {
                // 将选中的 Uri 通过 ResultReceiver 发送回去
                if (resultReceiver != null) {
                    Bundle bundle = new Bundle();
                    bundle.putParcelable("uri", data.getData());
                    resultReceiver.send(Activity.RESULT_OK, bundle);
                }
            } else {
                // 用户取消或出错，发送空结果
                if (resultReceiver != null) {
                    resultReceiver.send(Activity.RESULT_CANCELED, null);
                }
            }
            finish(); // 关闭代理 Activity
        }
    }
}