package zig.cheat.qq.ui.dialogs;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.os.Handler;
import android.os.Looper;
import android.text.InputType;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;

import zig.cheat.qq.jni.Jni;
import zig.cheat.qq.ui.Menu;
import zig.cheat.qq.ui.Utils;

public class ItemModifierDialog {

    private static final ExecutorService executor = Executors.newCachedThreadPool(new ThreadFactory() {
        private final AtomicLong counter = new AtomicLong(0);
        @Override
        public Thread newThread(Runnable r) {
            Thread t = new Thread(r, "ItemModifier-" + counter.incrementAndGet());
            t.setDaemon(true);
            return t;
        }
    });

    private final Context context;
    private final Handler handler;
    private final AtomicBoolean isDestroyed = new AtomicBoolean(false);
    private Dialog dialog;
    private float density;

    private TextView tvName, tvId;
    private LinearLayout attributesContainer;
    private List<AttributeEditor> attributeEditors = new ArrayList<>();
    private View loadingView;

    private static class AttributeEditor {
        String key;
        EditText input;
        String typeHint;
        AttributeEditor(String key, EditText input, String typeHint) {
            this.key = key;
            this.input = input;
            this.typeHint = typeHint;
        }
    }

    public ItemModifierDialog(Context ctx) {
        this.context = ctx;
        this.handler = new Handler(Looper.getMainLooper());
    }

    public void show() {
        if (context == null || !(context instanceof Activity) || ((Activity)context).isFinishing()) return;

        if (dialog != null && dialog.isShowing()) dialog.dismiss();

        try {
            createDialog();
            loadItemData();
        } catch (Exception e) {
            safeToast("Error: " + e.getMessage());
        }
    }

    private void createDialog() {
        dialog = new Dialog(context, android.R.style.Theme_Translucent_NoTitleBar);
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        dialog.setCancelable(true);
        dialog.setOnDismissListener(d -> cleanup());

        density = Math.max(1.0f, context.getResources().getDisplayMetrics().density);
        final float d = density;
        // 宽度自适应：不超过屏幕宽度减去左右边距
        int screenWidth = context.getResources().getDisplayMetrics().widthPixels;
        int maxWidth = screenWidth - (int)(32 * d); // 左右各留16dp
        final int dialogWidth = Math.min((int)(550 * d), maxWidth);

        LinearLayout root = new LinearLayout(context);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding((int)(16*d), (int)(8*d), (int)(16*d), (int)(8*d));

        GradientDrawable bg = new GradientDrawable();
        bg.setColor(Menu.COLOR_WINDOW_BG);
        bg.setCornerRadius(12*d);
        bg.setStroke((int)(1*d), Menu.COLOR_BORDER);
        root.setBackground(bg);

        root.addView(createTitle("ITEM MODIFIER", d));
        root.addView(createDivider(d));

        root.addView(createSectionTitle("ITEM INFO", d));
        LinearLayout infoRow = new LinearLayout(context);
        infoRow.setOrientation(LinearLayout.HORIZONTAL);
        infoRow.setLayoutParams(createLP(-1, -2, (int)(6*d), 0));

        LinearLayout nameCol = new LinearLayout(context);
        nameCol.setOrientation(LinearLayout.VERTICAL);
        nameCol.setLayoutParams(new LinearLayout.LayoutParams(0, -2, 1));
        nameCol.addView(createMiniLabel("NAME", d));
        tvName = createReadOnlyText("", d);
        nameCol.addView(tvName);
        infoRow.addView(nameCol);

        infoRow.addView(createSpacer(d));

        LinearLayout idCol = new LinearLayout(context);
        idCol.setOrientation(LinearLayout.VERTICAL);
        idCol.setLayoutParams(new LinearLayout.LayoutParams(0, -2, 1));
        idCol.addView(createMiniLabel("ID", d));
        tvId = createReadOnlyText("", d);
        idCol.addView(tvId);
        infoRow.addView(idCol);

        root.addView(infoRow);

        root.addView(createSectionTitle("ATTRIBUTES", d));

        // ScrollView 先设置为 MATCH_PARENT，稍后动态调整高度
        ScrollView scrollView = new ScrollView(context);
        scrollView.setLayoutParams(new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.MATCH_PARENT
        ));
        attributesContainer = new LinearLayout(context);
        attributesContainer.setOrientation(LinearLayout.VERTICAL);

        loadingView = createLoadingView(d);
        attributesContainer.addView(loadingView);

        scrollView.addView(attributesContainer);
        root.addView(scrollView);

