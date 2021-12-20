package com.github.opengles_android.common;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;

import com.github.opengles_android.R;

import javax.crypto.spec.GCMParameterSpec;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.widget.AppCompatSeekBar;
import androidx.core.content.ContextCompat;

public class NumSeekBar extends AppCompatSeekBar {
    private Paint mPaint;
    private Rect mTextBounds = new Rect();
    private int mPaddingRight;

    public NumSeekBar(@NonNull Context context) {
        this(context, null);
    }

    public NumSeekBar(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);

        mPaint = new Paint();
        mPaint.setTextSize(getResources().getDimension(R.dimen.seekbar_text_size));
        mPaint.setColor(ContextCompat.getColor(getContext(), R.color.text_color));
        mPaint.setAntiAlias(true);
        mPaddingRight = getPaddingRight();
    }

    @Override
    protected synchronized void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        float textWidth = 0;
        float textHeight = 0;

        if (null != getTooltipText()) {
            String title = getTooltipText().toString();
            mPaint.getTextBounds(title, 0, title.length(), mTextBounds);
            textHeight = mTextBounds.height();
            canvas.drawText(title, 0, textHeight, mPaint);
        }

        String text = String.valueOf(getProgress());
        mPaint.getTextBounds(text, 0, text.length(), mTextBounds);
        textWidth = mTextBounds.width();
        textHeight = mTextBounds.height();
        canvas.drawText(text, getWidth() - textWidth - mPaddingRight, textHeight, mPaint);

    }
}
