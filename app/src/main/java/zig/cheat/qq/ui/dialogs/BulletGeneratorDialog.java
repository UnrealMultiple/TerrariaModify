package zig.cheat.qq.ui.dialogs;

import android.animation.ValueAnimator;
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
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.DecelerateInterpolator;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
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

public class BulletGeneratorDialog {

    // ========== 可调常量 ==========
    private static final int TITLE_BAR_PADDING_VERTICAL_DP = 2;
    private static final int CONTENT_VERTICAL_SPACING_DP = 8;
    private static final int ANIMATION_DURATION_MS = 200;
    private static final int CARD_MARGIN_BOTTOM_DP = 8;
    private static final int DIALOG_FIXED_HEIGHT_DP = 400;
    // =============================

    private static final ExecutorService executor = Executors.newCachedThreadPool(new ThreadFactory() {
        private final AtomicLong counter = new AtomicLong(0);
        @Override
        public Thread newThread(Runnable r) {
            Thread t = new Thread(r, "BulletGenerator-" + counter.incrementAndGet());
            t.setDaemon(true);
            return t;
        }
    });

    private final Context context;
    private final Handler handler;
    private final AtomicBoolean isDestroyed = new AtomicBoolean(false);
    private Dialog dialog;
    private float density;

    private LinearLayout rightContainer;
    private List<WeaponEntry> weaponEntries = new ArrayList<>();

    private static final String CONFIG_FILE_NAME = "bullet_config.json";

    public BulletGeneratorDialog(Context ctx) {
        this.context = ctx;
        this.handler = new Handler(Looper.getMainLooper());
    }

    public void show() {
        if (context == null || !(context instanceof Activity) || ((Activity) context).isFinishing()) return;
        if (dialog != null && dialog.isShowing()) dialog.dismiss();
        try {
            createDialog();
            loadExistingConfig();
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
        final int width = (int) (720 * d);
        final int height = DIALOG_FIXED_HEIGHT_DP > 0 ? (int) (DIALOG_FIXED_HEIGHT_DP * d) : WindowManager.LayoutParams.WRAP_CONTENT;

        LinearLayout root = new LinearLayout(context);
        root.setOrientation(LinearLayout.HORIZONTAL);
        root.setPadding((int) (20 * d), (int) (12 * d), (int) (20 * d), (int) (12 * d));

        GradientDrawable bg = new GradientDrawable();
        bg.setColor(Menu.COLOR_WINDOW_BG);
        bg.setCornerRadius(16 * d);
        bg.setStroke((int) (1 * d), Menu.COLOR_BORDER);
        root.setBackground(bg);

        // 左侧按钮区
        LinearLayout leftPanel = new LinearLayout(context);
        leftPanel.setOrientation(LinearLayout.VERTICAL);
        leftPanel.setLayoutParams(new LinearLayout.LayoutParams((int) (110 * d), -1));
        leftPanel.setPadding(0, (int) (8 * d), (int) (12 * d), (int) (8 * d));

        Button addWeaponBtn = createLeftButton("添加武器", 0xFF10B981, d, v -> showAddWeaponDialog());
        Button attachBtn = createLeftButton("附加", 0xFF3B82F6, d, v -> attachConfig());

        LinearLayout.LayoutParams addParams = (LinearLayout.LayoutParams) addWeaponBtn.getLayoutParams();
        addParams.bottomMargin = (int) (12 * d);
        addWeaponBtn.setLayoutParams(addParams);

        leftPanel.addView(addWeaponBtn);
        leftPanel.addView(attachBtn);

        root.addView(leftPanel);

        // 右侧内容区
        LinearLayout rightPanel = new LinearLayout(context);
        rightPanel.setOrientation(LinearLayout.VERTICAL);
        rightPanel.setLayoutParams(new LinearLayout.LayoutParams(0, -1, 1));

        rightPanel.addView(createTitle("弹幕生成器", d));
        rightPanel.addView(createDivider(d));

        ScrollView scrollView = new ScrollView(context);
        scrollView.setLayoutParams(new LinearLayout.LayoutParams(-1, -1));
        scrollView.setPadding(0, (int) (4 * d), 0, 0);
        rightContainer = new LinearLayout(context);
        rightContainer.setOrientation(LinearLayout.VERTICAL);
        rightContainer.setPadding(0, 0, 0, (int) (4 * d));
        scrollView.addView(rightContainer);
        rightPanel.addView(scrollView);

        root.addView(rightPanel);

        dialog.setContentView(root);

        Window window = dialog.getWindow();
        if (window != null) {
            window.setLayout(width, height);
            window.setBackgroundDrawableResource(android.R.color.transparent);
            window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        }

        dialog.show();
    }

    // ========== 统一按钮缩放动画 ==========
    private void applyScaleAnimation(View button) {
        button.setOnTouchListener((v, event) -> {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    v.animate().scaleX(0.95f).scaleY(0.95f).setDuration(100).start();
                    break;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    v.animate().scaleX(1.0f).scaleY(1.0f).setDuration(100).start();
                    break;
            }
            return false; // 不消耗事件，允许点击继续传递
        });
    }

