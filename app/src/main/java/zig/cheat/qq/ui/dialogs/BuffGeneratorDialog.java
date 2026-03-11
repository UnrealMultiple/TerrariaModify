package zig.cheat.qq.ui.dialogs;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.os.Handler;
import android.os.Looper;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.util.LruCache;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

import zig.cheat.qq.ui.Menu;
import zig.cheat.qq.ui.Utils;

public class BuffGeneratorDialog {
    private static final int GRID_COLUMNS = 8;
    private static final int MAX_BITMAP_CACHE_SIZE = 50;
    private static final int TARGET_IMAGE_SIZE_DP = 60;
    private static final int DIALOG_WIDTH_DP = 900;
    private static final int LEFT_PANEL_WIDTH_DP = 250;
    private static final int ACTION_BAR_HEIGHT_DP = 40;
    private static final long SEARCH_DEBOUNCE_MS = 300;

    private final Context context;
    private final Handler handler;
    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private final AtomicBoolean isDestroyed = new AtomicBoolean(false);

    private Dialog dialog;
    private GridView gridView;
    private BuffGridAdapter gridAdapter;
    private TextView statusText;
    private Button generateButton;
    private EditText countEditText;
    private EditText searchEditText;
    private Button clearSearchButton;
    private TextView selectedCountText;

    private List<BuffInfo> allBuffs = new ArrayList<>();
    private List<BuffInfo> filteredBuffs = new ArrayList<>();
    private final Map<Integer, Boolean> selectedStates = new HashMap<>();

    private final LruCache<String, Bitmap> bitmapCache;
    private final Runnable searchRunnable = this::performSearchNow;

    public interface OnBuffsGenerateListener {
        void onBuffsGenerate(List<SelectedBuff> selectedBuffs, int count);
    }

    public static class SelectedBuff {
        public String name;
        public int id;
        public int time;
        public String displayName;
        public boolean isSelected;

        public SelectedBuff(String name, int id, int time, String displayName, boolean isSelected) {
            this.name = name;
            this.id = id;
            this.time = time;
            this.displayName = displayName;
            this.isSelected = isSelected;
        }
    }

    private OnBuffsGenerateListener generateListener;

    public void setOnGenerateListener(OnBuffsGenerateListener listener) {
        this.generateListener = listener;
    }

    private static class BuffInfo {
        String name;
        int id;
        String fileName;
        String displayName;

        BuffInfo(String name, int id, String fileName) {
            this.name = name;
            this.id = id;
            this.fileName = fileName;
            this.displayName = formatDisplayName(name);
        }

        private String formatDisplayName(String rawName) {
            String formatted = rawName.replace('_', ' ');
            if (!formatted.isEmpty()) {
                formatted = Character.toUpperCase(formatted.charAt(0)) + formatted.substring(1);
            }
            return formatted;
        }
    }

    public BuffGeneratorDialog(Context ctx) {
        this.context = ctx;
        this.handler = new Handler(Looper.getMainLooper());
        bitmapCache = new LruCache<String, Bitmap>(MAX_BITMAP_CACHE_SIZE) {
            @Override
            protected int sizeOf(String key, Bitmap bitmap) {
                return bitmap.getByteCount() / 1024;
            }
        };
    }

    public void show() {
        if (context == null || !(context instanceof Activity) || ((Activity) context).isFinishing()) {
            return;
        }
        if (dialog != null && dialog.isShowing()) dialog.dismiss();
        try {
            createDialog();
        } catch (Exception e) {
            safeToast("Dialog error");
        }
    }

