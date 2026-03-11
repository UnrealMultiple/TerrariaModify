package zig.cheat.qq.ui;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.GradientDrawable;
import android.view.MotionEvent;
import android.widget.Button;

public class Utils {
    private static Typeface cachedFont;
    
    public static Typeface getFont(Context context) {
        if (cachedFont == null) {
            try {
                cachedFont = Typeface.createFromAsset(context.getAssets(), "fonts/font_main.ttf");
            } catch (Throwable t) {
                cachedFont = Typeface.DEFAULT;
            }
        }
        return cachedFont;
    }

    @SuppressLint("ClickableViewAccessibility")
    public static Button createActionButton(Context context, String text, float d) {
        Button btn = new Button(context);
        btn.setText(text);
        btn.setTextColor(Menu.COLOR_TEXT_PRIMARY);
        btn.setTextSize(13);
        btn.setTypeface(Utils.getFont(context));
        btn.setAllCaps(false);
        btn.setPadding(0, 0, 0, 0);
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(Menu.COLOR_CARD_BG);
        bg.setStroke((int) (1 * d), Menu.COLOR_BORDER);
        bg.setCornerRadius(8 * d);
        btn.setBackground(bg);
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
    
    public static GradientDrawable createCardBg(int color, float radiusDp, Context context) {
        GradientDrawable gd = new GradientDrawable();
        gd.setColor(color);
        gd.setCornerRadius(radiusDp * context.getResources().getDisplayMetrics().density);
        return gd;
    }
    
    public static GradientDrawable createStrokeBg(int color, float radiusDp, int strokeWidthDp, int strokeColor, Context context) {
        float d = context.getResources().getDisplayMetrics().density;
        GradientDrawable gd = new GradientDrawable();
        gd.setColor(color);
        gd.setCornerRadius(radiusDp * d);
        gd.setStroke((int)(strokeWidthDp * d), strokeColor);
        return gd;
    }

    public static GradientDrawable createGradientBg(int[] colors, float radiusDp, Context context) {
        GradientDrawable gd = new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM, colors);
        gd.setCornerRadius(radiusDp * context.getResources().getDisplayMetrics().density);
        return gd;
    }

    public static GradientDrawable createCircleBg(int color) {
        GradientDrawable gd = new GradientDrawable();
        gd.setColor(color);
        gd.setShape(GradientDrawable.OVAL);
        return gd;
    }
    
    public static int dpToPx(float dp, Context context) {
        return (int)(dp * context.getResources().getDisplayMetrics().density);
    }
    
    public static Drawable getIcon(Context context, String name) {
        try {
            return Drawable.createFromStream(context.getAssets().open(name), null);
        } catch (Throwable t) {
            return null;
        }
    }
}