    // [Modified] 武器ID输入对话框 - 完全自定义，与BuffGeneratorDialog统一风格
    private void showAddWeaponDialog() {
        final float d = density;
        Dialog inputDialog = new Dialog(context, android.R.style.Theme_Translucent_NoTitleBar);
        inputDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        inputDialog.setCancelable(true);

        // 根布局
        LinearLayout root = new LinearLayout(context);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding((int) (24 * d), (int) (24 * d), (int) (24 * d), (int) (24 * d));

        GradientDrawable bg = new GradientDrawable();
        bg.setColor(Menu.COLOR_WINDOW_BG);
        bg.setCornerRadius(16 * d);
        bg.setStroke((int) (1 * d), Menu.COLOR_BORDER);
        root.setBackground(bg);

        // 标题
        TextView title = new TextView(context);
        title.setText("输入武器ID");
        title.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        title.setTextSize(18);
        title.setTypeface(Utils.getFont(context), Typeface.BOLD);
        title.setGravity(Gravity.CENTER);
        title.setPadding(0, 0, 0, (int) (16 * d));
        root.addView(title);

        // 输入框
        EditText input = new EditText(context);
        input.setHint("例如: 1001");
        input.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        input.setHintTextColor(Menu.COLOR_TEXT_SECONDARY);
        input.setTextSize(14);
        input.setTypeface(Typeface.MONOSPACE);
        input.setInputType(InputType.TYPE_CLASS_NUMBER);
        input.setBackground(createInputBg(d));
        input.setPadding((int) (12 * d), (int) (10 * d), (int) (12 * d), (int) (10 * d));
        LinearLayout.LayoutParams inputLp = new LinearLayout.LayoutParams(-1, (int) (48 * d));
        inputLp.bottomMargin = (int) (20 * d);
        input.setLayoutParams(inputLp);
        root.addView(input);

        // 按钮行
        LinearLayout buttonRow = new LinearLayout(context);
        buttonRow.setOrientation(LinearLayout.HORIZONTAL);
        buttonRow.setGravity(Gravity.CENTER);

        Button cancelBtn = new Button(context);
        cancelBtn.setText("取消");
        cancelBtn.setTextColor(Color.WHITE);
        cancelBtn.setTextSize(14);
        cancelBtn.setTypeface(Utils.getFont(context), Typeface.BOLD);
        cancelBtn.setAllCaps(false);
        GradientDrawable cancelBg = new GradientDrawable();
        cancelBg.setColor(0xFF6B7280); // 灰色
        cancelBg.setCornerRadius(8 * d);
        cancelBtn.setBackground(cancelBg);
        cancelBtn.setLayoutParams(new LinearLayout.LayoutParams(0, (int) (44 * d), 1));
        cancelBtn.setOnClickListener(v -> inputDialog.dismiss());
        applyScaleAnimation(cancelBtn); // 添加缩放动画

        Button okBtn = new Button(context);
        okBtn.setText("确定");
        okBtn.setTextColor(Color.WHITE);
        okBtn.setTextSize(14);
        okBtn.setTypeface(Utils.getFont(context), Typeface.BOLD);
        okBtn.setAllCaps(false);
        GradientDrawable okBg = new GradientDrawable();
        okBg.setColor(Menu.COLOR_ACCENT_BLUE);
        okBg.setCornerRadius(8 * d);
        okBtn.setBackground(okBg);
        okBtn.setLayoutParams(new LinearLayout.LayoutParams(0, (int) (44 * d), 1));
        okBtn.setOnClickListener(v -> {
            String weaponId = input.getText().toString().trim();
            if (weaponId.isEmpty()) {
                weaponId = "new_weapon_" + (weaponEntries.size() + 1);
            }
            addWeaponEntry(weaponId);
            inputDialog.dismiss();
        });
        applyScaleAnimation(okBtn);

        LinearLayout.LayoutParams btnLp = new LinearLayout.LayoutParams(0, (int) (44 * d), 1);
        btnLp.rightMargin = (int) (8 * d);
        cancelBtn.setLayoutParams(btnLp);
        okBtn.setLayoutParams(btnLp);

        buttonRow.addView(cancelBtn);
        buttonRow.addView(okBtn);
        root.addView(buttonRow);

        inputDialog.setContentView(root);

        Window window = inputDialog.getWindow();
        if (window != null) {
            window.setLayout((int) (400 * d), WindowManager.LayoutParams.WRAP_CONTENT);
            window.setBackgroundDrawableResource(android.R.color.transparent);
            window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
        }

        inputDialog.show();
    }

    // 统一的输入框背景
    private GradientDrawable createInputBg(float d) {
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(Menu.COLOR_CARD_BG);
        bg.setCornerRadius(8 * d);
        bg.setStroke((int) (1 * d), Menu.COLOR_BORDER);
        return bg;
    }

    /** 添加新武器卡片 */
    private void addWeaponEntry(String weaponId) {
        WeaponEntry entry = new WeaponEntry(context, weaponEntries.size() + 1, weaponId, density,
                entryToDelete -> {
                    weaponEntries.remove(entryToDelete);
                    rightContainer.removeView(entryToDelete.rootView);
                    updateAllWeaponTitles();
                });
        weaponEntries.add(entry);
        rightContainer.addView(entry.rootView);
    }

    /** 更新所有武器标题序号 */
    private void updateAllWeaponTitles() {
        for (int i = 0; i < weaponEntries.size(); i++) {
            weaponEntries.get(i).updateTitle(i + 1);
        }
    }