        LinearLayout buttonRow = new LinearLayout(context);
        buttonRow.setOrientation(LinearLayout.HORIZONTAL);
        buttonRow.setLayoutParams(createLP(-1, (int)(36*d), (int)(8*d), 0));

        Button cancelBtn = createActionButton("CANCEL", 0xFF666666, d, v -> dismiss());
        Button saveBtn = createActionButton("SAVE", 0xFF10B981, d, v -> saveChanges());

        buttonRow.addView(cancelBtn);
        buttonRow.addView(createSpacer(d));
        buttonRow.addView(saveBtn);
        root.addView(buttonRow);

        dialog.setContentView(root);

        Window window = dialog.getWindow();
        if (window != null) {
            window.setLayout(dialogWidth, WindowManager.LayoutParams.WRAP_CONTENT);
            window.setBackgroundDrawableResource(android.R.color.transparent);
            window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        }

        dialog.show();

        // 对话框显示后动态调整 ScrollView 高度，使其适应屏幕剩余空间
        adjustScrollViewHeight(dialog, scrollView, root, d);
    }

    /**
     * 动态调整 ScrollView 的高度，使其填充屏幕剩余可用空间
     */
    private void adjustScrollViewHeight(Dialog dialog, ScrollView scrollView, View root, float d) {
        if (dialog == null || dialog.getWindow() == null) return;

        dialog.getWindow().getDecorView().post(() -> {
            if (isDestroyed.get()) return;

            // 获取屏幕可用高度（去除状态栏和导航栏）
            int screenHeight = getScreenHeight(context);
            int statusBarHeight = getStatusBarHeight(context);
            int navBarHeight = getNavigationBarHeight(context);
            int availableHeight = screenHeight - statusBarHeight - navBarHeight;

            // 计算固定部分的总高度（除 ScrollView 外的所有子视图）
            int fixedHeight = 0;
            if (root instanceof ViewGroup) {
                ViewGroup group = (ViewGroup) root;
                for (int i = 0; i < group.getChildCount(); i++) {
                    View child = group.getChildAt(i);
                    if (child != scrollView) {
                        fixedHeight += child.getMeasuredHeight();
                        // 加上子视图的 margin
                        ViewGroup.MarginLayoutParams lp = (ViewGroup.MarginLayoutParams) child.getLayoutParams();
                        fixedHeight += lp.topMargin + lp.bottomMargin;
                    }
                }
            }

            // 加上根布局的 padding
            fixedHeight += root.getPaddingTop() + root.getPaddingBottom();

            // 计算 ScrollView 的理想高度，留出一些边距（例如 16dp）
            int maxScrollHeight = availableHeight - fixedHeight - (int)(16 * d);
            if (maxScrollHeight < (int)(100 * d)) {
                maxScrollHeight = (int)(200 * d);  // 设置一个最小高度
            }

            // 设置 ScrollView 的高度
            LinearLayout.LayoutParams lp = (LinearLayout.LayoutParams) scrollView.getLayoutParams();
            lp.height = maxScrollHeight;
            scrollView.setLayoutParams(lp);
        });
    }

    // 辅助方法：获取屏幕高度
    private int getScreenHeight(Context context) {
        return context.getResources().getDisplayMetrics().heightPixels;
    }

    // 辅助方法：获取状态栏高度
    private int getStatusBarHeight(Context context) {
        int result = 0;
        int resourceId = context.getResources().getIdentifier("status_bar_height", "dimen", "android");
        if (resourceId > 0) {
            result = context.getResources().getDimensionPixelSize(resourceId);
        }
        return result;
    }

    // 辅助方法：获取导航栏高度
    private int getNavigationBarHeight(Context context) {
        int result = 0;
        int resourceId = context.getResources().getIdentifier("navigation_bar_height", "dimen", "android");
        if (resourceId > 0) {
            result = context.getResources().getDimensionPixelSize(resourceId);
        }
        return result;
    }

    @SuppressLint("SetTextI18n")
    private View createLoadingView(float d) {
        TextView tv = new TextView(context);
        tv.setText("Loading item data...");
        tv.setTextColor(Menu.COLOR_TEXT_SECONDARY);
        tv.setTextSize(10);
        tv.setGravity(Gravity.CENTER);
        tv.setPadding(0, (int)(30*d), 0, (int)(30*d));
        return tv;
    }

    private void showEmptyState(String message) {
        TextView tv = new TextView(context);
        tv.setText(message);
        tv.setTextColor(0xFFEF4444);
        tv.setTextSize(10);
        tv.setGravity(Gravity.CENTER);
        tv.setPadding(0, (int)(30*density), 0, (int)(30*density));
        attributesContainer.addView(tv);
    }

    private void loadItemData() {
        executor.execute(() -> {
            try {
                String json = Jni.getCurrentItemJson();
                handler.post(() -> {
                    if (isDestroyed.get()) return;
                    if (json == null || json.isEmpty()) {
                        attributesContainer.removeView(loadingView);
                        showEmptyState("No item data available");
                        safeToast("Failed to load item data");
                    } else {
                        populateData(json);
                    }
                });
            } catch (Exception e) {
                handler.post(() -> {
                    if (isDestroyed.get()) return;
                    attributesContainer.removeView(loadingView);
                    showEmptyState("Error: " + e.getMessage());
                    safeToast("Error loading data: " + e.getMessage());
                });
            }
        });
    }

    private void populateData(String json) {
        if (isDestroyed.get() || attributesContainer == null) return;
        attributesContainer.removeView(loadingView);
        attributeEditors.clear();

        try {
            parseAndPopulate(json);
        } catch (Exception e) {
            showEmptyState("Parse error: " + e.getMessage());
            safeToast("Data parsing failed");
        }
    }

    private void parseAndPopulate(String json) {
        final float d = density;

        try {
            JSONObject obj = new JSONObject(json);

            String name = obj.optString("name", "Unknown");
            int id = obj.optInt("id", 0);
            tvName.setText(name);
            tvId.setText(String.valueOf(id));

            JSONObject attrs;
            if (obj.has("attributes")) {
                attrs = obj.getJSONObject("attributes");
            } else {
                // 如果属性直接放在根，则排除 name 和 id
                attrs = new JSONObject();
                Iterator<String> keys = obj.keys();
                while (keys.hasNext()) {
                    String key = keys.next();
                    if (!key.equals("name") && !key.equals("id")) {
                        attrs.put(key, obj.get(key));
                    }
                }
            }

            Iterator<String> keys = attrs.keys();
            while (keys.hasNext()) {
                String key = keys.next();
                Object value = attrs.get(key);
                String strValue = value != null ? value.toString() : "";

                String typeHint = "text";
                int inputType = InputType.TYPE_CLASS_TEXT;
                if (value instanceof Integer || value instanceof Long) {
                    typeHint = "number";
                    inputType = InputType.TYPE_CLASS_NUMBER;
                } else if (value instanceof Double || value instanceof Float) {
                    typeHint = "decimal";
                    inputType = InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_DECIMAL;
                } else if (value instanceof Boolean) {
                    typeHint = "boolean";
                }

                addAttributeRow(key, strValue, typeHint, inputType, d);
            }

        } catch (JSONException e) {
            throw new RuntimeException("JSON parse error", e);
        }
    }

    private void addAttributeRow(String key, String currentValue, String typeHint, int inputType, float d) {
        LinearLayout row = new LinearLayout(context);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setLayoutParams(createLP(-1, -2, (int)(4*d), 0));

        TextView label = new TextView(context);
        label.setText(key);
        label.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        label.setTextSize(10);
        label.setTypeface(Utils.getFont(context));
        label.setLayoutParams(new LinearLayout.LayoutParams(0, -2, 0.4f));
        row.addView(label);

        EditText input = new EditText(context);
        input.setText(currentValue);
        input.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        input.setHintTextColor(0xFF666666);
        input.setTextSize(10);
        input.setTypeface(Typeface.MONOSPACE);
        input.setInputType(inputType);
        input.setBackground(createInputBg(d));
        input.setPadding((int)(8*d), (int)(6*d), (int)(8*d), (int)(6*d));
        input.setLayoutParams(new LinearLayout.LayoutParams(0, (int)(32*d), 0.6f));

        // 确保可以获取焦点并弹出输入法
        input.setFocusable(true);
        input.setFocusableInTouchMode(true);
        input.setClickable(true);
        input.setLongClickable(true);

        row.addView(input);

        attributesContainer.addView(row);
        attributeEditors.add(new AttributeEditor(key, input, typeHint));
    }

    private void saveChanges() {
        try {
            JSONObject result = new JSONObject();
            result.put("name", tvName.getText().toString());
            result.put("id", Integer.parseInt(tvId.getText().toString()));

            JSONObject attrs = new JSONObject();
            for (AttributeEditor editor : attributeEditors) {
                String valueStr = editor.input.getText().toString().trim();
                Object finalValue;
                if ("number".equals(editor.typeHint)) {
                    finalValue = Integer.parseInt(valueStr);
                } else if ("decimal".equals(editor.typeHint)) {
                    finalValue = Double.parseDouble(valueStr);
                } else if ("boolean".equals(editor.typeHint)) {
                    finalValue = Boolean.parseBoolean(valueStr) || "1".equals(valueStr);
                } else {
                    finalValue = valueStr;
                }
                attrs.put(editor.key, finalValue);
            }
            result.put("attributes", attrs);

            final String modifiedJson = result.toString();

            executor.execute(() -> {
                try {
                    boolean success = Jni.updateItemAttributes(modifiedJson);
                    handler.post(() -> {
                        if (success) {
                            safeToast("Saved successfully");
                            dismiss();
                        } else {
                            safeToast("Save failed");
                        }
                    });
                } catch (Exception e) {
                    handler.post(() -> safeToast("Error: " + e.getMessage()));
                }
            });

        } catch (Exception e) {
            safeToast("Invalid input: " + e.getMessage());
        }
    }

    private TextView createTitle(String text, float d) {
        TextView tv = new TextView(context);
        tv.setText(text);
        tv.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        tv.setTextSize(12);
        tv.setTypeface(Utils.getFont(context), Typeface.BOLD);
        tv.setLetterSpacing(0.1f);
        return tv;
    }

    private TextView createSectionTitle(String text, float d) {
        TextView tv = new TextView(context);
        tv.setText(text);
        tv.setTextColor(Menu.COLOR_TEXT_SECONDARY);
        tv.setTextSize(8);
        tv.setTypeface(Utils.getFont(context));
        tv.setPadding(0, (int)(8*d), 0, (int)(2*d));
        tv.setLetterSpacing(0.08f);
        return tv;
    }

    private TextView createMiniLabel(String text, float d) {
        TextView tv = new TextView(context);
        tv.setText(text);
        tv.setTextColor(Menu.COLOR_TEXT_SECONDARY);
        tv.setTextSize(7);
        tv.setTypeface(Utils.getFont(context));
        tv.setPadding(0, 0, 0, (int)(2*d));
        return tv;
    }

    private TextView createReadOnlyText(String text, float d) {
        TextView tv = new TextView(context);
        tv.setText(text);
        tv.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        tv.setTextSize(10);
        tv.setTypeface(Utils.getFont(context));
        tv.setPadding((int)(8*d), (int)(6*d), (int)(8*d), (int)(6*d));
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(0xFF1A1A1A);
        bg.setCornerRadius(6*d);
        bg.setStroke((int)(1*d), 0xFF333333);
        tv.setBackground(bg);
        return tv;
    }

    private View createDivider(float d) {
        View v = new View(context);
        v.setLayoutParams(createLP(-1, (int)(1*d), (int)(8*d), 0));
        v.setBackgroundColor(Menu.COLOR_DIVIDER);
        return v;
    }

    private View createSpacer(float d) {
        View v = new View(context);
        v.setLayoutParams(new LinearLayout.LayoutParams((int)(4*d), 0));
        return v;
    }

    private LinearLayout.LayoutParams createLP(int w, int h, int top, int bottom) {
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(w, h);
        lp.topMargin = top;
        lp.bottomMargin = bottom;
        return lp;
    }

    private GradientDrawable createInputBg(float d) {
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(0xFF0F0F0F);
        bg.setCornerRadius(6*d);
        bg.setStroke((int)(1*d), 0xFF333333);
        return bg;
    }

    private Button createActionButton(String text, int color, float d, View.OnClickListener listener) {
        Button btn = new Button(context);
        btn.setText(text);
        btn.setTextColor(Color.WHITE);
        btn.setTextSize(11);
        btn.setTypeface(Utils.getFont(context), Typeface.BOLD);
        btn.setAllCaps(false);
        btn.setPadding((int)(8*d), (int)(6*d), (int)(8*d), (int)(6*d));
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(color);
        bg.setCornerRadius(6*d);
        btn.setBackground(bg);
        btn.setLayoutParams(new LinearLayout.LayoutParams(0, -1, 1));
        btn.setOnClickListener(v -> safeExecute(() -> listener.onClick(v)));
        return btn;
    }

    private void safeExecute(Runnable r) {
        if (isDestroyed.get()) return;
        try {
            r.run();
        } catch (Exception e) {
            safeToast("Error: " + e.getMessage());
        }
    }

    private void safeToast(String msg) {
        if (context != null && msg != null) {
            handler.post(() -> {
                if (!isDestroyed.get()) {
                    Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
                }
            });
        }
    }

    private void cleanup() {
        isDestroyed.set(true);
        handler.removeCallbacksAndMessages(null);
        //executor.shutdown();
        attributeEditors.clear();
    }

    public void dismiss() {
        if (dialog != null && dialog.isShowing()) {
            try {
                dialog.dismiss();
            } catch (Exception ignored) {}
        }
        cleanup();
    }
}