    @SuppressLint({"SetTextI18n", "ClickableViewAccessibility"})
    private void createDialog() {
        dialog = new Dialog(context, android.R.style.Theme_Translucent_NoTitleBar);
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        dialog.setCancelable(true);
        dialog.setOnDismissListener(d -> cleanup());

        final float d = context.getResources().getDisplayMetrics().density;
        // 计算自适应宽度：不超过屏幕宽度减去左右边距（各20dp）
        int screenWidth = context.getResources().getDisplayMetrics().widthPixels;
        int maxWidth = screenWidth - (int)(40 * d);
        final int dialogWidth = Math.min((int)(DIALOG_WIDTH_DP * d), maxWidth);

        final int leftPanelWidth = (int) (LEFT_PANEL_WIDTH_DP * d);
        final int actionBarHeight = (int) (ACTION_BAR_HEIGHT_DP * d);

        LinearLayout root = new LinearLayout(context);
        root.setOrientation(LinearLayout.HORIZONTAL);
        root.setPadding((int) (20 * d), (int) (20 * d), (int) (20 * d), (int) (20 * d));

        GradientDrawable bg = new GradientDrawable();
        bg.setColor(Menu.COLOR_WINDOW_BG);
        bg.setCornerRadius(16 * d);
        bg.setStroke((int) (1 * d), Menu.COLOR_BORDER);
        root.setBackground(bg);

        // ========== 左侧面板 ==========
        LinearLayout leftPanel = new LinearLayout(context);
        leftPanel.setOrientation(LinearLayout.VERTICAL);
        leftPanel.setLayoutParams(new LinearLayout.LayoutParams(leftPanelWidth, -1));
        leftPanel.setPadding(0, 0, (int) (20 * d), 0);

        // 标题
        TextView title = new TextView(context);
        title.setText("BUFF GENERATOR");
        title.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        title.setTextSize(20);
        title.setTypeface(Utils.getFont(context));
        title.setLetterSpacing(0.1f);
        title.setGravity(Gravity.CENTER);
        title.setPadding(0, 0, 0, (int) (20 * d));
        leftPanel.addView(title);

        // 搜索框
        LinearLayout searchRow = new LinearLayout(context);
        searchRow.setOrientation(LinearLayout.HORIZONTAL);
        searchRow.setGravity(Gravity.CENTER_VERTICAL);
        searchRow.setPadding(0, 0, 0, (int) (12 * d));

        searchEditText = new EditText(context);
        searchEditText.setHint("搜索名称或ID...");
        searchEditText.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        searchEditText.setHintTextColor(Menu.COLOR_TEXT_SECONDARY);
        searchEditText.setTextSize(13);
        searchEditText.setTypeface(Utils.getFont(context));
        searchEditText.setBackground(createInputBg(d));
        searchEditText.setPadding((int) (10 * d), (int) (6 * d), (int) (10 * d), (int) (6 * d));
        searchEditText.setSingleLine(true);

        searchEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                handler.removeCallbacks(searchRunnable);
                handler.postDelayed(searchRunnable, SEARCH_DEBOUNCE_MS);
            }