    /**
     * 从应用私有目录读取 bullet_config.json 文件内容
     * @param context 上下文（用于获取 files 目录）
     * @return JSON 字符串，若文件不存在或读取失败则返回 null
     */
    public static String loadConfigString(Context context) {
        if (context == null) return null;
        File file = new File(context.getFilesDir(), CONFIG_FILE_NAME);
        if (!file.exists()) return null;
        try (FileInputStream fis = new FileInputStream(file)) {
            byte[] data = new byte[(int) file.length()];
            int read = fis.read(data);
            if (read <= 0) return null;
            return new String(data, StandardCharsets.UTF_8);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    private void loadExistingConfig() {
        executor.execute(() -> {
            File file = new File(context.getFilesDir(), CONFIG_FILE_NAME);
            if (!file.exists()) {
                handler.post(() -> {
                    // 无配置，默认添加一个武器卡片方便使用
                    showAddWeaponDialog();
                });
                return;
            }
            try (FileInputStream fis = new FileInputStream(file)) {
                byte[] data = new byte[(int) file.length()];
                int read = fis.read(data);
                if (read <= 0) {
                    handler.post(this::showAddWeaponDialog);
                    return;
                }
                String jsonStr = new String(data, StandardCharsets.UTF_8);
                JSONObject rootObj = new JSONObject(jsonStr);
                handler.post(() -> {
                    rightContainer.removeAllViews();
                    weaponEntries.clear();

                    Iterator<String> keys = rootObj.keys();
                    int index = 1;
                    while (keys.hasNext()) {
                        String weaponId = keys.next();
                        try {
                            JSONArray bulletsArray = rootObj.getJSONArray(weaponId);
                            WeaponEntry entry = new WeaponEntry(context, index++, weaponId, density,
                                    entryToDelete -> {
                                        weaponEntries.remove(entryToDelete);
                                        rightContainer.removeView(entryToDelete.rootView);
                                        updateAllWeaponTitles();
                                    });
                            entry.populateFromJson(bulletsArray);
                            weaponEntries.add(entry);
                            rightContainer.addView(entry.rootView);
                        } catch (JSONException e) {
                            e.printStackTrace();
                        }
                    }
                    if (weaponEntries.isEmpty()) showAddWeaponDialog();
                });
            } catch (Exception e) {
                handler.post(() -> {
                    safeToast("读取配置失败，请手动添加武器");
                    showAddWeaponDialog();
                });
            }
        });
    }

    public void attachConfig() {
        try {
            JSONObject rootObj = new JSONObject();
            for (WeaponEntry weapon : weaponEntries) {
                rootObj.put(weapon.weaponId, weapon.toJson());
            }
            final String jsonStr = rootObj.toString();

            executor.execute(() -> {
                try {
                    File file = new File(context.getFilesDir(), CONFIG_FILE_NAME);
                    try (FileOutputStream fos = new FileOutputStream(file)) {
                        fos.write(jsonStr.getBytes(StandardCharsets.UTF_8));
                    }
                    Jni.attachBulletConfig(jsonStr);
                    handler.post(() -> safeToast("配置已保存"));
                } catch (IOException e) {
                    handler.post(() -> safeToast("保存文件失败: " + e.getMessage()));
                }
            });
        } catch (JSONException e) {
            safeToast("生成JSON失败: " + e.getMessage());
        }
    }

    // ========== 内部类：武器条目 ==========
    private class WeaponEntry {
        View rootView;
        LinearLayout contentLayout;          // 武器内容（包含添加按钮和弹幕容器）
        LinearLayout bulletsContainer;       // 存放所有弹幕卡片
        TextView titleView;
        TextView expandIcon;
        boolean expanded = false;
        boolean isAnimating = false;

        String weaponId;                      // 武器ID（作为JSON键）
        List<BulletEntry> bulletEntries = new ArrayList<>();

        private final float d;                 // 保存密度值，用于创建子视图

        interface DeleteCallback {
            void onDelete(WeaponEntry entry);
        }

        @SuppressLint("SetTextI18n")
        WeaponEntry(Context context, int index, String weaponId, float d, DeleteCallback deleteCallback) {
            this.weaponId = weaponId;
            this.d = d;

            LinearLayout card = new LinearLayout(context);
            card.setOrientation(LinearLayout.VERTICAL);
            LinearLayout.LayoutParams cardParams = new LinearLayout.LayoutParams(-1, -2);
            cardParams.bottomMargin = (int) (CARD_MARGIN_BOTTOM_DP * d);
            card.setLayoutParams(cardParams);

            GradientDrawable cardBg = new GradientDrawable();
            cardBg.setColor(0xFF1E1E1E);
            cardBg.setCornerRadius(10 * d);
            cardBg.setStroke((int) (1.5f * d), 0xFFAAAAAA);
            card.setBackground(cardBg);

            // 标题栏
            LinearLayout titleBar = new LinearLayout(context);
            titleBar.setOrientation(LinearLayout.HORIZONTAL);
            titleBar.setLayoutParams(new LinearLayout.LayoutParams(-1, -2));
            titleBar.setPadding((int) (12 * d),
                    (int) (TITLE_BAR_PADDING_VERTICAL_DP * d),
                    (int) (12 * d),
                    (int) (TITLE_BAR_PADDING_VERTICAL_DP * d));
            titleBar.setBackgroundColor(0xFF2A2A2A);
            titleBar.setClickable(true);

            expandIcon = new TextView(context);
            expandIcon.setText("▼");
            expandIcon.setTextColor(Menu.COLOR_TEXT_PRIMARY);
            expandIcon.setTextSize(12);
            expandIcon.setTypeface(Typeface.DEFAULT_BOLD);
            expandIcon.setPadding(0, 0, (int) (8 * d), 0);

            titleView = new TextView(context);
            titleView.setText("武器 " + index + " (ID: " + weaponId + ")");
            titleView.setTextColor(Menu.COLOR_TEXT_PRIMARY);
            titleView.setTextSize(12);
            titleView.setTypeface(Utils.getFont(context), Typeface.BOLD);
            titleView.setLayoutParams(new LinearLayout.LayoutParams(0, -2, 1));

            Button deleteBtn = new Button(context);
            deleteBtn.setText("✕");
            deleteBtn.setTextColor(0xFFEF4444);
            deleteBtn.setTextSize(14);
            deleteBtn.setTypeface(Typeface.DEFAULT_BOLD);
            deleteBtn.setBackground(null);
            deleteBtn.setPadding((int) (10 * d), 0, (int) (6 * d), 0);
            deleteBtn.setOnClickListener(v -> deleteCallback.onDelete(this));
            // [Modified] 添加缩放动画
            applyScaleAnimation(deleteBtn);

            titleBar.addView(expandIcon);
            titleBar.addView(titleView);
            titleBar.addView(deleteBtn);
            titleBar.setOnClickListener(v -> {
                if (!isAnimating) {
                    toggleFold(context);
                }
            });

            card.addView(titleBar);

            // 内容区域
            contentLayout = new LinearLayout(context);
            contentLayout.setOrientation(LinearLayout.VERTICAL);
            contentLayout.setPadding((int) (12 * d), (int) (12 * d), (int) (12 * d), (int) (12 * d));

            // 添加弹幕按钮
            Button addBulletBtn = new Button(context);
            addBulletBtn.setText("+ 添加弹幕");
            addBulletBtn.setTextColor(Color.WHITE);
            addBulletBtn.setTextSize(11);
            addBulletBtn.setTypeface(Utils.getFont(context), Typeface.BOLD);
            addBulletBtn.setBackground(createAccentButtonBg(d, 0xFF3B82F6));
            addBulletBtn.setLayoutParams(new LinearLayout.LayoutParams(-1, (int) (36 * d)));
            addBulletBtn.setOnClickListener(v -> addBulletEntry(context));
            // [Modified] 添加缩放动画
            applyScaleAnimation(addBulletBtn);
            contentLayout.addView(addBulletBtn);

            // 弹幕容器
            bulletsContainer = new LinearLayout(context);
            bulletsContainer.setOrientation(LinearLayout.VERTICAL);
            bulletsContainer.setPadding(0, (int) (8 * d), 0, 0);
            contentLayout.addView(bulletsContainer);

            card.addView(contentLayout);
            if (!expanded) {
                contentLayout.setVisibility(View.GONE);
                expandIcon.setText("▶");
            }
            this.rootView = card;
        }

        private void addBulletEntry(Context context) {
            BulletEntry entry = new BulletEntry(context, bulletEntries.size() + 1, d,
                    entryToDelete -> {
                        bulletEntries.remove(entryToDelete);
                        bulletsContainer.removeView(entryToDelete.rootView);
                        updateBulletTitles();
                    });
            bulletEntries.add(entry);
            bulletsContainer.addView(entry.rootView);
        }

        private void updateBulletTitles() {
            for (int i = 0; i < bulletEntries.size(); i++) {
                bulletEntries.get(i).updateTitle(i + 1);
            }
        }

        private void toggleFold(Context context) {
            if (isAnimating) return;
            isAnimating = true;

            final boolean willExpand = !expanded;
            final int targetHeight;

            if (willExpand) {
                contentLayout.setVisibility(View.VISIBLE);
                contentLayout.measure(
                        View.MeasureSpec.makeMeasureSpec(((View) rootView).getWidth(), View.MeasureSpec.AT_MOST),
                        View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED)
                );
                targetHeight = contentLayout.getMeasuredHeight();
            } else {
                targetHeight = 0;
            }

            int startHeight = contentLayout.getHeight();
            if (startHeight == 0 && contentLayout.getVisibility() == View.GONE) {
                startHeight = contentLayout.getMeasuredHeight();
            }

            ValueAnimator animator = ValueAnimator.ofInt(startHeight, targetHeight);
            animator.setDuration(ANIMATION_DURATION_MS);
            animator.setInterpolator(new DecelerateInterpolator());
            animator.addUpdateListener(animation -> {
                int value = (int) animation.getAnimatedValue();
                ViewGroup.LayoutParams params = contentLayout.getLayoutParams();
                params.height = value;
                contentLayout.setLayoutParams(params);
            });
            animator.addListener(new android.animation.AnimatorListenerAdapter() {
                @Override
                public void onAnimationEnd(android.animation.Animator animation) {
                    if (willExpand) {
                        ViewGroup.LayoutParams params = contentLayout.getLayoutParams();
                        params.height = ViewGroup.LayoutParams.WRAP_CONTENT;
                        contentLayout.setLayoutParams(params);
                        contentLayout.setVisibility(View.VISIBLE);
                        expandIcon.setText("▼");
                    } else {
                        contentLayout.setVisibility(View.GONE);
                        ViewGroup.LayoutParams params = contentLayout.getLayoutParams();
                        params.height = ViewGroup.LayoutParams.WRAP_CONTENT;
                        contentLayout.setLayoutParams(params);
                        expandIcon.setText("▶");
                    }
                    expanded = willExpand;
                    isAnimating = false;
                }
            });
            animator.start();
        }

        void updateTitle(int newIndex) {
            titleView.setText("武器 " + newIndex + " (ID: " + weaponId + ")");
        }

        /** 从JSON数组填充弹幕 */
        void populateFromJson(JSONArray array) throws JSONException {
            for (int i = 0; i < array.length(); i++) {
                JSONObject obj = array.getJSONObject(i);
                BulletEntry entry = new BulletEntry(rootView.getContext(), i + 1, this.d,
                        entryToDelete -> {
                            bulletEntries.remove(entryToDelete);
                            bulletsContainer.removeView(entryToDelete.rootView);
                            updateBulletTitles();
                        });
                entry.populateFromJson(obj);
                bulletEntries.add(entry);
                bulletsContainer.addView(entry.rootView);
            }
        }

        /** 返回该武器下的弹幕JSON数组 */
        JSONArray toJson() throws JSONException {
            JSONArray array = new JSONArray();
            for (BulletEntry bullet : bulletEntries) {
                array.put(bullet.toJson());
            }
            return array;
        }
    }

    // ========== 内部类：弹幕条目（无绑定武器） ==========
    private class BulletEntry {
        View rootView;
        LinearLayout contentLayout;
        TextView titleView;
        TextView expandIcon;
        boolean expanded = false;
        boolean isAnimating = false;

        EditText etId, etDamage, etFireSpeed, etKnockback;
        EditText etDirX, etDirY;
        CheckBox chkUseCursor;
        EditText etSpawnX, etSpawnY;
        CheckBox chkPlayerPos;
        EditText etAi1, etAi2, etAi3;

        interface DeleteCallback {
            void onDelete(BulletEntry entry);
        }

        @SuppressLint("SetTextI18n")
        BulletEntry(Context context, int index, float d, DeleteCallback deleteCallback) {
            LinearLayout card = new LinearLayout(context);
            card.setOrientation(LinearLayout.VERTICAL);
            LinearLayout.LayoutParams cardParams = new LinearLayout.LayoutParams(-1, -2);
            cardParams.bottomMargin = (int) (CARD_MARGIN_BOTTOM_DP * d);
            card.setLayoutParams(cardParams);

            GradientDrawable cardBg = new GradientDrawable();
            cardBg.setColor(0xFF1E1E1E);
            cardBg.setCornerRadius(8 * d);
            cardBg.setStroke((int) (1 * d), 0xFFAAAAAA);
            card.setBackground(cardBg);

            // 标题栏
            LinearLayout titleBar = new LinearLayout(context);
            titleBar.setOrientation(LinearLayout.HORIZONTAL);
            titleBar.setLayoutParams(new LinearLayout.LayoutParams(-1, -2));
            titleBar.setPadding((int) (10 * d),
                    (int) (TITLE_BAR_PADDING_VERTICAL_DP * d),
                    (int) (10 * d),
                    (int) (TITLE_BAR_PADDING_VERTICAL_DP * d));
            titleBar.setBackgroundColor(0xFF2A2A2A);
            titleBar.setClickable(true);

            expandIcon = new TextView(context);
            expandIcon.setText("▼");
            expandIcon.setTextColor(Menu.COLOR_TEXT_PRIMARY);
            expandIcon.setTextSize(11);
            expandIcon.setTypeface(Typeface.DEFAULT_BOLD);
            expandIcon.setPadding(0, 0, (int) (6 * d), 0);

            titleView = new TextView(context);
            titleView.setText("弹幕 " + index);
            titleView.setTextColor(Menu.COLOR_TEXT_PRIMARY);
            titleView.setTextSize(11);
            titleView.setTypeface(Utils.getFont(context), Typeface.BOLD);
            titleView.setLayoutParams(new LinearLayout.LayoutParams(0, -2, 1));

            Button deleteBtn = new Button(context);
            deleteBtn.setText("✕");
            deleteBtn.setTextColor(0xFFEF4444);
            deleteBtn.setTextSize(12);
            deleteBtn.setTypeface(Typeface.DEFAULT_BOLD);
            deleteBtn.setBackground(null);
            deleteBtn.setPadding((int) (8 * d), 0, (int) (4 * d), 0);
            deleteBtn.setOnClickListener(v -> deleteCallback.onDelete(this));
            // [Modified] 添加缩放动画
            applyScaleAnimation(deleteBtn);

            titleBar.addView(expandIcon);
            titleBar.addView(titleView);
            titleBar.addView(deleteBtn);
            titleBar.setOnClickListener(v -> {
                if (!isAnimating) {
                    toggleFold(context);
                }
            });

            card.addView(titleBar);

            // 内容区域
            contentLayout = new LinearLayout(context);
            contentLayout.setOrientation(LinearLayout.VERTICAL);
            contentLayout.setPadding((int) (10 * d), (int) (10 * d), (int) (10 * d), (int) (10 * d));

            // 弹幕ID
            contentLayout.addView(createLabeledEditText(context, "弹幕ID", d, et -> etId = et, InputType.TYPE_CLASS_NUMBER, "132"));
            addVerticalSpacer(contentLayout, d);
            // 伤害
            contentLayout.addView(createLabeledEditText(context, "伤害", d, et -> etDamage = et, InputType.TYPE_CLASS_NUMBER, "100"));
            addVerticalSpacer(contentLayout, d);
            // 发射速度
            contentLayout.addView(createLabeledEditText(context, "发射速度", d, et -> etFireSpeed = et, InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_DECIMAL, "20"));
            addVerticalSpacer(contentLayout, d);
            // 击退
            contentLayout.addView(createLabeledEditText(context, "击退", d, et -> etKnockback = et, InputType.TYPE_CLASS_NUMBER, "10"));
            addVerticalSpacer(contentLayout, d);

            // 方向行
            LinearLayout dirRow = new LinearLayout(context);
            dirRow.setOrientation(LinearLayout.HORIZONTAL);
            dirRow.setGravity(Gravity.CENTER_VERTICAL);
            LinearLayout dirInputGroup = new LinearLayout(context);
            dirInputGroup.setOrientation(LinearLayout.VERTICAL);
            dirInputGroup.setLayoutParams(new LinearLayout.LayoutParams(0, -2, 2));
            TextView dirLabel = new TextView(context);
            dirLabel.setText("方向");
            dirLabel.setTextColor(Menu.COLOR_TEXT_SECONDARY);
            dirLabel.setTextSize(9);
            dirLabel.setTypeface(Utils.getFont(context));
            dirLabel.setPadding(0, 0, 0, (int) (4 * d));
            dirInputGroup.addView(dirLabel);
            LinearLayout dirInputRow = new LinearLayout(context);
            dirInputRow.setOrientation(LinearLayout.HORIZONTAL);
            etDirX = createNumberEditText(context, d, "X", "0");
            etDirY = createNumberEditText(context, d, "Y", "0");
            dirInputRow.addView(etDirX);
            dirInputRow.addView(createSpacer(context, d, 6));
            dirInputRow.addView(etDirY);
            dirInputGroup.addView(dirInputRow);
            dirRow.addView(dirInputGroup);
            dirRow.addView(createSpacer(context, d, 12));
            chkUseCursor = new CheckBox(context);
            chkUseCursor.setText("使用光标位置");
            chkUseCursor.setTextColor(Menu.COLOR_TEXT_PRIMARY);
            chkUseCursor.setTextSize(10);
            chkUseCursor.setTypeface(Utils.getFont(context));
            chkUseCursor.setLayoutParams(new LinearLayout.LayoutParams(0, -2, 1));
            chkUseCursor.setGravity(Gravity.CENTER_VERTICAL);
            chkUseCursor.setChecked(true);
            dirRow.addView(chkUseCursor);
            contentLayout.addView(dirRow);
            addVerticalSpacer(contentLayout, d);

            // 发射位置行
            LinearLayout spawnRow = new LinearLayout(context);
            spawnRow.setOrientation(LinearLayout.HORIZONTAL);
            spawnRow.setGravity(Gravity.CENTER_VERTICAL);
            LinearLayout spawnInputGroup = new LinearLayout(context);
            spawnInputGroup.setOrientation(LinearLayout.VERTICAL);
            spawnInputGroup.setLayoutParams(new LinearLayout.LayoutParams(0, -2, 2));
            TextView spawnLabel = new TextView(context);
            spawnLabel.setText("发射位置");
            spawnLabel.setTextColor(Menu.COLOR_TEXT_SECONDARY);
            spawnLabel.setTextSize(9);
            spawnLabel.setTypeface(Utils.getFont(context));
            spawnLabel.setPadding(0, 0, 0, (int) (4 * d));
            spawnInputGroup.addView(spawnLabel);
            LinearLayout spawnInputRow = new LinearLayout(context);
            spawnInputRow.setOrientation(LinearLayout.HORIZONTAL);
            etSpawnX = createNumberEditText(context, d, "X", "0");
            etSpawnY = createNumberEditText(context, d, "Y", "0");
            spawnInputRow.addView(etSpawnX);
            spawnInputRow.addView(createSpacer(context, d, 6));
            spawnInputRow.addView(etSpawnY);
            spawnInputGroup.addView(spawnInputRow);
            spawnRow.addView(spawnInputGroup);
            spawnRow.addView(createSpacer(context, d, 12));
            chkPlayerPos = new CheckBox(context);
            chkPlayerPos.setText("玩家位置");
            chkPlayerPos.setTextColor(Menu.COLOR_TEXT_PRIMARY);
            chkPlayerPos.setTextSize(10);
            chkPlayerPos.setTypeface(Utils.getFont(context));
            chkPlayerPos.setLayoutParams(new LinearLayout.LayoutParams(0, -2, 1));
            chkPlayerPos.setGravity(Gravity.CENTER_VERTICAL);
            chkPlayerPos.setChecked(true);
            spawnRow.addView(chkPlayerPos);
            contentLayout.addView(spawnRow);
            addVerticalSpacer(contentLayout, d);

            // AI 一行
            LinearLayout aiRow = new LinearLayout(context);
            aiRow.setOrientation(LinearLayout.HORIZONTAL);
            LinearLayout ai1Wrapper = createMiniLabeledEditText(context, "AI1", d, et -> etAi1 = et, InputType.TYPE_CLASS_NUMBER, "0", 1);
            LinearLayout ai2Wrapper = createMiniLabeledEditText(context, "AI2", d, et -> etAi2 = et, InputType.TYPE_CLASS_NUMBER, "0", 1);
            LinearLayout ai3Wrapper = createMiniLabeledEditText(context, "AI3", d, et -> etAi3 = et, InputType.TYPE_CLASS_NUMBER, "0", 1);
            aiRow.addView(ai1Wrapper);
            aiRow.addView(createSpacer(context, d, 6));
            aiRow.addView(ai2Wrapper);
            aiRow.addView(createSpacer(context, d, 6));
            aiRow.addView(ai3Wrapper);
            contentLayout.addView(aiRow);

            card.addView(contentLayout);
            if (!expanded) {
                contentLayout.setVisibility(View.GONE);
                expandIcon.setText("▶");
            }
            this.rootView = card;
        }

        private void toggleFold(Context context) {
            if (isAnimating) return;
            isAnimating = true;

            final boolean willExpand = !expanded;
            final int targetHeight;

            if (willExpand) {
                contentLayout.setVisibility(View.VISIBLE);
                contentLayout.measure(
                        View.MeasureSpec.makeMeasureSpec(((View) rootView).getWidth(), View.MeasureSpec.AT_MOST),
                        View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED)
                );
                targetHeight = contentLayout.getMeasuredHeight();
            } else {
                targetHeight = 0;
            }

            int startHeight = contentLayout.getHeight();
            if (startHeight == 0 && contentLayout.getVisibility() == View.GONE) {
                startHeight = contentLayout.getMeasuredHeight();
            }

            ValueAnimator animator = ValueAnimator.ofInt(startHeight, targetHeight);
            animator.setDuration(ANIMATION_DURATION_MS);
            animator.setInterpolator(new DecelerateInterpolator());
            animator.addUpdateListener(animation -> {
                int value = (int) animation.getAnimatedValue();
                ViewGroup.LayoutParams params = contentLayout.getLayoutParams();
                params.height = value;
                contentLayout.setLayoutParams(params);
            });
            animator.addListener(new android.animation.AnimatorListenerAdapter() {
                @Override
                public void onAnimationEnd(android.animation.Animator animation) {
                    if (willExpand) {
                        ViewGroup.LayoutParams params = contentLayout.getLayoutParams();
                        params.height = ViewGroup.LayoutParams.WRAP_CONTENT;
                        contentLayout.setLayoutParams(params);
                        contentLayout.setVisibility(View.VISIBLE);
                        expandIcon.setText("▼");
                    } else {
                        contentLayout.setVisibility(View.GONE);
                        ViewGroup.LayoutParams params = contentLayout.getLayoutParams();
                        params.height = ViewGroup.LayoutParams.WRAP_CONTENT;
                        contentLayout.setLayoutParams(params);
                        expandIcon.setText("▶");
                    }
                    expanded = willExpand;
                    isAnimating = false;
                }
            });
            animator.start();
        }

        private void addVerticalSpacer(LinearLayout parent, float d) {
            View spacer = new View(parent.getContext());
            spacer.setLayoutParams(new LinearLayout.LayoutParams(-1, (int) (CONTENT_VERTICAL_SPACING_DP * d)));
            parent.addView(spacer);
        }

        void populateFromJson(JSONObject obj) throws JSONException {
            etId.setText(obj.optString("id", "0"));
            etDamage.setText(obj.optString("damage", "0"));
            etFireSpeed.setText(obj.optString("fireSpeed", "0"));
            etKnockback.setText(obj.optString("knockback", "0"));

            JSONObject dir = obj.optJSONObject("direction");
            if (dir != null) {
                etDirX.setText(dir.optString("x", "0"));
                etDirY.setText(dir.optString("y", "0"));
            }
            chkUseCursor.setChecked(obj.optBoolean("useCursorPosition", false));

            JSONObject spawn = obj.optJSONObject("spawnPosition");
            if (spawn != null) {
                etSpawnX.setText(spawn.optString("x", "0"));
                etSpawnY.setText(spawn.optString("y", "0"));
            }
            chkPlayerPos.setChecked(obj.optBoolean("usePlayerPosition", false));

            JSONObject ai = obj.optJSONObject("ai");
            if (ai != null) {
                etAi1.setText(ai.optString("ai1", "0"));
                etAi2.setText(ai.optString("ai2", "0"));
                etAi3.setText(ai.optString("ai3", "0"));
            }
        }

        void updateTitle(int newIndex) {
            titleView.setText("弹幕 " + newIndex);
        }

        JSONObject toJson() throws JSONException {
            JSONObject obj = new JSONObject();
            obj.put("id", Integer.parseInt(getText(etId, "0")));
            obj.put("damage", Integer.parseInt(getText(etDamage, "0")));
            obj.put("fireSpeed", Float.parseFloat(getText(etFireSpeed, "0")));
            obj.put("knockback", Integer.parseInt(getText(etKnockback, "0")));

            JSONObject dir = new JSONObject();
            dir.put("x", Float.parseFloat(getText(etDirX, "0")));
            dir.put("y", Float.parseFloat(getText(etDirY, "0")));
            obj.put("direction", dir);
            obj.put("useCursorPosition", chkUseCursor.isChecked());

            JSONObject spawn = new JSONObject();
            spawn.put("x", Float.parseFloat(getText(etSpawnX, "0")));
            spawn.put("y", Float.parseFloat(getText(etSpawnY, "0")));
            obj.put("spawnPosition", spawn);
            obj.put("usePlayerPosition", chkPlayerPos.isChecked());

            JSONObject ai = new JSONObject();
            ai.put("ai1", Integer.parseInt(getText(etAi1, "0")));
            ai.put("ai2", Integer.parseInt(getText(etAi2, "0")));
            ai.put("ai3", Integer.parseInt(getText(etAi3, "0")));
            obj.put("ai", ai);
            return obj;
        }

        private String getText(EditText et, String def) {
            String s = et.getText().toString().trim();
            return s.isEmpty() ? def : s;
        }
    }

