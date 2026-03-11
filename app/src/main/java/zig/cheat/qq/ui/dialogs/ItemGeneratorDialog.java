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
import android.widget.EditText;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

import zig.cheat.qq.ui.Menu;
import zig.cheat.qq.ui.Utils;

public class ItemGeneratorDialog {
    private static final int PAGE_SIZE = 48;
    private static final int GRID_COLUMNS = 8;
    private static final int MAX_BITMAP_CACHE_SIZE = 30;
    private static final int TARGET_IMAGE_SIZE_DP = 60;
    private static final int DIALOG_WIDTH_DP = 600;
    private static final long SEARCH_DEBOUNCE_MS = 300;

    private final Context context;
    private final Handler handler;
    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private final AtomicBoolean isDestroyed = new AtomicBoolean(false);

    private Dialog dialog;
    private GridView gridView;
    private ItemGridAdapter gridAdapter;
    private TextView statusText;
    private TextView pageInfoText;
    private Button prevButton, nextButton;
    private EditText searchEditText;
    // 新增：数量输入框
    private EditText countEditText;

    private List<ItemInfo> allItems = new ArrayList<>();
    private List<ItemInfo> filteredItems = new ArrayList<>();
    private List<ItemInfo> currentPageItems = new ArrayList<>();
    private int currentPage = 0;
    private int totalPages = 0;

    private final LruCache<String, Bitmap> bitmapCache;

    private final Runnable searchRunnable = this::performSearchNow;

    // 修改：物品点击监听器接口，增加数量参数
    public interface OnItemClickListener {
        void onItemClick(String name, int id, int count);
    }

    private OnItemClickListener itemClickListener;

    public void setOnItemClickListener(OnItemClickListener listener) {
        this.itemClickListener = listener;
    }

    private static class ItemInfo {
        String name;
        int id;
        String fileName;

        ItemInfo(String name, int id, String fileName) {
            this.name = name;
            this.id = id;
            this.fileName = fileName;
        }
    }