            @Override
            public void afterTextChanged(Editable s) {}
        });

        LinearLayout.LayoutParams searchLp = new LinearLayout.LayoutParams(0, actionBarHeight, 1);
        searchLp.rightMargin = (int) (8 * d);
        searchEditText.setLayoutParams(searchLp);
        searchRow.addView(searchEditText);

        clearSearchButton = new Button(context);
        clearSearchButton.setText("✕");
        clearSearchButton.setTextColor(Color.WHITE);
        clearSearchButton.setTextSize(14);
        clearSearchButton.setTypeface(Utils.getFont(context));
        clearSearchButton.setAllCaps(false);
        clearSearchButton.setPadding(0, 0, 0, 0);
        GradientDrawable clearBg = new GradientDrawable();
        clearBg.setColor(0xFF6B7280);
        clearBg.setCornerRadius(8 * d);
        clearSearchButton.setBackground(clearBg);
        clearSearchButton.setOnTouchListener((v, event) -> {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    v.animate().scaleX(0.95f).scaleY(0.95f).setDuration(100).start();
                    break;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    v.animate().scaleX(1.0f).scaleY(1.0f).setDuration(100).start();
                    break;
            }
            return false;
        });
        clearSearchButton.setOnClickListener(v -> {
            searchEditText.setText("");
            performSearchNow();
        });

        LinearLayout.LayoutParams clearLp = new LinearLayout.LayoutParams(actionBarHeight, actionBarHeight);
        clearSearchButton.setLayoutParams(clearLp);
        searchRow.addView(clearSearchButton);

        leftPanel.addView(searchRow);

        // 数量输入
        LinearLayout countWrapper = new LinearLayout(context);
        countWrapper.setOrientation(LinearLayout.HORIZONTAL);
        countWrapper.setBackground(createInputBg(d));
        countWrapper.setGravity(Gravity.CENTER_VERTICAL);
        countWrapper.setPadding((int) (8 * d), 0, (int) (8 * d), 0);
        countWrapper.setLayoutParams(new LinearLayout.LayoutParams(-1, actionBarHeight));

        TextView countLabel = new TextView(context);
        countLabel.setText("时长");
        countLabel.setTextColor(Menu.COLOR_TEXT_SECONDARY);
        countLabel.setTextSize(13);
        countLabel.setTypeface(Utils.getFont(context));
        countLabel.setPadding(0, 0, (int) (4 * d), 0);
        countWrapper.addView(countLabel);

        countEditText = new EditText(context);
        countEditText.setHint("60");
        countEditText.setText("60");
        countEditText.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        countEditText.setHintTextColor(Menu.COLOR_TEXT_SECONDARY);
        countEditText.setTextSize(14);
        countEditText.setTypeface(Typeface.MONOSPACE);
        countEditText.setInputType(InputType.TYPE_CLASS_NUMBER);
        countEditText.setBackground(null);
        countEditText.setGravity(Gravity.CENTER);
        countEditText.setPadding(0, 0, 0, 0);
        countEditText.setLayoutParams(new LinearLayout.LayoutParams(0, -1, 1));
        countWrapper.addView(countEditText);

        leftPanel.addView(countWrapper);

        // 生成按钮
        generateButton = new Button(context);
        generateButton.setText("生成BUFF");
        generateButton.setTextColor(Color.WHITE);
        generateButton.setTextSize(15);
        generateButton.setTypeface(Utils.getFont(context));
        generateButton.setAllCaps(false);
        generateButton.setPadding(0, 0, 0, 0);
        GradientDrawable generateBg = new GradientDrawable();
        generateBg.setColor(Menu.COLOR_ACCENT_BLUE);
        generateBg.setCornerRadius(8 * d);
        generateButton.setBackground(generateBg);
        generateButton.setOnTouchListener((v, event) -> {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    v.animate().scaleX(0.95f).scaleY(0.95f).setDuration(100).start();
                    break;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    v.animate().scaleX(1.0f).scaleY(1.0f).setDuration(100).start();
                    break;
            }
            return false;
        });
        generateButton.setOnClickListener(v -> generateSelectedBuffs());

        LinearLayout.LayoutParams generateLp = new LinearLayout.LayoutParams(-1, (int) (44 * d));
        generateLp.topMargin = (int) (12 * d);
        generateLp.bottomMargin = (int) (12 * d);
        generateButton.setLayoutParams(generateLp);
        leftPanel.addView(generateButton);

        // 全选/取消全选按钮
        LinearLayout buttonRow = new LinearLayout(context);
        buttonRow.setOrientation(LinearLayout.HORIZONTAL);
        buttonRow.setPadding(0, 0, 0, (int) (12 * d));

        Button selectAllButton = Utils.createActionButton(context, "全选", d);
        selectAllButton.setOnClickListener(v -> selectAll(true));

        Button deselectAllButton = Utils.createActionButton(context, "取消选择", d);
        deselectAllButton.setOnClickListener(v -> selectAll(false));

        LinearLayout.LayoutParams buttonLp = new LinearLayout.LayoutParams(0, actionBarHeight, 1);
        buttonLp.rightMargin = (int) (8 * d);
        selectAllButton.setLayoutParams(buttonLp);
        deselectAllButton.setLayoutParams(buttonLp);

        buttonRow.addView(selectAllButton);
        buttonRow.addView(deselectAllButton);
        leftPanel.addView(buttonRow);

        // 选中数量显示
        selectedCountText = new TextView(context);
        selectedCountText.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        selectedCountText.setTextSize(14);
        selectedCountText.setTypeface(Utils.getFont(context));
        selectedCountText.setPadding((int) (12 * d), (int) (8 * d), (int) (12 * d), (int) (8 * d));
        selectedCountText.setGravity(Gravity.CENTER);
        GradientDrawable selectedBg = new GradientDrawable();
        selectedBg.setColor(Menu.COLOR_ACCENT_BLUE);
        selectedBg.setCornerRadius(20 * d);
        selectedCountText.setBackground(selectedBg);
        selectedCountText.setText("已选 0 个");
        selectedCountText.setLayoutParams(new LinearLayout.LayoutParams(-1, -2));
        leftPanel.addView(selectedCountText);

        // 状态文本
        statusText = new TextView(context);
        statusText.setTextColor(Menu.COLOR_TEXT_SECONDARY);
        statusText.setTextSize(12);
        statusText.setTypeface(Utils.getFont(context));
        statusText.setPadding(0, (int) (12 * d), 0, 0);
        statusText.setGravity(Gravity.CENTER);
        leftPanel.addView(statusText);

        root.addView(leftPanel);

        // ========== 右侧面板（GridView） ==========
        FrameLayout rightPanel = new FrameLayout(context);
        rightPanel.setLayoutParams(new LinearLayout.LayoutParams(0, -1, 1));

        gridView = new GridView(context);
        gridView.setNumColumns(GRID_COLUMNS);
        gridView.setVerticalSpacing((int) (12 * d));
        gridView.setHorizontalSpacing((int) (12 * d));
        gridView.setStretchMode(GridView.STRETCH_COLUMN_WIDTH);
        gridView.setPadding(0, 0, 0, 0);

        FrameLayout.LayoutParams gridLp = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT);
        gridView.setLayoutParams(gridLp);
        rightPanel.addView(gridView);

        root.addView(rightPanel);

        dialog.setContentView(root);

        Window window = dialog.getWindow();
        if (window != null) {
            window.setLayout(dialogWidth, WindowManager.LayoutParams.WRAP_CONTENT);
            window.setBackgroundDrawableResource(android.R.color.transparent);
        }

        dialog.show();
        scanAssets();
        updateSelectedCount();
    }

    private GradientDrawable createInputBg(float d) {
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(Menu.COLOR_CARD_BG);
        bg.setCornerRadius(8 * d);
        bg.setStroke((int) (1 * d), Menu.COLOR_BORDER);
        return bg;
    }


    private void selectAll(boolean select) {
        for (int i = 0; i < filteredBuffs.size(); i++) {
            selectedStates.put(i, select);
        }
        updateSelectedCount();
        if (gridAdapter != null) {
            gridAdapter.notifyDataSetChanged();
        }
    }

    private void updateSelectedCount() {
        int count = 0;
        for (Boolean selected : selectedStates.values()) {
            if (selected != null && selected) count++;
        }
        if (selectedCountText != null) {
            selectedCountText.setText("已选 " + count + " 个");
        }
    }

    private void performSearchNow() {
        if (isDestroyed.get()) return;
        String query = searchEditText.getText().toString().trim().toLowerCase();
        if (query.isEmpty()) {
            filteredBuffs = new ArrayList<>(allBuffs);
        } else {
            filteredBuffs = new ArrayList<>();
            for (BuffInfo buff : allBuffs) {
                if (buff.displayName.toLowerCase().contains(query) ||
                        buff.name.toLowerCase().contains(query) ||
                        String.valueOf(buff.id).contains(query)) {
                    filteredBuffs.add(buff);
                }
            }
        }

        // 清空选中状态
        selectedStates.clear();
        updateSelectedCount();

        if (gridAdapter != null) {
            gridAdapter.notifyDataSetChanged();
        }
        safeSetStatus("共搜索到 " + filteredBuffs.size() + " 个BUFF");
    }

    private void generateSelectedBuffs() {
        if (generateListener == null) {
            safeToast("未设置生成监听器");
            return;
        }

        // 获取时长
        int duration = 60;
        try {
            String durationStr = countEditText.getText().toString().trim();
            if (!durationStr.isEmpty()) {
                duration = Integer.parseInt(durationStr);
                if (duration <= 0) duration = 1;
            }
        } catch (NumberFormatException ignored) {}

        List<SelectedBuff> selectedList = new ArrayList<>();
        for (int i = 0; i < filteredBuffs.size(); i++) {
            Boolean selected = selectedStates.get(i);
            if (selected != null && selected) {
                BuffInfo buff = filteredBuffs.get(i);
                selectedList.add(new SelectedBuff(buff.name, buff.id, duration, buff.displayName, true));
            }
        }

        if (selectedList.isEmpty()) {
            safeToast("请先选择要生成的BUFF");
            return;
        }

        generateListener.onBuffsGenerate(selectedList, duration);
        safeToast("已生成 " + selectedList.size() + " 个BUFF");
    }

    private void scanAssets() {
        executor.execute(() -> {
            if (isDestroyed.get()) return;

            List<BuffInfo> buffs = new ArrayList<>();
            AssetManager am = context.getAssets();
            try {
                String[] files = am.list("buffs");
                if (files != null) {
                    for (String file : files) {
                        if (file.toLowerCase().endsWith(".png")) {
                            String base = file.substring(0, file.length() - 4);
                            int lastUnderscore = base.lastIndexOf('_');
                            if (lastUnderscore > 0 && lastUnderscore < base.length() - 1) {
                                String name = base.substring(0, lastUnderscore);
                                String idStr = base.substring(lastUnderscore + 1);
                                try {
                                    int id = Integer.parseInt(idStr);
                                    buffs.add(new BuffInfo(name, id, file));
                                } catch (NumberFormatException ignored) {}
                            }
                        }
                    }
                }
            } catch (IOException e) {
                handler.post(() -> safeSetStatus("读取buff文件夹失败"));
                return;
            }

            buffs.sort((a, b) -> Integer.compare(a.id, b.id));

            handler.post(() -> {
                if (isDestroyed.get()) return;
                allBuffs = buffs;
                filteredBuffs = new ArrayList<>(allBuffs);
                selectedStates.clear();
                gridAdapter = new BuffGridAdapter();
                gridView.setAdapter(gridAdapter);
                safeSetStatus("共 " + allBuffs.size() + " 个BUFF");
                updateSelectedCount();
            });
        });
    }

    private class BuffGridAdapter extends BaseAdapter {
        @Override
        public int getCount() {
            return filteredBuffs.size();
        }

        @Override
        public BuffInfo getItem(int position) {
            return filteredBuffs.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @SuppressLint("SetTextI18n")
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            if (convertView == null) {
                // 使用FrameLayout作为根布局
                FrameLayout frameLayout = new FrameLayout(context);
                frameLayout.setPadding(0, 0, 0, 0);

                // 垂直布局放置图片和文字
                LinearLayout contentLayout = new LinearLayout(context);
                contentLayout.setOrientation(LinearLayout.VERTICAL);
                contentLayout.setGravity(Gravity.CENTER_HORIZONTAL);

                FrameLayout.LayoutParams contentLp = new FrameLayout.LayoutParams(
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT);
                contentLp.gravity = Gravity.CENTER;
                contentLayout.setLayoutParams(contentLp);

                // 图片容器
                FrameLayout imageContainer = new FrameLayout(context);
                int size = (int) (TARGET_IMAGE_SIZE_DP * d());
                LinearLayout.LayoutParams containerLp = new LinearLayout.LayoutParams(size, size);
                containerLp.bottomMargin = (int) (4 * d());
                imageContainer.setLayoutParams(containerLp);

                // 图片
                ImageView imageView = new ImageView(context);
                FrameLayout.LayoutParams imgLp = new FrameLayout.LayoutParams(size, size);
                imageView.setLayoutParams(imgLp);

                int padding = (int) (6 * d());
                imageView.setPadding(padding, padding, padding, padding);
                imageView.setScaleType(ImageView.ScaleType.FIT_CENTER);

                GradientDrawable imageBg = new GradientDrawable();
                imageBg.setColor(Menu.COLOR_CARD_BG);
                imageBg.setStroke((int) (1 * d()), Menu.COLOR_BORDER);
                imageBg.setCornerRadius(12 * d());
                imageView.setBackground(imageBg);
                imageView.setElevation(2 * d());

                imageContainer.addView(imageView);

                CheckBox checkBox = new CheckBox(context);
                checkBox.setButtonDrawable(android.R.color.transparent);
                checkBox.setBackground(createCheckBoxBg(false));

                FrameLayout.LayoutParams checkLp = new FrameLayout.LayoutParams(
                        (int) (24 * d()), (int) (24 * d()));
                checkLp.gravity = Gravity.TOP | Gravity.RIGHT;
                checkLp.topMargin = (int) (-6 * d());
                checkLp.rightMargin = (int) (-6 * d());
                checkBox.setLayoutParams(checkLp);

                imageContainer.addView(checkBox);
                contentLayout.addView(imageContainer);

                // BUFF名称
                TextView textView = new TextView(context);
                textView.setTextColor(Menu.COLOR_TEXT_PRIMARY);
                textView.setTextSize(11);
                textView.setTypeface(Utils.getFont(context));
                textView.setGravity(Gravity.CENTER);
                textView.setMaxLines(1);
                textView.setPadding(0, (int) (4 * d()), 0, 0);
                contentLayout.addView(textView);

                // BUFF ID
                TextView idView = new TextView(context);
                idView.setTextColor(Menu.COLOR_TEXT_SECONDARY);
                idView.setTextSize(9);
                idView.setTypeface(Typeface.MONOSPACE);
                idView.setGravity(Gravity.CENTER);
                contentLayout.addView(idView);

                frameLayout.addView(contentLayout);

                holder = new ViewHolder();
                holder.checkBox = checkBox;
                holder.imageView = imageView;
                holder.textView = textView;
                holder.idView = idView;
                holder.imageContainer = imageContainer;
                holder.frameLayout = frameLayout;
                frameLayout.setTag(holder);
                convertView = frameLayout;
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            BuffInfo buff = getItem(position);
            holder.textView.setText(buff.displayName);
            holder.idView.setText("ID: " + buff.id);

            // 设置复选框状态 - 先移除监听器再设置
            holder.checkBox.setOnCheckedChangeListener(null);

            Boolean isSelected = selectedStates.get(position);
            boolean checked = isSelected != null && isSelected;
            holder.checkBox.setChecked(checked);

            // 更新选中状态的外观
            updateSelectionAppearance(holder, checked);

            // 加载图片
            holder.imageView.setTag(buff.fileName);
            Bitmap cached = bitmapCache.get(buff.fileName);
            if (cached != null && !cached.isRecycled()) {
                holder.imageView.setImageBitmap(cached);
            } else {
                holder.imageView.setImageBitmap(null);
                loadBitmapAsync(buff.fileName, holder.imageView);
            }

            // 设置复选框的监听器
            final int currentPosition = position;
            final ViewHolder finalHolder = holder;
            holder.checkBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    selectedStates.put(currentPosition, isChecked);
                    updateSelectedCount();
                    updateSelectionAppearance(finalHolder, isChecked);
                }
            });

            // 整个项目点击切换复选框
            convertView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    boolean newState = !finalHolder.checkBox.isChecked();
                    finalHolder.checkBox.setChecked(newState);
                }
            });

            return convertView;
        }

        private void updateSelectionAppearance(ViewHolder holder, boolean isSelected) {
            // 更新图片边框
            GradientDrawable imageBg = (GradientDrawable) holder.imageView.getBackground();
            if (imageBg != null) {
                if (isSelected) {
                    imageBg.setStroke((int) (3 * d()), Menu.COLOR_ACCENT_BLUE);
                } else {
                    imageBg.setStroke((int) (1 * d()), Menu.COLOR_BORDER);
                }
            }

            // 更新复选框背景
            holder.checkBox.setBackground(createCheckBoxBg(isSelected));
        }

        private GradientDrawable createCheckBoxBg(boolean isChecked) {
            GradientDrawable drawable = new GradientDrawable();
            drawable.setShape(GradientDrawable.RECTANGLE);
            drawable.setCornerRadius(6 * d());
            if (isChecked) {
                drawable.setColor(Menu.COLOR_ACCENT_BLUE);
                drawable.setStroke((int) (1 * d()), Menu.COLOR_ACCENT_BLUE);
            } else {
                drawable.setColor(0x99000000);
                drawable.setStroke((int) (1 * d()), Menu.COLOR_BORDER);
            }
            return drawable;
        }

        private float d() {
            return context.getResources().getDisplayMetrics().density;
        }

        private void loadBitmapAsync(final String fileName, final ImageView imageView) {
            executor.execute(() -> {
                if (isDestroyed.get()) return;
                if (!fileName.equals(imageView.getTag())) return;

                Bitmap bitmap = null;
                InputStream is = null;
                try {
                    is = context.getAssets().open("buffs/" + fileName);
                    BitmapFactory.Options options = new BitmapFactory.Options();
                    options.inSampleSize = 1;
                    options.inPreferredConfig = Bitmap.Config.RGB_565;
                    bitmap = BitmapFactory.decodeStream(is, null, options);
                    if (bitmap != null) {
                        bitmapCache.put(fileName, bitmap);
                    }
                } catch (IOException ignored) {
                } finally {
                    if (is != null) try { is.close(); } catch (IOException ignored) {}
                }

                final Bitmap result = bitmap;
                handler.post(() -> {
                    if (isDestroyed.get()) return;
                    if (fileName.equals(imageView.getTag()) && result != null) {
                        imageView.setImageBitmap(result);
                    }
                });
            });
        }
    }

    private static class ViewHolder {
        CheckBox checkBox;
        ImageView imageView;
        TextView textView;
        TextView idView;
        FrameLayout imageContainer;
        FrameLayout frameLayout;
    }

    private void safeToast(String msg) {
        if (context != null && msg != null && msg.length() < 50) {
            handler.post(() -> {
                if (!isDestroyed.get()) Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
            });
        }
    }

    private void safeSetStatus(String msg) {
        if (msg == null || msg.length() > 100) msg = "...";
        final String finalMsg = msg;
        handler.post(() -> {
            if (statusText != null && !isDestroyed.get()) {
                statusText.setText(finalMsg);
            }
        });
    }

    private void cleanup() {
        isDestroyed.set(true);
        handler.removeCallbacksAndMessages(null);
        executor.shutdownNow();
        if (bitmapCache != null) bitmapCache.evictAll();
        if (allBuffs != null) allBuffs.clear();
        if (filteredBuffs != null) filteredBuffs.clear();
        selectedStates.clear();
        gridView = null;
        gridAdapter = null;
        statusText = null;
        generateButton = null;
        countEditText = null;
        searchEditText = null;
        clearSearchButton = null;
        selectedCountText = null;
    }

    public void dismiss() {
        if (dialog != null && dialog.isShowing()) {
            try { dialog.dismiss(); } catch (Exception ignored) {}
        }
        cleanup();
    }
}