    // ========== UI 辅助方法 ==========

    private static LinearLayout createLabeledEditText(Context context, String label, float d,
                                                      java.util.function.Consumer<EditText> setter, int inputType, String defaultValue) {
        LinearLayout wrapper = new LinearLayout(context);
        wrapper.setOrientation(LinearLayout.VERTICAL);
        wrapper.setLayoutParams(new LinearLayout.LayoutParams(-1, -2));

        TextView labelView = new TextView(context);
        labelView.setText(label);
        labelView.setTextColor(Menu.COLOR_TEXT_SECONDARY);
        labelView.setTextSize(9);
        labelView.setTypeface(Utils.getFont(context));
        labelView.setPadding(0, 0, 0, (int) (4 * d));
        wrapper.addView(labelView);

        EditText et = new EditText(context);
        et.setText(defaultValue);
        et.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        et.setHintTextColor(0xFF666666);
        et.setTextSize(11);
        et.setTypeface(Typeface.MONOSPACE);
        et.setInputType(inputType);
        et.setBackground(createInputBg(d, context));
        et.setPadding((int) (10 * d), (int) (8 * d), (int) (10 * d), (int) (8 * d));
        et.setLayoutParams(new LinearLayout.LayoutParams(-1, (int) (40 * d)));
        et.setFocusable(true);
        et.setFocusableInTouchMode(true);
        wrapper.addView(et);

        setter.accept(et);
        return wrapper;
    }