    public ItemGeneratorDialog(Context ctx) {
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
        final int width = (int) (DIALOG_WIDTH_DP * d);

        LinearLayout root = new LinearLayout(context);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding((int) (16 * d), (int) (16 * d), (int) (16 * d), (int) (16 * d));

        GradientDrawable bg = new GradientDrawable();
        bg.setColor(Menu.COLOR_WINDOW_BG);
        bg.setCornerRadius(12 * d);
        bg.setStroke((int) (1 * d), Menu.COLOR_BORDER);
        root.setBackground(bg);

        // 标题
        TextView title = new TextView(context);
        title.setText("ITEM GENERATOR");
        title.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        title.setTextSize(14);
        title.setTypeface(Utils.getFont(context));
        title.setLetterSpacing(0.1f);
        title.setGravity(Gravity.CENTER);
        root.addView(title);

        // ========== 搜索行 + 数量输入框（已增加输入框之间的间距） ==========
        LinearLayout searchLayout = new LinearLayout(context);
        searchLayout.setOrientation(LinearLayout.HORIZONTAL);
        // 增加左右内边距，使搜索行不贴边
        searchLayout.setPadding((int) (12 * d), (int) (8 * d), (int) (12 * d), (int) (4 * d));

        // 搜索框
        searchEditText = new EditText(context);
        searchEditText.setHint("搜索名称或ID...");
        searchEditText.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        searchEditText.setHintTextColor(Menu.COLOR_TEXT_SECONDARY);
        searchEditText.setTextSize(12);
        searchEditText.setTypeface(Utils.getFont(context));
        searchEditText.setBackground(createInputBg(d));
        searchEditText.setPadding((int) (12 * d), (int) (8 * d), (int) (12 * d), (int) (8 * d));
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

        LinearLayout.LayoutParams searchLp = new LinearLayout.LayoutParams(0, (int) (36 * d), 1);
        searchEditText.setLayoutParams(searchLp);
        searchLayout.addView(searchEditText);

        // 数量输入框（带简短标签）
        LinearLayout countWrapper = new LinearLayout(context);
        countWrapper.setOrientation(LinearLayout.HORIZONTAL);
        countWrapper.setBackground(createInputBg(d));
        countWrapper.setPadding((int) (6 * d), 0, (int) (6 * d), 0);
        // 设置固定宽度，并添加左右边距：左边距与搜索框隔开，右边距与清除按钮隔开
        LinearLayout.LayoutParams countWrapperLp = new LinearLayout.LayoutParams((int) (100 * d), (int) (36 * d));
        countWrapperLp.leftMargin = (int) (8 * d);   // ← 新增左边距，使搜索框与数量框之间产生间距
        countWrapperLp.rightMargin = (int) (8 * d);  // 保留右边距，与清除按钮保持距离
        countWrapper.setLayoutParams(countWrapperLp);

        TextView countLabel = new TextView(context);
        countLabel.setText("数量");
        countLabel.setTextColor(Menu.COLOR_TEXT_SECONDARY);
        countLabel.setTextSize(10);
        countLabel.setTypeface(Utils.getFont(context));
        countLabel.setPadding((int) (4 * d), 0, (int) (4 * d), 0);
        countLabel.setGravity(Gravity.CENTER_VERTICAL);
        countWrapper.addView(countLabel);

        countEditText = new EditText(context);
        countEditText.setHint("1");
        countEditText.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        countEditText.setHintTextColor(0xFF666666);
        countEditText.setTextSize(11);
        countEditText.setTypeface(Typeface.MONOSPACE);
        countEditText.setInputType(InputType.TYPE_CLASS_NUMBER);
        countEditText.setBackground(null);
        countEditText.setPadding(0, 0, (int) (4 * d), 0);
        countEditText.setLayoutParams(new LinearLayout.LayoutParams(0, -1, 1));
        countWrapper.addView(countEditText);

        searchLayout.addView(countWrapper);

        // 清除搜索按钮
        Button clearSearch = new Button(context);
        clearSearch.setText("✕");
        clearSearch.setTextColor(Color.WHITE);
        clearSearch.setTextSize(12);
        clearSearch.setTypeface(Utils.getFont(context));
        clearSearch.setAllCaps(false);
        GradientDrawable clearBg = new GradientDrawable();
        clearBg.setColor(0xFF6B7280);
        clearBg.setCornerRadius(6 * d);
        clearSearch.setBackground(clearBg);
        clearSearch.setOnTouchListener((v, event) -> {
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
        clearSearch.setOnClickListener(v -> {
            searchEditText.setText("");
            performSearchNow();
        });
        LinearLayout.LayoutParams clearLp = new LinearLayout.LayoutParams((int) (40 * d), (int) (36 * d));
        clearLp.leftMargin = (int) (8 * d);
        clearSearch.setLayoutParams(clearLp);
        searchLayout.addView(clearSearch);

        root.addView(searchLayout);

        // 分隔线
        View divider = new View(context);
        LinearLayout.LayoutParams divLp = new LinearLayout.LayoutParams(-1, (int) (1 * d));
        divLp.topMargin = (int) (8 * d);
        divLp.bottomMargin = (int) (8 * d);
        divider.setLayoutParams(divLp);
        divider.setBackgroundColor(Menu.COLOR_DIVIDER);
        root.addView(divider);

        // 翻页栏
        LinearLayout pageBar = new LinearLayout(context);
        pageBar.setOrientation(LinearLayout.HORIZONTAL);
        pageBar.setGravity(Gravity.CENTER_VERTICAL);
        LinearLayout.LayoutParams barLp = new LinearLayout.LayoutParams(-1, (int) (30 * d));
        barLp.topMargin = (int) (4 * d);
        barLp.bottomMargin = (int) (8 * d);
        pageBar.setLayoutParams(barLp);

        prevButton = createPageButton("◀", d);
        prevButton.setOnClickListener(v -> goToPage(currentPage - 1));
        pageBar.addView(prevButton);

        pageInfoText = new TextView(context);
        pageInfoText.setTextColor(Menu.COLOR_TEXT_SECONDARY);
        pageInfoText.setTextSize(12);
        pageInfoText.setTypeface(Utils.getFont(context));
        pageInfoText.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams infoLp = new LinearLayout.LayoutParams(0, -1, 1);
        pageInfoText.setLayoutParams(infoLp);
        pageBar.addView(pageInfoText);

        nextButton = createPageButton("▶", d);
        nextButton.setOnClickListener(v -> goToPage(currentPage + 1));
        pageBar.addView(nextButton);

        root.addView(pageBar);

        // GridView
        gridView = new GridView(context);
        gridView.setNumColumns(GRID_COLUMNS);
        gridView.setVerticalSpacing((int) (8 * d));
        gridView.setHorizontalSpacing((int) (8 * d));
        gridView.setStretchMode(GridView.STRETCH_COLUMN_WIDTH);
        gridView.setPadding(0, 0, 0, 0);

        LinearLayout.LayoutParams gridLp = new LinearLayout.LayoutParams(-1, 0, 1.0f);
        gridView.setLayoutParams(gridLp);
        root.addView(gridView);

        // 状态文本
        statusText = new TextView(context);
        statusText.setTextColor(Menu.COLOR_TEXT_SECONDARY);
        statusText.setTextSize(10);
        statusText.setTypeface(Utils.getFont(context));
        statusText.setPadding(0, (int) (8 * d), 0, 0);
        statusText.setGravity(Gravity.CENTER);
        root.addView(statusText);

        dialog.setContentView(root);

        Window window = dialog.getWindow();
        if (window != null) {
            window.setLayout(width, WindowManager.LayoutParams.WRAP_CONTENT);
            window.setBackgroundDrawableResource(android.R.color.transparent);
        }

        dialog.show();
        scanAssets();
    }

    private GradientDrawable createInputBg(float d) {
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(0xFF0F0F0F);
        bg.setCornerRadius(6 * d);
        bg.setStroke((int) (1 * d), 0xFF333333);
        return bg;
    }

    @SuppressLint("ClickableViewAccessibility")
    private Button createPageButton(String text, float d) {
        Button btn = new Button(context);
        btn.setText(text);
        btn.setTextColor(Color.WHITE);
        btn.setTextSize(13);
        btn.setTypeface(Utils.getFont(context));
        btn.setAllCaps(false);
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(0x1AFFFFFF);           // 半透明底色
        bg.setStroke((int) (1 * d), 0x33FFFFFF); // 描边
        bg.setCornerRadius(12 * d);
        btn.setBackground(bg);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams((int) (60 * d), (int) (40 * d));
        btn.setLayoutParams(lp);
        btn.setOnTouchListener((v, event) -> {
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
        return btn;
    }

    private void scanAssets() {
        executor.execute(() -> {
            if (isDestroyed.get()) return;

            List<ItemInfo> items = new ArrayList<>();
            AssetManager am = context.getAssets();
            try {
                String[] files = am.list("items");
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
                                    items.add(new ItemInfo(name, id, file));
                                } catch (NumberFormatException ignored) {}
                            }
                        }
                    }
                }
            } catch (IOException e) {
                handler.post(() -> safeSetStatus("读取 assets 失败"));
                return;
            }

            items.sort((a, b) -> a.name.compareToIgnoreCase(b.name));

            handler.post(() -> {
                if (isDestroyed.get()) return;
                allItems = items;
                filteredItems = new ArrayList<>(allItems);
                totalPages = (int) Math.ceil((double) filteredItems.size() / PAGE_SIZE);
                currentPage = 0;
                updatePageData();
                safeSetStatus("共 " + allItems.size() + " 个物品");
            });
        });
    }

    private void performSearchNow() {
        if (isDestroyed.get()) return;
        String query = searchEditText.getText().toString().trim().toLowerCase();
        if (query.isEmpty()) {
            filteredItems = new ArrayList<>(allItems);
        } else {
            filteredItems = new ArrayList<>();
            for (ItemInfo item : allItems) {
                if (item.name.toLowerCase().contains(query) ||
                        String.valueOf(item.id).contains(query)) {
                    filteredItems.add(item);
                }
            }
        }
        totalPages = (int) Math.ceil((double) filteredItems.size() / PAGE_SIZE);
        currentPage = 0;
        updatePageData();
        safeSetStatus("共搜索到 " + filteredItems.size() + " 个物品");
    }

    @SuppressLint("SetTextI18n")
    private void updatePageData() {
        if (filteredItems.isEmpty()) {
            currentPageItems.clear();
            pageInfoText.setText("0/0");
            prevButton.setEnabled(false);
            nextButton.setEnabled(false);
        } else {
            int from = currentPage * PAGE_SIZE;
            int to = Math.min(from + PAGE_SIZE, filteredItems.size());
            currentPageItems = new ArrayList<>(filteredItems.subList(from, to));

            pageInfoText.setText((currentPage + 1) + "/" + totalPages);
            prevButton.setEnabled(currentPage > 0);
            nextButton.setEnabled(currentPage < totalPages - 1);
        }

        if (gridAdapter == null) {
            gridAdapter = new ItemGridAdapter();
            gridView.setAdapter(gridAdapter);
        } else {
            gridAdapter.notifyDataSetChanged();
        }
    }

    private void goToPage(int page) {
        if (page < 0 || page >= totalPages) return;
        currentPage = page;
        updatePageData();
    }

    private class ItemGridAdapter extends BaseAdapter {
        @Override
        public int getCount() {
            return currentPageItems.size();
        }

        @Override
        public ItemInfo getItem(int position) {
            return currentPageItems.get(position);
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
                LinearLayout layout = new LinearLayout(context);
                layout.setOrientation(LinearLayout.VERTICAL);
                layout.setGravity(Gravity.CENTER_HORIZONTAL);
                // 移除父布局的内边距，由子视图自己控制间距
                layout.setPadding(0, 0, 0, 0);

                // 创建 ImageView，设置大小和底部外边距（与文字隔开）
                ImageView imageView = new ImageView(context);
                int size = (int) (TARGET_IMAGE_SIZE_DP * d());
                LinearLayout.LayoutParams imgLp = new LinearLayout.LayoutParams(size, size);
                imgLp.bottomMargin = (int) (4 * d());
                imageView.setLayoutParams(imgLp);

                // 【关键】设置内边距，让图片内容与边框之间留出空白
                int padding = (int) (6 * d()); // 可根据视觉效果调整（例如 4dp、6dp）
                imageView.setPadding(padding, padding, padding, padding);
                imageView.setScaleType(ImageView.ScaleType.FIT_CENTER);

                // 设置更协调的边框样式（无填充 + 极细半透描边 + 大圆角）
                GradientDrawable imageBg = new GradientDrawable();
                imageBg.setColor(0x1AFFFFFF);               // 半透明底色（约10%）
                imageBg.setStroke((int) (0.5 * d()), 0x33FFFFFF); // 极细半透描边
                imageBg.setCornerRadius(12 * d());           // 较大圆角，更柔和
                imageView.setBackground(imageBg);

                // 可选：添加轻微阴影（API 21+），不影响边框
                imageView.setElevation(4 * d());
                imageView.setOutlineProvider(null);      // 避免自动裁剪轮廓

                layout.addView(imageView);

                // 文字标签
                TextView textView = new TextView(context);
                textView.setTextColor(Menu.COLOR_TEXT_PRIMARY);
                textView.setTextSize(10);
                textView.setTypeface(Utils.getFont(context));
                textView.setGravity(Gravity.CENTER);
                textView.setMaxLines(2);
                textView.setPadding(0, (int) (2 * d()), 0, 0);
                layout.addView(textView);

                holder = new ViewHolder();
                holder.imageView = imageView;
                holder.textView = textView;
                layout.setTag(holder);
                convertView = layout;
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            // 绑定数据
            ItemInfo item = getItem(position);
            holder.textView.setText(item.name);
            holder.imageView.setTag(item.fileName);

            Bitmap cached = bitmapCache.get(item.fileName);
            if (cached != null && !cached.isRecycled()) {
                holder.imageView.setImageBitmap(cached);
            } else {
                holder.imageView.setImageBitmap(null);
                loadBitmapAsync(item.fileName, holder.imageView);
            }

            // 点击事件
            convertView.setOnClickListener(v -> {
                if (itemClickListener != null) {
                    int count = 1;
                    try {
                        String countStr = countEditText.getText().toString().trim();
                        if (!countStr.isEmpty()) {
                            count = Integer.parseInt(countStr);
                            if (count <= 0) count = 1;
                        }
                    } catch (NumberFormatException ignored) {}
                    itemClickListener.onItemClick(item.name, item.id, count);
                }
            });

            return convertView;
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
                    is = context.getAssets().open("items/" + fileName);
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
        ImageView imageView;
        TextView textView;
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
        if (currentPageItems != null) currentPageItems.clear();
        gridView = null;
        gridAdapter = null;
        statusText = null;
        prevButton = nextButton = null;
        pageInfoText = null;
        searchEditText = null;
        countEditText = null;
    }

    public void dismiss() {
        if (dialog != null && dialog.isShowing()) {
            try { dialog.dismiss(); } catch (Exception ignored) {}
        }
        cleanup();
    }
}