    private static LinearLayout createMiniLabeledEditText(Context context, String label, float d,
                                                          java.util.function.Consumer<EditText> setter, int inputType, String defaultValue, float weight) {
        LinearLayout wrapper = new LinearLayout(context);
        wrapper.setOrientation(LinearLayout.VERTICAL);
        wrapper.setLayoutParams(new LinearLayout.LayoutParams(0, -2, weight));

        TextView labelView = new TextView(context);
        labelView.setText(label);
        labelView.setTextColor(Menu.COLOR_TEXT_SECONDARY);
        labelView.setTextSize(8);
        labelView.setTypeface(Utils.getFont(context));
        labelView.setPadding(0, 0, 0, (int) (2 * d));
        wrapper.addView(labelView);

        EditText et = new EditText(context);
        et.setText(defaultValue);
        et.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        et.setHintTextColor(0xFF666666);
        et.setTextSize(10);
        et.setTypeface(Typeface.MONOSPACE);
        et.setInputType(inputType);
        et.setBackground(createInputBg(d, context));
        et.setPadding((int) (6 * d), (int) (4 * d), (int) (6 * d), (int) (4 * d));
        et.setLayoutParams(new LinearLayout.LayoutParams(-1, (int) (32 * d)));
        et.setFocusable(true);
        et.setFocusableInTouchMode(true);
        wrapper.addView(et);

        setter.accept(et);
        return wrapper;
    }

    private static EditText createNumberEditText(Context context, float d, String hint, String defaultValue) {
        EditText et = new EditText(context);
        et.setHint(hint);
        et.setText(defaultValue);
        et.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        et.setHintTextColor(0xFF666666);
        et.setTextSize(11);
        et.setTypeface(Typeface.MONOSPACE);
        et.setInputType(InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_DECIMAL);
        et.setBackground(createInputBg(d, context));
        et.setPadding((int) (8 * d), (int) (6 * d), (int) (8 * d), (int) (6 * d));
        et.setLayoutParams(new LinearLayout.LayoutParams(0, (int) (36 * d), 1));
        et.setFocusable(true);
        et.setFocusableInTouchMode(true);
        return et;
    }

    private static GradientDrawable createInputBg(float d, Context context) {
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(0xFF0F0F0F);
        bg.setCornerRadius(8 * d);
        bg.setStroke((int) (1 * d), 0xFFAAAAAA);
        return bg;
    }

    private static GradientDrawable createAccentButtonBg(float d, int color) {
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(color);
        bg.setCornerRadius(6 * d);
        return bg;
    }

    private Button createLeftButton(String text, int color, float d, View.OnClickListener listener) {
        Button btn = new Button(context);
        btn.setText(text);
        btn.setTextColor(Color.WHITE);
        btn.setTextSize(14);
        btn.setTypeface(Utils.getFont(context), Typeface.BOLD);
        btn.setAllCaps(false);
        btn.setGravity(Gravity.CENTER);
        btn.setPadding((int) (10 * d), (int) (14 * d), (int) (10 * d), (int) (14 * d));
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(color);
        bg.setCornerRadius(10 * d);
        btn.setBackground(bg);
        btn.setLayoutParams(new LinearLayout.LayoutParams(-1, -2));
        btn.setOnClickListener(v -> safeExecute(() -> listener.onClick(v)));
        // [Modified] 添加缩放动画
        applyScaleAnimation(btn);
        return btn;
    }

    private TextView createTitle(String text, float d) {
        TextView tv = new TextView(context);
        tv.setText(text);
        tv.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        tv.setTextSize(16);
        tv.setTypeface(Utils.getFont(context), Typeface.BOLD);
        tv.setPadding(0, 0, 0, (int) (6 * d));
        return tv;
    }

    private View createDivider(float d) {
        View v = new View(context);
        v.setLayoutParams(new LinearLayout.LayoutParams(-1, (int) (1 * d)));
        v.setBackgroundColor(Menu.COLOR_DIVIDER);
        return v;
    }

    private static View createSpacer(Context context, float d, int widthDp) {
        View v = new View(context);
        v.setLayoutParams(new LinearLayout.LayoutParams((int) (widthDp * d), 0));
        return v;
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
        weaponEntries.clear();
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